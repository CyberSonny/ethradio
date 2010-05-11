/* Name: network.c
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#include "nike_e.h"
#include "enc28j60.h"
#include "io.h"
#include "FIFO.h"
#include "compiler.h"

#pragma inline=forced
UINT16 _READ_U16(volatile UINT8 *p)
{
#pragma diag_suppress=Pa082
  return p[0]|(p[1]<<8);
#pragma diag_default=Pa082
}
#define READ_U16(VAR) (_READ_U16((volatile UINT8 *)(&VAR)))
//#define READ_U16(VAR) (VAR)

#pragma inline=forced
UINT16 _READ_U16_REV(volatile UINT8 *p)
{
#pragma diag_suppress=Pa082
  return p[1]|(p[0]<<8);
#pragma diag_default=Pa082
}

//Определяет, использовать ли номер IP-пакета
#define USE_IPSEQ
//#define IPSEQ_DEBUG

#include "arp.h"
#include "ip.h"
#include "icmp.h"
#include "tcp.h"
#include "network_addr.h"
#include "network.h"

#define TOTAL_INLINE

//__no_init ETH_FRAME ETH_PKT @ ETH_PKT_BASE;

/*volatile */UINT16 ETH_PKT_len;
/*volatile */UINT8 ETH_PKT_mode; //0 - свободен, 1 - занят приемником, 2-16 - счетчик передач

volatile union
{
  struct
  {
    UINT8 count_200ms_low;
    UINT8 count_200ms_high;
    UINT16 irs_high;
  };
  struct
  {
    UINT16 IRS_L;
    UINT16 IRS_H;
  };
};

//static UINT8 MAC_BROADCAST[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

#ifdef CPU8BIT
#include "network_routines_avr.c"
#else
#include "network_routines_arm.c"
#endif

extern __eeprom UINT32 OUTG_IP[2];
//extern char MAC_GATE0[ETH_HWA_LEN];

TCP_SOCK *QUE_sock;

__z void AddTCPsocket(TCP_SOCK *s)
{
  TCP_SOCK *p;
 // UREG save_lock=ETH_TASK_LOCK_PORT;
 // ETH_TASK_LOCK=1;
  s->timer_init=0;
  p=QUE_sock;
  s->next=p;
  QUE_sock=s;
 // if (!(save_lock&ETH_TASK_LOCK_MASK)) ETH_TASK_LOCK=0;
}

__x UREG RemoveTCPsocket(TCP_SOCK *s)
{
  TCP_SOCK *p=(TCP_SOCK *)(&QUE_sock);
  TCP_SOCK *n;
  UREG f=0;
 // UREG save_lock=ETH_TASK_LOCK_PORT;
 // ETH_TASK_LOCK=1;
  while((n=p->next))
  {
    if (n==s)
    {
      p->next=s->next;
      f=1;
      break;
    }
    p=n;
  }
  //if (!(save_lock&ETH_TASK_LOCK_MASK)) ETH_TASK_LOCK=0;
  return f;
}


static __z void prepare_sock(TCP_SOCK *s, UREG state, UREG tmr)
{
  s->state=state;
  s->flags=state==TCP_STATE_CONNECTING?TCP_FLAG_SYN:TCP_FLAG_SYN | TCP_FLAG_ACK;
  s->timer=tmr;
  
  UINT16 irs_l;
  UINT16 irs_h;
  irs_l=IRS_L;
  irs_h=IRS_H;
  irs_h^=irs_l;
  irs_h+=irs_l;
  IRS_H=irs_h;
  s->SEQNO=((UINT32)irs_h<<16)|irs_l;
  
  s->timer_init=TCP_TIMER_INITV;
  s->sended_len=0;
  s->send_disp=0;
}

__x void TCPconnect(TCP_SOCK *s, UINT32 ip, UINT16 port)
{
  TCP_SOCK *ss;
//  UREG save_lock=ETH_TASK_LOCK_PORT;
//  ETH_TASK_LOCK=1;
  s->rport=htons(port);
  s->lport=0;
  port=IRS_L;
  port^=IRS_H;
  port=(port&0x3FF)+1024;
L1:
  ss=QUE_sock;
  while(ss)
  {
    if ((ss->state>TCP_STATE_LISTENING)&&port==ntohs(ss->lport))
    {
      port++;
      goto L1;
    }
    ss=ss->next;
  }
  s->lport=htons(port);
  s->rip=ip;
  //s->ACKNO=0; //Начинаем с 0
  prepare_sock(s,TCP_STATE_CONNECTING,0);
  ETH_TASK_WAKEUP=1;
 // if (!(save_lock&ETH_TASK_LOCK_MASK)) ETH_TASK_LOCK=0;
}

static __x UREG CallTCPhook_null(UREG code, TCP_SOCK *s);

static __x UREG ClosingTCP(UREG ev, TCP_SOCK *s)
{
  s->timer_init=0;
  s->txreq=TCP_TXREQ_NONE;
  if (s->type&TCP_TYPE_SERVER)
  {
    s->state=TCP_STATE_LISTENING;
  }
  else
    s->state=TCP_STATE_CLOSED;
  return CallTCPhook_null(ev,s);
}

static __x_z UREG CallTCPhook(UREG code, UREG len, void *p, TCP_SOCK *s);

#pragma optimize=no_inline
static __x UREG CallTCPhook_null(UREG code, TCP_SOCK *s)
{
  return CallTCPhook(code,0,NULL,s);
}

#pragma optimize=no_inline
static __x_z UREG CallTCPhook(UREG code, UREG len, void *p, TCP_SOCK *s)
{
  return s->hook(code,len,p,s);
}

#define __get_eth_root(VAR) ((ETH_FRAME *)(&ETH_PKT))

extern __monitor void ETH_STOP_BACK_PRESSURE(void);
extern __monitor __x UREG ETH_TRANSMIT_PACKET(ETH_FRAME *ep, unsigned int l);

#ifdef USE_IPSEQ
int IPSEQ;
#endif

