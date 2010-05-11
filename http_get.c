/* Name: http_get.c
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#include "nike_e.h"
#include "tcp.h"
#include "pgmspace.h"
#include "vs.h"
#include "compiler.h"
#include "FIFO.h"
#include "http_get.h"
#include "LCD.h"
#include "station.h"
#include "player.h"
#include "io.h"
//#include "network_addr.h"

extern UINT32 IP;
extern UINT32 MASK_IP;
extern volatile UREG PLAYER_STATE;
GET_SOCK get_sock;

volatile UINT8 GET_WINDOW_STATE=GET_WINDOW_STATE_UPDATE;

__z void _i2a(char *s, UINT16 v);

extern __eeprom UINT32 OUTG_IP[2];
//extern UINT32 OUTG_IP[2];

extern void *debug_addr;


#pragma inline = forced
UINT32 _MP3fifo_free(void)
{
	UINT32 free;
	if(MP3fifo_pRD > MP3fifo_pWR)
	{
		free = MP3fifo_pRD-MP3fifo_pWR;
	}
	else if(MP3fifo_pRD < MP3fifo_pWR)
	{
		free = MP3fifo_pEND-MP3fifo_pWR+1+MP3fifo_pRD-MP3fifo_pSTART;
	}
	else
	{
		free = MP3fifo_pEND-MP3fifo_pSTART+1;
	}
	return free-1;
}

void AddGETsocket(void)
{
  __x UREG GET_hook(UREG state, UREG len, UINT8 *data, TCP_SOCK *_s);
  get_sock.sock.type=TCP_TYPE_CLIENT;
  get_sock.sock.state=TCP_STATE_CLOSED;
  get_sock.sock.hook=GET_hook;
  get_sock.sock.win=htons(SPEC_MAX_WIN);
  debug_addr=&get_sock;
  AddTCPsocket(&get_sock.sock);
}
extern volatile UREG is_playing;
extern volatile UREG stationNum;
void StartGET(void)
{  
  if (get_sock.sock.state==TCP_STATE_CLOSED) 
  {
    MP3fifo_pWR= MP3fifo_pRD= MP3fifo_pSTART; 
    ETHfifo_pWR= ETHfifo_pRD= ETHfifo_pSTART; 
    ETHfifo_CNT=0;
    GET_WINDOW_STATE=GET_WINDOW_STATE_UPDATE;
    PLAYER_STATE=PLAYER_STATE_STOPED;
    UINT32 IPsrc=station_list[stationNum].IP;
    UINT16 port = station_list[stationNum].port;  
    if ((IP&MASK_IP)==(IPsrc&MASK_IP)) get_sock.sock.ACKNO=IPsrc;// ��� �����     
     else get_sock.sock.ACKNO=OUTG_IP[1]; // ���� ����� ����
    TCPconnect(&get_sock.sock,IPsrc,port);     
  }
}

void StopGET(void)
{
  //is_playing=0;
  CloseTCPsocket(&get_sock.sock);
}

enum HTTP_GET_STATE
{
  _GET_STOP=0, //������������
  _GET_CONREQ, //������ ����������
  _GET_SENDREQ, //������� HTTP-�������
  _GET_WAITACK, //�������� ������������� �������
  _GET_HEADER, //����� ���������
  _GET_BODY, //����� ����
  _GET_METADATA //����� ����������
};

extern __x_z UREG stricmp_P(const char *s, const char __flash *d);

static __z UINT16 a2i(const char *s)
{
  UREG c;
  UINT16 i=0;
  for(;;)
  {
    c=*s++;
    if (c==' ') continue;
    c-='0';
    if (c>9) return i;
    i*=10;
    i+=c;
  }
}

static __x UREG GET_hook_DATA_RX(UREG len, UINT8 *data, GET_SOCK *s)
{
  UREG c;
  UREG state=s->state;
  UINT32 pWR;      
  UINT16 bl;
  pWR = MP3fifo_pWR; // ������    
    
  if (state==_GET_BODY) // ������� ���� - ���� ���� ����� ����
  {    
    if (PLAYER_STATE!=PLAYER_STATE_PLAYING) 
    {      
      if (PLAYER_STATE!=PLAYER_STATE_BUFFERING) LCD_fprintline(1,"Buffering");     
      PLAYER_STATE=PLAYER_STATE_BUFFERING;
    }
L_BODY:
  {
    UREG addr;
    FM_CS_ENABLE();   // �������� FRAM
    SPDR=FM_WREN;     // ��������� ������
    while (!(SPSR & (1<<SPIF)));               
    FM_CS_DISABLE();
    FM_CS_ENABLE();   // �������� FRAM
    SPDR=FM_WRITE;    // �������� - "������"
    addr=(UREG)(pWR>>16);
    while (!(SPSR & (1<<SPIF)));
    SPDR=addr;  // ��������� ����� - ������ [24..16]
    addr=(UREG)(pWR>>8);
    while (!(SPSR & (1<<SPIF)));       
    SPDR=addr;  // ��������� ����� - ������ [15..8]
    addr=(UREG)(pWR);
    while (!(SPSR & (1<<SPIF)));
    SPDR=addr;    //��������� ����� - ������ [7..0]    
    while (!(SPSR & (1<<SPIF)));    
  }
L_BODY2:    
    //������� ��������� ������ ����
    if (!len) goto GET_hook_DATA_RX_exit;
    bl=s->block_length; // "��������" ������ ����� ����-������        
    do
    {            
      UREG c=*data++; //������ ��������� ���� ����
      if (bl) //������ ���� �� �����, ��� ���������� ������������
      {	
	if (!(--bl))
	{
	  //���� ����������, � c - ������ ����������
          if (!c)                          // ����� ���������� 0 (�� ���)
          {
//             META_len=0;
//             pMETA_buf=&META_buf[0];
             bl=s->metadata_interval;
             s->block_length=bl;
             continue;
          }
          s->state=state=_GET_METADATA;
          s->block_length=c*16;           // ���������� � �������� ����� ����� � ��3 �������
	  len--;
          LCD_putMETA(0,0); 
	  goto L_METADATA;                // ���� �� ��������� ����������
	}
      }      
        UREG f=0;
        SPDR = c; // �������� ���� � FIFO      
        pWR += 1; // �������� ������        
        if (pWR > MP3fifo_pEND) f=1;
	while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished 
	if (f) 
        {
          UREG addr;
          pWR=MP3fifo_pSTART; // ����� �� ������� ������ - ���� ������������� ����                
          FM_CS_DISABLE();
          FM_CS_ENABLE();   // �������� FRAM
          SPDR=FM_WREN;     // ��������� ������
          while (!(SPSR & (1<<SPIF)));               
          FM_CS_DISABLE();
          FM_CS_ENABLE();   // �������� FRAM
          SPDR=FM_WRITE;    // �������� - "������"
          addr=(UREG)(pWR>>16);
          while (!(SPSR & (1<<SPIF)));
          SPDR=addr;  // ��������� ����� - ������ [24..16]
          addr=(UREG)(pWR>>8);
          while (!(SPSR & (1<<SPIF)));       
          SPDR=addr;  // ��������� ����� - ������ [15..8]
          addr=(UREG)(pWR);          
          while (!(SPSR & (1<<SPIF)));
          SPDR=addr;    //��������� ����� - ������ [7..0]    
          while (!(SPSR & (1<<SPIF)));  
        }	
    }
    while(--len);
    s->block_length=bl;    

GET_hook_DATA_RX_exit:
    
    FM_CS_DISABLE();
    MP3fifo_pWR = pWR;        
    // �������� �����
    // ������ ����������� ���������� ����� � ������ FRAM 
    // ������������� ������ ���� ������ ������ ����� ��������� ���� � ������ FRAM
    UINT32 freeFIFO=_MP3fifo_free();
    if (freeFIFO < TCP_MAX_DATA_LEN)
    {   
      // ������� ��� � FIFO ����� ��� ��� - �������� ����
      // �������������� ����� ������ ������ � �������� �����
      GET_WINDOW_STATE=GET_WINDOW_STATE_ZERO;
      freeFIFO=0; 
      #ifdef CONSOLE_DEBUG
      _print_fstr("\r\nZEROWIN");
      #endif
    }    
    else if (freeFIFO>=SPEC_MAX_WIN) freeFIFO=SPEC_MAX_WIN;
//    else if (freeFIFO>=(0xFFFF-512)) freeFIFO=0xFFFF;
//    else freeFIFO+=512;
    s->sock.win=htons((UINT16)(freeFIFO)); // ������ ������ ����
    return 0; // ������ ���� ��������� - �������...
  }
  if (state==_GET_METADATA)
  {
  L_METADATA:
    //������� ��������� ����������
    if (!len) goto GET_hook_DATA_RX_exit;//{FM_CS_DISABLE(); return 0;}
    bl=s->block_length;
    do
    {
      if (!(--bl))
      {
	//���� ����������, � c - ������ ����������
	s->state=state=_GET_BODY;
	s->block_length=s->metadata_interval;
	len--;
	goto L_BODY2;
      }
      UREG c=*data++;
     //��� ����� ����������� ����� ����������, ���� ����
     //UDR=c; // ������� � �������
//     if (META_len++<31) *pMETA_buf++=c;
     LCD_putMETA(1,c); 
    }
    while(--len);
    s->block_length=bl;
  }
  // ���� �����... 
  if (!len) goto GET_hook_DATA_RX_exit;//{FM_CS_DISABLE();return 0;}
  UREG pos=s->header_pos; // pos - ������� ������� � ������-���������
  // ���������� �� ����������
  do 
  {
    c=*data++;
    switch(state)
    {
        case _GET_HEADER: 
        // ���� ������ ���� ����� ��������� (Header)  
          if (c==10) // ������� '\n'  (����� ������)
          {      
            s->req[pos]=0;
            if (pos) // ���� ������� � ������ �� �������
            {       
              if (!stricmp_P(s->req,"icy-metaint")) // ������� �������� ����� ����� ����������?
              {
                //������ ����� ����� �����������
                s->metadata_interval=a2i(s->req+12)+1; //�������� ������ ����� ���������� (��� ���������� ���� ������ ����������)
              }
              else if (!stricmp_P(s->req,"icy-br")) // ������� �������� ��������
              {
                //�������
                LCD_putBR(s->req+7);
              }
            }   
            else // ������� \r\n (����� ����������, ������������� �� ����)
            {              
              //__no_operation();
              s->state=state=_GET_BODY;
              s->block_length=s->metadata_interval; // ���������� ������ ����� ����� �����������
              len--;
              goto L_BODY; // �������� �� ��������� ����������
            }
            pos=0; //�������� ����� ���������� ������
            break;
         }
         if (c==13) break; //����������
         if (pos<(sizeof(s->req)-1))
         {
            //���� ���� ���� ��������� �������, ��������� �������� ���������
           if (c==':') c=0;
           s->req[pos++]=c;
          }
        break;
    }
  }
  while(--len);
  s->header_pos=pos;
  s->state=state;
  goto GET_hook_DATA_RX_exit;
//  FM_CS_DISABLE();
//  return 0;
}

//extern volatile UREG stop_req;

static __x UREG GET_hook(UREG state, UREG len, UINT8 *data, TCP_SOCK *_s)
{
  GET_SOCK *s=(GET_SOCK *)_s;
  UINT32 freeFIFO;
//  UREG i;
//  UREG j;
  switch(state)
  {
  case TCP_EVENT_CLOSE:
  case TCP_EVENT_ABORT:
    s->state=_GET_STOP;
    s->header_pos=0;
    s->block_length=0;
    s->metadata_interval=0;
    PLAYER_STATE=PLAYER_STATE_STOPED;
    if (PLAYER_STATE==PLAYER_STATE_STOPED) 
    {
      LCD_fprintline(1,"Stoped");     
      LCD_softCLR();     
    }
    return 0;
  case TCP_EVENT_CONREQ:
    s->state=_GET_CONREQ;
    return 0;
  case TCP_EVENT_ASYNC_REQ:
    s->sock.async_req=0;
    freeFIFO=MP3fifo_free();
    //if (freeFIFO<SPEC_MAX_WIN)
    if (freeFIFO<4*TCP_MAX_DATA_LEN)
    {
      // ���� ��� ����� ��� ������ ������������ ���� �� ������ ����������� �����      
      GET_WINDOW_STATE=GET_WINDOW_STATE_ZERO;
      #ifdef CONSOLE_DEBUG
      _print_fstr("\r\nNO_Async_start");
      #endif
      return 0;
    }
    #ifdef CONSOLE_DEBUG
    _print_fstr("\r\nAsync_start");
    #endif
    GET_WINDOW_STATE=GET_WINDOW_STATE_UPDATE;
    if (freeFIFO>=SPEC_MAX_WIN) freeFIFO=SPEC_MAX_WIN;
    s->sock.win=htons((UINT16)freeFIFO); // ������� ����������� ����
    s->sock.txreq=TCP_TXREQ_SEND;
    return 0;
  case TCP_EVENT_DATA:
    if (PLAYER_STATE==PLAYER_STATE_STOPREQ)
    {
      StopGET();
      return 0;
    }
    if (s->state==_GET_CONREQ)
    {
      s->state=_GET_SENDREQ;
      return 1; //������ ��������
    }
    if (s->state<_GET_HEADER) return 0;
    return GET_hook_DATA_RX(len,data,s);
  case TCP_EVENT_ACK:
    //������������� ������
    if (PLAYER_STATE==PLAYER_STATE_STOPREQ)
    {
      StopGET();
      return 0;
    }
    if (s->state==_GET_WAITACK&&len)
    {
      s->state=_GET_HEADER;
    }
    return 0;
  case TCP_EVENT_REGENERATE:
    if (s->state==_GET_WAITACK) s->state=_GET_SENDREQ;
  case TCP_EVENT_SEND:
  if (s->state==_GET_SENDREQ)
    {
      s->state=_GET_WAITACK;
      static __flash char req1[]="GET /";     
      static __flash char req2[]=" HTTP/1.0\r\nHost:";        
      static __flash char req3[]="\r\nIcy-MetaData:1\r\nUser-Agent: uEthRadio/0.1\r\nConnection: close\r\n\r\n";
      UREG i=sizeof(req1)-1;
      char __flash *s=req1;
      UINT8 *d=data;
      do
      {
	*d++=*s++;// "GET/"
      }
      while(--i);
      UREG s1=0;      
      char __eeprom *es;
      es=station_list[stationNum].req;
      
       while(*es)      
      {
            *d++=*es++;//  "������"
            s1++;
       };
          
      i=sizeof(req2)-1;
      s=req2;
      do
      {
	*d++=*s++;// "/HTTP1.0 Host:"
      }
      while(--i);     
      
      __no_init char i2a_buf[4];
      char *sIP = i2a_buf;
      UINT32 sockIP = station_list[stationNum].IP;
      UREG s2=0;
      i=0;
      
      while (i++<4)
      {
        _i2a(i2a_buf, (UREG)(sockIP));
        do
        {
  	  *d++=*sIP++;// "IP"
          s2++;
        }
        while (*sIP);                
        sIP=i2a_buf;
        if (i<4) {*d++='.';s2++;}
        sockIP=sockIP>>8;
      };           
      
      i=sizeof(req3)-1;
      s=req3;
      do
      {
	*d++=*s++;// ""
      }
      while(--i);                  
      return (sizeof(req1)-1)+s1+s2+(sizeof(req2)-1)+(sizeof(req3)-1);
    }
    return 0;
  }
  return 0;
}