#pragma optimize=no_inline
static __z unsigned int ExtractMSS(TCP_PKT *tcp)
{
  unsigned int mss=TCP_MAX_DATA_LEN>536?536:TCP_MAX_DATA_LEN;
  if (tcp->tcp.hlen<(TCP_HEADER_LEN+4)) return mss;
  if (tcp->data[0]==2&&tcp->data[1]==4)
  {
    unsigned int rmss=ntohs(*((UINT16*)(tcp->data+2)));
    mss=TCP_MAX_DATA_LEN;
    if (rmss<mss) mss=rmss;
  }
  return mss;
}

__x static void set_state_SYN_RECEIVED(TCP_SOCK *s, UINT32 *ackno, unsigned int mss)
{
  s->rmss=mss;
  prepare_sock(s,TCP_STATE_SYN_RECEIVED,TCP_TIMER_INITV);
  inc32cpy(&s->ACKNO,ackno);
}

void *debug_addr;

//char const __flash _DecByteNum[]="%04X";
//char const __flash _Razer2[]="\r\n";
extern volatile UINT8 BufferPKT;
extern volatile UINT8 FIFO_pkt_TS;
extern char const __flash _EthWRlen[];
extern char const __flash _EthTS[]; 
extern char const __flash _EthCNT[];
extern char const __flash _EthpWRH[];
extern char const __flash _EthpWRL[];

extern char const __flash _EthRDlen[];
extern char const __flash _EthpRDH[];
extern char const __flash _EthpRDL[];

#pragma inline = forced
void ETHfifo_write(UINT8 *data, UINT16 len, UINT8 TS)
{
	UINT32 pWR; //локальный указатель на запись
        UINT16 len1;        
	UINT8 c;
	if(len == 0)  return;
        if (ETHfifo_free() < ETH_MAX_PACKET_SIZE+3) return; // если пакет+длина+таймаут не влезут - не пишем
	UINT8 header_cnt=0;
       	pWR = ETHfifo_pWR;
        len+=3; //  к длине Ethernet пакета добавим 2 байта его длины и 1 байт метки времени
        #ifdef CONSOLE_DEBUG
           _print_num (_EthWRlen,len); _print_num (_EthTS,(UINT16)TS);
           _print_num (_EthpWRH,(UINT16)(pWR>>16)); _print_num (_EthpWRL,(UINT16)pWR);
        #endif
        len1=len;
        {
          UREG addr;
          FM_CS_ENABLE();
          SPDR=FM_WREN;        
          while (!(SPSR & (1<<SPIF)));        
          FM_CS_DISABLE();        
          FM_CS_ENABLE();
          SPDR=FM_WRITE;       
          addr= (UREG) (pWR>>16);        
          while (!(SPSR & (1<<SPIF)));  
          SPDR= addr;
          addr= (UREG)(pWR>>8);
          while (!(SPSR & (1<<SPIF)));  
          SPDR=addr;  
          addr= (UREG)(pWR);
          while (!(SPSR & (1<<SPIF)));          
          SPDR=addr;        
          while (!(SPSR & (1<<SPIF)));          
        }  
        // Открыли сессию записи в FIFO                                       
        do
        {
          switch (header_cnt)
          {
           case 0:
            c=(UINT8) ((len-3) >>8);
            header_cnt++;
            break;
           case 1:
            c=(UINT8) (len-3);
            header_cnt++;
            break;
          case 2:  
            c= TS;
            header_cnt++;
            break;
          default:
            c = *data++;
            break;
          }             		
          SPDR = c;                     // записали байт в FRAM
          UREG f=0;
          if (++pWR>ETHfifo_pEND) f=1;// при записи след. ячейки упремся в потолок
          while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished                 
          if (f) // при записи след. ячейки упремся в потолок... начнем с начала
          {
            UREG addr;
       	    FM_CS_DISABLE(); // закроем старую сессию    
            pWR=ETHfifo_pSTART;
            FM_CS_ENABLE();  // откроем новую сессию записи с начального адреса
            SPDR=FM_WREN;        
            while (!(SPSR & (1<<SPIF)));              
	    FM_CS_DISABLE();
  	    FM_CS_ENABLE();
            SPDR=FM_WRITE;        
            addr=(UREG)(pWR>>16);
            while (!(SPSR & (1<<SPIF)));  
            SPDR=addr;        
            addr=(UREG)(pWR>>8);        
            while (!(SPSR & (1<<SPIF)));  
            SPDR=addr;
            addr=(UREG)(pWR);
            while (!(SPSR & (1<<SPIF)));          
            SPDR=addr;        
            while (!(SPSR & (1<<SPIF)));    // Открыли сессию записи в FIFO                                   
          }
        }
        while (len1--);
        FM_CS_DISABLE();
     	ETHfifo_pWR = pWR;
        ETHfifo_CNT++;// увеличиваем счетчик пакетов в FIFO
        #ifdef CONSOLE_DEBUG
            _print_num (_EthCNT,(UINT16)ETHfifo_CNT);
        #endif       
}


#pragma diag_suppress=Ta006
void INT_ETH_PROCESS_PKT2(void)
{
  unsigned int len;
  TCP_SOCK *s;
  ETH_FRAME *ep;
  ARP_FRAME *ap;
  ARP_FRAME *apo;
  IP_FRAME *ip;
  ICMP_PKT *icmp;
  TCP_PKT *tcp;
  TCP_HEADER *ptcp;
  TCP_FRAME *tcpf;
  void *ipdata;
  
  for(;;)
  {
  L_CHECK_INCOMING:
    if (ETH_PKT_mode>=2)
    {
      //if (ETH_TRANSMIT_PACKET(&ETH_PKT,READ_U16(ETH_PKT_len))) continue;
     // print_dump((unsigned char *)&ETH_PKT,READ_U16(ETH_PKT_len));
      if ((enc28j60PacketSend((unsigned char *)&ETH_PKT,READ_U16(ETH_PKT_len)))==0) ETH_PKT_mode=0;
      else ETH_PKT_mode=16;
    }
    if (ETH_PKT_mode!=1) break;
    ep=&ETH_PKT;
    len=READ_U16(ETH_PKT_len);
    ap=(ARP_FRAME*)ep;
    ip=(IP_FRAME*)ep;
    switch(ep->hdr.type)
    {
    case ETH_PROTOCOL_ARP:
      if (ap->arp.proto!=ARP_ETHCODE) break;
      if (ap->arp.hw_type!=ARP_HARDWARE) break;
      switch(ap->arp.opcode)
      {
      case ARP_REQUEST:
	if (IPcmp(&ap->arp.dst_ip,&IP)) break;
	if ((ep->hdr.dst_mac[0]&ep->hdr.dst_mac[1]&ep->hdr.dst_mac[2]&ep->hdr.dst_mac[3]&ep->hdr.dst_mac[4]&ep->hdr.dst_mac[5])!=0xFF) break;
	//if (CRC32_M4((UINT8*)ep,READ_U16(ETH_PKT_len))) break;
	apo=(ARP_FRAME*)ep;
	IPcpy(&apo->arp.dst_ip,&ap->arp.src_ip);
	MACcpy(apo->arp.dst_mac+ETH_HWA_LEN,ap->arp.src_mac+ETH_HWA_LEN);
	MACcpy(apo->arp.src_mac+ETH_HWA_LEN,MAC0+ETH_HWA_LEN);
	MACcpy(apo->eth.dst_mac+ETH_HWA_LEN,apo->eth.src_mac+ETH_HWA_LEN);
	MACcpy(apo->eth.src_mac+ETH_HWA_LEN,MAC0+ETH_HWA_LEN);
	apo->arp.opcode=ARP_RESPONSE;
	IPcpyIP(&apo->arp.src_ip);
	len=ARP_FRAME_LEN;
	ep=(ETH_FRAME*)apo;
	goto L_SENDETH;
      case ARP_RESPONSE:
	if (IPcmp(&ap->arp.dst_ip,&IP)) break;
	if (MACcmp(ep->hdr.dst_mac,MAC0)) break;
	//if (CRC32_M4((UINT8*)ep,READ_U16(ETH_PKT_len))) break;
	MACcpy(ap->eth.src_mac+ETH_HWA_LEN,ap->arp.src_mac+ETH_HWA_LEN); //Именно тот MAC, который внутри пакета
	{
	  //Ищем клиента, который находится в состоянии CONNECTING
	  s=QUE_sock;
	  while(s)
	  {
	      if ((s->state==TCP_STATE_CONNECTING)&&(!IPcmp(&ap->arp.src_ip,&s->ACKNO)))
            //if ((s->state==TCP_STATE_CONNECTING))
	    {
	      //Наш клиент ;)
	      s->state=TCP_STATE_SYN_SENT;
              s->ACKNO=0;
	      ip=(IP_FRAME*)ep;
	      tcpf=(TCP_FRAME*)ip;
	      IPcpy(&tcpf->ip.dest_ip,&s->rip);
	      MACcpy(s->dst_mac+ETH_HWA_LEN,ip->eth.src_mac+ETH_HWA_LEN); //Сохраним нужный MAC-адрес
	      ptcp=(TCP_HEADER*)&s->lport;
	      len=0;
	      goto L_SENDTCPWITHDATA;
	    }
	    s=s->next;
	  }
	}
	break;
      }
      break;
    case ETH_PROTOCOL_IP:      
      if (IPcmp(&ip->ip.dest_ip,&IP)) break;
      if (MACcmp(ep->hdr.dst_mac,MAC0)) break;
      {
        //print_lenRX((unsigned char *)&ETH_PKT, len);	
	unsigned int i=len;  
	UREG ip_hlen;
	len=ntohs(ip->ip.tlen);
	//i-=((ETH_HEADER_LEN)+4); //Без заголовка и CRC32
        i-=((ETH_HEADER_LEN)); //enc28j60 port
	if ((ip->ip.vihl&0xF0)!=0x40) break;
	ip_hlen=((ip->ip.vihl&0x0F)<<2);
	if (ip_hlen>IP_MAX_OPTLEN) break;
	if (ip_hlen>=i) break;
	if (len>i) break;
	if (len<ip_hlen) break;
	len-=ip_hlen;
	ipdata=(void*)((UINT8*)(&ip->ip)+ip_hlen);
        if (!BufferPKT)
        {
	  if (ip->ip.frags&htons(IP_MOREFRAGS | IP_FRAGOFF)) break;// тут вылетело
        }
	//if (CRC32_M4((UINT8*)ep,READ_U16(ETH_PKT_len))) break;
        if (!BufferPKT)
        {
          if (IPChecksum((UINT16 *)&ip->ip, IP_HEADER_LEN)!=IP_GOOD_CS) break;
        }	
      }
      switch(ip->ip.proto)
      {
      case IP_ICMP:
	icmp=ipdata;
	if (len<ICMP_HEADER_LEN) break;
	if (IPChecksum((UINT16*)icmp,len)!=IP_GOOD_CS) break;
	if (icmp->icmp.code) break;
	if (icmp->icmp.type!=ICMP_ECHO_REQUEST) break;
	ep=__get_eth_root(ip);
	IPcpy(&((IP_FRAME*)ep)->ip.dest_ip,&ip->ip.src_ip);
	ip=(IP_FRAME*)ep;
	if (len) netw_memcpy(ip->data,icmp,len);
	icmp=(ICMP_PKT*)ip->data;
	//netw_memcpy(icmp->data+2*sizeof(TCP_SOCK),debug_addr,16); //DEBUG - можно пингом подсмотреть RAM, например ;)
	//netw_memcpy(icmp->data+0*sizeof(TCP_SOCK),QUE_sock,sizeof(TCP_SOCK));
	//netw_memcpy(icmp->data+1*sizeof(TCP_SOCK),QUE_sock->next,sizeof(TCP_SOCK));
	icmp->icmp.type=ICMP_ECHO_RESPONSE;
	icmp->icmp.checksum=0;
	icmp->icmp.checksum=IPChecksum((UINT16*)icmp,len);
	goto L_CREATEIP;
      case IP_TCP:
	{
	  UREG tcp_hlen;
	  tcp=ipdata;
	  if (len<TCP_HEADER_LEN) break;
          if (!BufferPKT)
          {
	    ip->ip.checksum=htons((UINT16)len);
	    ip->ip.frags=~IPChecksum((UINT16*)tcp,len);
	    tcp_hlen=tcp->tcp.hlen=(tcp->tcp.hlen&0xF0)>>2;
          }
          else  tcp_hlen=tcp->tcp.hlen;
            if (tcp_hlen<TCP_HEADER_LEN) break;
	    if (tcp_hlen>(TCP_MAX_OPT_LEN+TCP_HEADER_LEN)) break;
	    if (len<tcp_hlen) break;
	    len-=tcp_hlen;
	}
	ip->ip.ttl=0;
        if (!BufferPKT)
        {
	  if (IPChecksum((UINT16*)&ip->ip.frags,14)!=IP_GOOD_CS) break;
        }
	//Ищем нужный сокет
	s=QUE_sock;
	while(s)
	{
	  if (s->lport==tcp->tcp.dest_port)
	  {
	    //Локальный порт совпадает
	    if (s->state==TCP_STATE_LISTENING)
	    {
	      //Сокет в состоянии LISTENING, делаем bind
	      if((tcp->tcp.flags&(TCP_FLAG_SYN|TCP_FLAG_ACK|TCP_FLAG_RESET|TCP_FLAG_FIN))!=TCP_FLAG_SYN) break; //Это не SYN
	      IPcpy(&s->rip,&ip->ip.src_ip);
	      s->rport=tcp->tcp.src_port;
	      MACcpy(s->dst_mac+ETH_HWA_LEN,__get_eth_root(ip)->hdr.src_mac+ETH_HWA_LEN);
	      goto L_FOUND_SOCKET;
	    }
	    else
	    {
	      //Этот сокет не в состоянии LISTENING, проверяем необходимость добавления в очередь
	      if (s->rport==tcp->tcp.src_port && !IPcmp(&s->rip, &ip->ip.src_ip) /*s->rip==ip->ip.src_ip*/) goto L_FOUND_SOCKET; //Найден сокет в открытом состоянии
	      //Попробуем добавить ломящегося клиента в очередь (только если сокет не закрыт (и только серверный сокет))
	      if (
		  (tcp->tcp.flags&(TCP_FLAG_SYN|TCP_FLAG_ACK|TCP_FLAG_RESET|TCP_FLAG_FIN))==TCP_FLAG_SYN
		    &&
		  (s->state!=TCP_STATE_CLOSED)
		    //&&
		  //(s->type&TCP_TYPE_SERVER)
		    )
	      {
		//Сначала проверяем, а не ожидается ли такой в очереди
		TCP_QUE_CLIENT *p;
		UREG i=CallTCPhook(TCP_EVENT_QUE_GET,0,(TCP_QUE_CLIENT**)(&tcp->tcp.ackno),s);
		p=*((TCP_QUE_CLIENT **)(&tcp->tcp.ackno));
		while(i)
		{
		  if (p->rport==tcp->tcp.src_port && !IPcmp(&p->rip,&ip->ip.src_ip)) goto L_TCPDROP; //Такой мы уже запомнили
		  p++;
		  i--;
		}
		//Используем tcp.ackno как переменную для хранения адреса TCP_QUE_CLIENT
		if (CallTCPhook(TCP_EVENT_QUE_ALLOC,0,(TCP_QUE_CLIENT**)(&tcp->tcp.ackno),s))
		{
		  p=*((TCP_QUE_CLIENT **)(&tcp->tcp.ackno));
		  MACcpy(p->dst_mac+ETH_HWA_LEN,__get_eth_root(ip)->hdr.src_mac+ETH_HWA_LEN);
		  p->mss=ExtractMSS(tcp);
		  p->rport=tcp->tcp.src_port;
		  IPcpy(&p->rip,&ip->ip.src_ip);
		  //p->seqno=tcp->tcp.seqno;
		  IPcpy(&p->seqno,&tcp->tcp.seqno);
		  goto L_TCPDROP;
		}
	      }
	    }
	  }
	  s=s->next;
	}
	//Не нашли ничего подходящего
        goto L_FAST_FIN;
      L_FOUND_SOCKET:
	if (tcp->tcp.flags & TCP_FLAG_RESET)
	{
	  //Обрабатываем флаг сброса
	  switch(s->state)
	  {
	    //Для всех состояний, кроме SYN-SENT, все сегменты с сигналом перезагрузки (RST) проходят проверку полей SEQ.
	  default:
	    //Сигнал перезагрузки признается, если его номер очереди попадает в окно
	    if (cmp_S_A(tcp,s)) goto L_TCPDROP;
	    break;
	    //Если получатель находился в состоянии LISTENING
	  case TCP_STATE_LISTENING:
	    goto L_TCPDROP; //то он игнорирует сигнал
	    //В состоянии SYN SENT 
	  case TCP_STATE_SYN_SENT:
	    //сигнал RST признается, если поле ACK подтверждает ранее сделанную посылку сигнала SYN.
	    if (cmp_A_S(tcp,s)!=1) goto L_TCPDROP;
	    break;
	  }
	  if (ClosingTCP(TCP_EVENT_ABORT,s)) s->state=TCP_STATE_QUE_CLIENT;//Будем имитировать приход SYN-пакета
	L_TCPDROP:
	  break;
	}
	if (tcp->tcp.flags & TCP_FLAG_SYN)
	{
	  //Обрабатываем флаг SYN
	  switch(s->state)
	  {
	  default:
	    if (cmp_S_A(tcp,s)||(len>ntohs(s->win))) goto L_TCPDROP;//break; //Не попадаем в окно приема
	    if (ClosingTCP(TCP_EVENT_ABORT,s)) s->state=TCP_STATE_QUE_CLIENT;//Будем имитировать приход SYN-пакета
	    goto L_SENDRESET;
	  case TCP_STATE_SYN_SENT:
	    //Нам ответили, проверяем
	    if (cmp_A_S(tcp,s)!=1) goto L_TCPDROP; //Не наш
	    inc32cpy(&s->ACKNO,&tcp->tcp.seqno);
	    s->rmss=ExtractMSS(tcp);
	    CallTCPhook_null(TCP_EVENT_CONREQ,s);
	    goto L_CONNECTED;
	  case TCP_STATE_LISTENING:
	    if (tcp->tcp.flags & TCP_FLAG_ACK) goto L_SENDRESET;
	    CallTCPhook_null(TCP_EVENT_CONREQ,s);
	    set_state_SYN_RECEIVED(s,&tcp->tcp.seqno,ExtractMSS(tcp));
	    break;
	  }
	}
	else
	{
          #ifdef CONSOLE_DEBUG         
          _print_fstr("\r\nTCP");
          _print_num("FLG:",(UINT16)BufferPKT);
          #endif
	  int d;
	  if (!(tcp->tcp.flags & TCP_FLAG_ACK)) goto L_TCPDROP; //Drop packets without ACK
	  if (s->state==TCP_STATE_SYN_SENT) goto L_TCPDROP;
	  d=cmp_S_A(tcp,s);
	  if (len>ntohs(s->win)) goto L_TCPACK;
//	  if (d==1 && s->state==TCP_STATE_FINW1) goto L_LINUX_BUG;
          if (d && s->state==TCP_STATE_FINW1) goto L_LINUX_BUG;
//          ->>> Вот тут надо сделать следующую фигню <<<-
 //         собственно фигня:
             #ifdef CONSOLE_DEBUG
            _print_num("\r\nd:",d);
            _print_num("ID:",_READ_U16_REV (((UINT8*)(&ip->ip.id))));
            #endif
          if (d>0)
          {
            
            //Слишком свежий пакет
            if (BufferPKT)//флаг, что обрабатываемый пакет достали из буфера
            {
                #ifdef CONSOLE_DEBUG
                //_print_num("\r\nSave'n'drop#",_READ_U16_REV (((UINT8*)(&tcp->tcp.seqno))+2)); // дампим записанный SEQ#
                _print_fstr("Save'n'drop"); 
                #endif
                ETHfifo_write(ETH_PKT.hdr.dst_mac,ETH_PKT_len, FIFO_pkt_TS);
                //сохраняем пакет в буфер; //Ибо его достали в основном цикле, таймаут не трогаем
                goto L_TCPDROP; //Нафиг пока
            }
                #ifdef CONSOLE_DEBUG
                //_print_num("\r\nSave'n'ACK#:", _READ_U16_REV (((UINT8*)(&tcp->tcp.seqno))+2)); // дампим записанный SEQ#
                _print_fstr("Save'n'ACK"); 
                #endif
               //инициализируем таймаут для пакета, который пойдет в буфер;
               //сохраняем пакет в буфер; //Ибо его достали в основном цикле
                ETHfifo_write(ETH_PKT.hdr.dst_mac,ETH_PKT_len, (UINT8)(irs_high+20));
                goto L_TCPACK;
          }          
	  if (d) goto L_TCPACK;
	  switch(s->state)
	  {
	  case TCP_STATE_SYN_RECEIVED:
	    if (cmp_A_S(tcp,s)!=1) //Если же подтверждение в сегменте оказалось неприемлемым,
	      goto L_SENDRESET; //то сформировать сегмент с сигналом перезагрузки
	  L_CONNECTED:
	    inc32(&s->SEQNO,1);
	    s->state=TCP_STATE_CONNECTED;
	    s->flags=TCP_FLAG_ACK;
//	    s->type&=~TCP_TYPE_FLAG_DELAYED_ACK;
             s->type|=TCP_TYPE_FLAG_DELAYED_ACK;
	    goto L_TCP_CHECK_INCOMING;
	  case TCP_STATE_CONNECTED:
	    if ((d=cmp_A_S(tcp,s))<0) goto L_TCPDROP; //Очень древнее подтверждение, или уж очень новое
	    {
	      unsigned int i;
	      unsigned int o;
	      i=s->sended_len;
	      if (d>i)
	      {
#ifdef IPSEQ_DEBUG
		IPSEQ=1;
#endif
		 goto L_TCPACK; //Мы столько никогда не посылали, посылаем ACK
	      }
	      i-=d;
	      s->sended_len=i; //Уменьшаем общее количество посланного на количество подтвержденного
	      //Расчитываем новое смещение относительно SEQ
	      o=i=s->send_disp;
	      if (d>i)
	      {
		i=0;
	      }
	      else
	      {
		i-=d;
	      }
	      s->send_disp=i;
	      if (i==0&&o!=0)
	      {
		//Пробуем расчитать новый RTT
		UREG i=s->timer_init;
		if (s->timer>(__multiply_unsigned(i,64)>>8))
		{
		  if (i==255)
		  {
		    //Специальный случай
		    i=128;
		  }
		  else
		    i>>=1;
		  if (i<TCP_TIMER_INITV) i=TCP_TIMER_INITV;
		}
		s->timer=i;
		s->timer_init=i;
	      }
	    }
	    inc32i(&s->SEQNO,d);
	    {
	      UREG i;
	      for(;;)
	      {
		i=255; if (d<i) i=d; d-=i;
		if (!i) break;
		CallTCPhook(TCP_EVENT_ACK,i,NULL,s);
	      }
	    }
	  L_TCP_CHECK_INCOMING:
	    {
	      UREG is_data_for_send;
	      {
		//Окно на другой стороне
		unsigned int rw=ntohs(tcp->tcp.window);
		if (rw>(TCP_MAX_DATA_LEN*4)) rw=TCP_MAX_DATA_LEN*4; //Не надо больше 4х пакетов за раз ;)
		s->rwin=rw;
	      }
	      inc32i(&s->ACKNO,len);
	      {
		unsigned int d=0;
		unsigned int j;
		do
		{
		  j=len-d; //Сколько осталось
		  if (j>255) j=255; //Слишком много ;)
		  is_data_for_send=CallTCPhook(TCP_EVENT_DATA,j,tcp->data-TCP_HEADER_LEN+tcp->tcp.hlen+d,s);
		  j=len-d;
		  if (j>255) j=255;
		  d+=j;
		}
		while(d!=len);
	      }
	      if (tcp->tcp.flags & TCP_FLAG_FIN)
	      {
		inc32(&s->ACKNO,1); //Подтвердим FIN
		s->state=TCP_STATE_FINW1;
	      }
	      if (s->state==TCP_STATE_FINW1)
	      {
		//Кто-то инициировал закрытие сокета
		s->flags=TCP_FLAG_ACK|TCP_FLAG_FIN;
                s->timer=0;
                //Попробуем послать чуть позже
		goto L_TCPDROP; //А вдруг там в другом пакете уже нас FIN дожидается?
	      }
              if (!s->win) goto L_TCPACK; //Если окно стало равным 0, посылаем подтверждение в принудительном порядке
	      if (is_data_for_send&&(s->send_disp==0)) //Если запрос передачи - передаем через REQ
	      {
		//Кроме того, необходимо, чтобы у нас не было неподтвержденных пакетов
		s->txreq=TCP_TXREQ_SEND; //Продолжаем посылать
	      }
	      else 
	      {
		if (len)
		{
		  s->timer=s->timer_init; //Нам пришли данные, начинаем отсчет сначала
		  UREG fda=s->type;
		  //Надо подтвердить данные, проверяем на delayed ack
		  if (fda&TCP_TYPE_FLAG_DELAYED_ACK)
		  {
		    //Принудительный ответ на каждый второй пакет с данными
		    s->type=fda&~TCP_TYPE_FLAG_DELAYED_ACK;
		    //Отключаем таймер delayed ack
		    if (s->txreq>TCP_TXREQ_SEND) s->txreq=TCP_TXREQ_NONE;
#ifdef IPSEQ_DEBUG
		    IPSEQ=4;
#endif
		    goto L_TCPACK;
		  }
		  s->txreq=TCP_TXREQ_DACK2;
		  s->type=fda|TCP_TYPE_FLAG_DELAYED_ACK;
		}
	      }
	    }
	    goto L_TCPDROP;
	  case TCP_STATE_CLOSED:
	    goto L_SENDRESET;
	  case TCP_STATE_FINW1:
	    if ((!(tcp->tcp.flags & TCP_FLAG_FIN)) && cmp_A_S(tcp,s)<=0) goto L_TCPDROP;//Если же подтверждение в сегменте оказалось неприемлемым,
	  L_LINUX_BUG:
	    if (ClosingTCP(TCP_EVENT_CLOSE,s)) s->state=TCP_STATE_QUE_CLIENT;//Будем имитировать приход SYN-пакета
	    if (tcp->tcp.flags & TCP_FLAG_FIN) goto L_FAST_FIN;
	    goto L_TCPDROP;
	  default:
            {
            L_SENDRESET:
            L_FAST_FIN:
              if(tcp->tcp.flags & TCP_FLAG_ACK)
              {
                swapmem(&tcp->tcp.seqno,4);
                if (tcp->tcp.flags & TCP_FLAG_FIN)
                {
                  inc32(&tcp->tcp.ackno,1);
                  tcp->tcp.flags = TCP_FLAG_ACK | TCP_FLAG_FIN;
                }
                else
                {
                  tcp->tcp.flags = TCP_FLAG_RESET;
                }
              }
              else
              {
                inc32cpy(&tcp->tcp.ackno,&tcp->tcp.seqno);
                tcp->tcp.seqno=0;
                tcp->tcp.flags = TCP_FLAG_RESET | TCP_FLAG_ACK;	
              }
              swapmem(&tcp->tcp.src_port,2);
              ptcp=&tcp->tcp;
            }
	    goto L_SENDTCP;
	  }
	}
      L_TCPACK:
	ptcp=(TCP_HEADER*)&s->lport;
      L_SENDTCP:
	len=0; //Нет данных
	//Костыль, копирование ip_dest из старого в новый
	IPcpy(&(tcpf=(TCP_FRAME*)__get_eth_root(ip))->ip.dest_ip,&ip->ip.src_ip);
      L_SENDTCPWITHDATA:
	IPcpyIP(&tcpf->ip.src_ip); //Для правильного расчета контрольной суммы псевдозаголовка
	tcpf->ip.ttl=0;
	tcpf->ip.proto=IP_TCP;
	netw_memcpy(&tcpf->tcp.src_port,ptcp,16);
	tcpf->tcp.hlen=(TCP_HEADER_LEN << 2) & 0xfc;
	if (s)
	{
	  unsigned int d=s->send_disp;
	  if (s->txreq==TCP_TXREQ_DUPACK)
	  {
	    d=0;
	    s->txreq=TCP_TXREQ_NONE;
	  }
	  inc32i(&tcpf->tcp.seqno,d);
	  d=s->send_disp;
	  d+=len;
	  s->send_disp=d;
	  if (d>s->sended_len) s->sended_len=d;
	  //Если посылаем SYN, то добавляем опцию MSS
	  if (tcpf->tcp.flags & TCP_FLAG_SYN)
	  {
	    tcpf->tcp.hlen=((TCP_HEADER_LEN+4) << 2) & 0xfc;
	    tcpf->data[0]=0x02;
	    tcpf->data[1]=0x04;
	    tcpf->data[2]=(TCP_MAX_DATA_LEN)>>8;
	    tcpf->data[3]=(TCP_MAX_DATA_LEN)&0xFF;
	    len+=4;
	  }
	}
	len+=TCP_HEADER_LEN;
	tcpf->ip.checksum=htons(len);
	tcpf->tcp.urgent=0;
	tcpf->tcp.checksum=0; //~IPChecksum((UINT16*)&tcpf->ip.ttl,12);
	tcpf->tcp.checksum=IPChecksum((UINT16*)&tcpf->ip.ttl,len+12);  //IPChecksum((UINT16*)&tcpf->tcp,len);
	ip=(IP_FRAME *)tcpf;
      L_CREATEIP:
	len+=IP_HEADER_LEN;
	MACcpy(ip->eth.dst_mac+ETH_HWA_LEN,ip->eth/*ep->hdr*/.src_mac+ETH_HWA_LEN); //Временно, тут нужен ARP кеш
	MACcpy(ip->eth.src_mac+ETH_HWA_LEN,MAC0+ETH_HWA_LEN);
	ip->eth.type=ETH_PROTOCOL_IP;
	/* Make IP header	*/
	ip->ip.vihl = IP_DEF_VIHL;
	ip->ip.tos = IP_DEF_TOS;
	ip->ip.tlen = htons(len);
#ifdef USE_IPSEQ
#ifdef IPSEQ_DEBUG
	ip->ip.id = htons(IPSEQ);
	IPSEQ=0;
#else
	ip->ip.id = htons(++IPSEQ);
#endif
#else
	ip->ip.id=0;
#endif
	ip->ip.frags = htons(IP_DONT_FRAGMENT);
	ip->ip.ttl = 0x80;	/* ttl */
	ip->ip.checksum = 0;	/* prepare checksum */
	IPcpyIP(&ip->ip.src_ip);	/* My IP */
	/* Make checksum */
	ip->ip.checksum = IPChecksum((UINT16 *)&ip->ip, IP_HEADER_LEN);	/* create checksum */
	len+=ETH_HEADER_LEN;
	ep=(ETH_FRAME *)ip;
      L_SENDETH:
	if (len<60) len=60;
	//CRC32((UINT8*)ep,len,1);
	ETH_PKT_len=len;//+4;
	ETH_PKT_mode=16;
	continue;
      }
      break;
    }
    //Освобождение пакета
    ETH_PKT_mode=0;
    ETH_STOP_BACK_PRESSURE();
  }
  // Проверка таймера
  if (TIMER_200ms)
  {
    TIMER_200ms=0;
    s=((TCP_SOCK *)&QUE_sock);
    while((s=s->next))
    {
      UREG i;
      if (s->timer_init&&(i=s->timer)) s->timer=i-1;
      if (s->txreq>TCP_TXREQ_SEND) s->txreq--;
    }
  }
  s=((TCP_SOCK *)&QUE_sock);
  while((s=s->next))
  {
    // Обработка чита
    if (s->async_req) CallTCPhook_null(TCP_EVENT_ASYNC_REQ,s);
    if (s->state==TCP_STATE_QUE_CLIENT || //Есть клиент в очереди
	s->txreq==TCP_TXREQ_SEND) //Запрос передачи
    {
      //s->timer_init=TCP_TIMER_INITV>>1; //Таймер начинает считать сначала
      goto L_TRY_CHEAT;
    }
    // Обработка таймера сокетов (перепосылка пакета)
    if ((!s->timer_init) || (s->timer)) continue;
    switch(s->state)
    {
    case TCP_STATE_CONNECTING:
      if (s->timer_init==255)
      {
	ClosingTCP(TCP_EVENT_ABORT,s); //Нет соединения
	continue;
      }
    case TCP_STATE_SYN_SENT:
    case TCP_STATE_CONNECTED:
    case TCP_STATE_SYN_RECEIVED:
    case TCP_STATE_FINW1:
      s->timer=1; //Если вдруг не получится занять буфер, будем повторять снова
    L_TRY_CHEAT:
      {
	UREG pp;
	__disable_interrupt();
	pp=ETH_PKT_mode;
	ETH_PKT_mode=1;
	__enable_interrupt();
	if (pp) goto L_CHECK_INCOMING;
	ETH_STOP_BACK_PRESSURE();
	ip=(IP_FRAME*)(&ETH_PKT);
      }
      tcpf=(TCP_FRAME*)ip;
      if (s->state==TCP_STATE_CONNECTING)
      {
	ARP_FRAME *apo;    
	UREG f=s->timer_init;
	f<<=1;
	if (f>=(TCP_TIMER_INITV<<3))
	{
	  s->timer_init=255; //Последний выстрел
	}
	else
	  s->timer_init=f;
	s->timer=f;
	//Теперь готовим ARP-запрос
	apo=(ARP_FRAME*)(&ETH_PKT);
	apo->arp.hw_type=ARP_HARDWARE;
	apo->arp.proto=ARP_ETHCODE;
	apo->arp.hw_len=ETH_HWA_LEN;
	apo->arp.protocol_len=ETH_PRA_LEN;
	apo->arp.opcode=ARP_REQUEST;
	apo->eth.type=ETH_PROTOCOL_ARP;
	MACcpy(apo->arp.src_mac+ETH_HWA_LEN,MAC0+ETH_HWA_LEN);
	IPcpyIP(&apo->arp.src_ip);
	netw_memset(apo->arp.dst_mac,0,ETH_HWA_LEN);
        apo->arp.dst_ip=s->ACKNO;
//      apo->arp.dst_ip=s->rip;
//       apo->arp.dst_ip=OUTG_IP[1];        
	MACcpy(apo->eth.src_mac+ETH_HWA_LEN,MAC0+ETH_HWA_LEN);        
//        MACcpy(apo->eth.dst_mac+ETH_HWA_LEN,MAC_GATE0+ETH_HWA_LEN);
	netw_memset(apo->eth.dst_mac,0xFF,ETH_HWA_LEN);
	len=ARP_FRAME_LEN;
	ep=(ETH_FRAME*)apo;
	goto L_SENDETH;    
      }
      if (s->state==TCP_STATE_QUE_CLIENT)
      {
	CallTCPhook(TCP_EVENT_QUE_GET,0,(TCP_QUE_CLIENT**)(&tcpf->tcp.ackno),s);
	TCP_QUE_CLIENT *p=(*((TCP_QUE_CLIENT **)(&tcpf->tcp.ackno)));
	MACcpy(s->dst_mac+ETH_HWA_LEN,p->dst_mac+ETH_HWA_LEN);
	IPcpy(&s->rip,&p->rip);
	s->rport=p->rport;
	CallTCPhook_null(TCP_EVENT_CONREQ,s);
	set_state_SYN_RECEIVED(s,&p->seqno,p->mss);
	CallTCPhook_null(TCP_EVENT_QUE_REMOVE,s);
      }
      IPcpy(&tcpf->ip.dest_ip,&s->rip);
      MACcpy(ip->eth.src_mac+ETH_HWA_LEN,s->dst_mac+ETH_HWA_LEN);
      ptcp=(TCP_HEADER*)&s->lport;
      len=0;
      //if (s->state==TCP_STATE_QUE_CLIENT) goto L_SENDTCPWITHDATA;
      if (s->timer_init==255)
      {
	//Закрываем по таймауту
	s->flags=TCP_FLAG_RESET; //Посылаем RST
	while(CallTCPhook_null(TCP_EVENT_QUE_REMOVE,s)); //Пока всех не выбросим из очереди, все равно все устарело
	ClosingTCP(TCP_EVENT_ABORT,s);
	goto L_SENDTCPWITHDATA;
      }
      {
	if (s->txreq==TCP_TXREQ_SEND) goto L_RESEND_OR_CHEAT;
	if (s->state==TCP_STATE_CONNECTED)
	{
	  s->send_disp=0; //Т.к. производим откат
	L_RESEND_OR_CHEAT:
	  s->type&=~TCP_TYPE_FLAG_DELAYED_ACK;
	  {
	    unsigned int d;
	    unsigned int sd=s->send_disp;
	    UREG j=TCP_EVENT_SEND;
	    if (sd==0) j=TCP_EVENT_REGENERATE; //Если смещение 0, регенерируем от ACK
	    d=s->rwin-sd;
	    UREG i;
	    len=0;
	    unsigned int nl=s->rmss;
	    //Проверим размер окна и MSS приемной стороны
	    if (d==nl)
	    {
	      //Этот пакет - последний для окна. После него не стоит продолжать передачу
	      s->txreq=TCP_TXREQ_STOP;
	    }
	    if (d>nl)
	    {
	      //Размер окна больше чем MSS
	      d=nl; //Количество данных для передачи равно MSS
	    }
	    i=255; if (d<i) i=d; d-=i;
	    nl=CallTCPhook(j,i,tcpf->data,s);
	    for(;nl;)
	    {
	      len+=nl;
	      i=255; if (d<i) i=d; d-=i;
	      if (!i) break;
	      nl=CallTCPhook(TCP_EVENT_SEND,i,tcpf->data+len,s);
	    }
	  }	  
	  {
	    UREG i=s->flags;
	    if (len) i|=TCP_FLAG_PUSH; else i&=~TCP_FLAG_PUSH;
	    s->flags=i;
	  }
	  if (s->txreq==TCP_TXREQ_STOP)
	  {
	    s->txreq=TCP_TXREQ_NONE;
	    goto L_STOP_TX;
	  }
          if (s->txreq==TCP_TXREQ_SEND)
          {
	  L_STOP_TX:
	    //Если есть данные, будем продолжать слать, нам пофиг ;)
            if (len==0) s->txreq=TCP_TXREQ_DUPACK; //Если данных нет, посылаем DUP для получения быстрого ACK на последний пакет
#ifdef IPSEQ_DEBUG
	    IPSEQ=2;
#endif
            goto L_SENDTCPWITHDATA; //Не надо трогать таймер после перепосылки
          }
	}
      }
      //Следующая итерация таймера, в 2 раза больше пауза
      {
	UREG f=s->timer_init<<1;
	if ((s->state==TCP_STATE_SYN_RECEIVED)||(s->state==TCP_STATE_SYN_SENT))
	{
	  if (f>=(TCP_TIMER_INITV<<3))
	  {
	    s->timer_init=255; //Последний выстрел
	  }
	  else
	    s->timer_init=f;
	}
	else
	{
	  if (f==(UREG)256)
	  {
	    f=255;
	  }
	  s->timer_init=f;
	}
	s->timer=f;
      }
#ifdef IPSEQ_DEBUG
      IPSEQ=3;
#endif
      goto L_SENDTCPWITHDATA;
    default:
      s->timer_init=0; //какого вообще хрена тут делает таймер ;)
      break;
    }
  }
}

#pragma diag_default=Ta006

#pragma diag_suppress=Ta006
__interrupt void TIMER_TASK(void)
{
  //Обработка таймеров
 // if ((++IRS)&0x1FFF) return; //Каждые 8192*25.6мкс=210мс
  if ((count_200ms_low+=8)) return;
  if ((count_200ms_high+=1)) return;
  irs_high++;
  TIMER_200ms=1;
  WAKEUP_ETH();
}
#pragma diag_default=Ta006

#pragma vector=EE_RDY_vect
__interrupt __raw void INT_ETH_PROCESS_PKT(void)
{
   
}

#pragma vector=TIMER0_COMP_vect
__interrupt __raw void INT_TIMER_DISPATCH(void)
{
  if (TIMER_TASK_LOCK) return;
  TIMER_TASK_LOCK=1;
//  DISABLE_INT_ETH();
  __enable_interrupt();
  if (TIMER_TASK_TMR)
  {
    TIMER_TASK_TMR=0;
    ((void(*)(void))TIMER_TASK)();
  }
  else
    TIMER_TASK_TMR=1;
  __watchdog_reset();
  __disable_interrupt();
  TIMER_TASK_LOCK=0;
  //if (ETH_TASK_LOCK) return;
  //if (ETH_TASK_WAKEUP) ENABLE_INT_ETH();
}

/*
void ExecuteETH(void)
{
  ENABLE_INT_ETH();
}
*/
