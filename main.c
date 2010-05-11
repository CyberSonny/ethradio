/* Name: main.c
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#include "nic.h"
#include "network.h"
#include "nike_e.h"
#include "tcp.h"
#include "pages.h"
#include "pgmspace.h"

#include "http_srv.h"
#include "io.h"

#include "vs.h"
#include "lcd.h"
#include "FIFO.h"
#include "http_get.h"
#include "station.h"
#include "kb.h"
#include "player.h"

//#include "revision.c"

//#define TIFR_OCF1B  TIFR&(1<<OCF1B)


//char i2a_buf[5];
__no_init UINT16  MP3indic;
UREG http_q_c_pos;
TCP_QUE_CLIENT http_q_c[4];

__root __eeprom char DUMMY_EEPROM@ 0 =0x55 ;
//volatile char EEPROM_FAULT; //Глобальная ошибка EEPROM
volatile char EEPROM_RESTORED; //Восстановлено 2 из 3

extern UINT8 MAC0[];
extern UINT8 MAC_GATE0[];
extern UINT32 IP;
extern UINT32 MASK_IP;

__eeprom UINT32 OUTG_IP[2]={IP2UINT32(192,168,113,2),IP2UINT32(192,168,113,1)};
__eeprom UINT32 MASK_IP_EEPROM={IP2UINT32(255,255,255,0)};
__eeprom char MAC_EEPROM[ETH_HWA_LEN]={0x00,0x13,0x12,0x19,0x80,0x28};
__eeprom UINT32 IP_EEPROM=IP2UINT32(192,168,113,3);
__eeprom char HTTP_LOGIN[16]="root";
__eeprom char HTTP_PASS[16]="root";

#pragma inline = forced
UINT32 _MP3fifo_len(void)
{
	UINT32 len;

	if(MP3fifo_pRD > MP3fifo_pWR)
	{
		len = MP3fifo_pEND-MP3fifo_pRD+1+MP3fifo_pWR-MP3fifo_pSTART;
	}
	else if(MP3fifo_pRD < MP3fifo_pWR)
	{
		len = MP3fifo_pWR-MP3fifo_pRD;
	}
	else
	{
		len = 0;
	}

	return len;
}

__no_init UINT8 VS_BUFFER32 [32];

#pragma inline = forced
UINT8 _MP3fifo_read32()
{
	UINT32 pRD;
	UINT8 c;
        UREG d;
        UINT8* data=&VS_BUFFER32[0];
//        MP3len=_MP3fifo_len();
        if (_MP3fifo_len()<32) return 0; // если в буфере <32 байт, то выходим с 0
//        MP3indic=(UINT16) (MP3len>>8);        
	pRD = MP3fifo_pRD;
        
        if ((pRD+32) <= MP3fifo_pEND)   // перехода на начало не будет                                     
        {                               //можно считывать все 32 байта подряд
       	  FM_CS_ENABLE();                              
	  SPDR=FM_READ;
          UREG addr;
          addr=(pRD>>16);
          while (!(SPSR & (1<<SPIF)));
	  SPDR=addr;
          addr=(pRD>>8);
          while (!(SPSR & (1<<SPIF)));
	  SPDR=addr;
          addr=(pRD);        
          while (!(SPSR & (1<<SPIF)));
      	  SPDR=addr;
       	  d = 0xff;
          while (!(SPSR & (1<<SPIF)));          
	  SPDR = d;
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 1
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 2
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 3
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 4
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 5
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 6
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 7
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 8
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 9
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //10
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //11
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //12
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //13
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //14
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //15
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //16
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 1
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 2
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 3
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 4
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 5
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 6
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 7
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 8
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; // 9
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //10
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //11
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //12
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //13
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //14
          while (!(SPSR & (1<<SPIF))); c = SPDR; SPDR = d; *data++ = c; //15
          while (!(SPSR & (1<<SPIF))); c = SPDR; *data = c; //16
	  FM_CS_DISABLE();     
          pRD += 32; // сместим указатель чтения на первый непрочитанный элемент (переноса не будет)
	  MP3fifo_pRD = pRD;                    
        }
        else // при чтении надо будет перенести указатель на старт - будем читать 2мя порциями
        {
          UINT8 i, len1, len2;          
          len1=MP3fifo_pEND-pRD+1; // первая порция - читаем до потолка
          len2 = 32-len1;// вторая порция - адрес на старт и продолжаем
          if (len1)
          {
            UREG addr;
            FM_CS_ENABLE();
            SPDR=FM_READ;
            addr = (UREG) (pRD>>16);
            while (!(SPSR & (1<<SPIF)));
	    SPDR= addr;
            addr= (UREG)(pRD>>8); 
            while (!(SPSR & (1<<SPIF)));
	    SPDR=addr;
            addr= (UREG)(pRD);
            while (!(SPSR & (1<<SPIF)));
      	    SPDR= addr;       
            d = 0xFF;
            while (!(SPSR & (1<<SPIF)));
            for (i=0; i<len1; i++)
            {
              SPDR = d;
              while (!(SPSR & (1<<SPIF)));
              *data++= SPDR;
            }
            FM_CS_DISABLE();
            pRD=MP3fifo_pSTART; // ставим начальный адрес (двигаем хвост)
            MP3fifo_pRD = pRD; // указатель чтения мп3 данных  
          } 
          // и если есть что читать - дочитываем
          if (len2)
          {
            UREG addr;
            FM_CS_ENABLE();
	    SPDR=FM_READ;
            addr=(UREG)(pRD>>16);
            while (!(SPSR & (1<<SPIF)));
	    SPDR=addr;
            addr=(UREG)(pRD>>8);  
            while (!(SPSR & (1<<SPIF)));
	    SPDR=addr;
            addr=(UREG)(pRD);        
            while (!(SPSR & (1<<SPIF)));
      	    SPDR=addr;
            pRD+=len2;// сдвигаем хвост на кол-во прочитанных байт и прибавляем 1 для указания на след. непрочитанный          
	    MP3fifo_pRD = pRD; // указатель чтения мп3 данных          
      	    d = 0xff;            
            while (!(SPSR & (1<<SPIF)));            
            for (i=0; i<len2; i++)
            {
              SPDR = d;
              while (!(SPSR & (1<<SPIF)));
              *data++ = SPDR;
            }            
            FM_CS_DISABLE();
          }
        }
            data=&VS_BUFFER32[0];
//            cbr (SPSR, SPI2X); // set low freq. 1/4 SCLK (4MHz)
            VS_DS_clr();
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//0
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//2
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//4
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//6
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//8
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//10
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//12
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//14
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//16
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//18
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//20
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//22
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//24
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//26
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data++;while (!(SPSR & (1<<SPIF)));//28
            SPDR=*data++;while (!(SPSR & (1<<SPIF)));SPDR=*data;while (!(SPSR & (1<<SPIF)));//30-31
            VS_DS_set();
//            sbr (SPSR, SPI2X); // set high freq. 1/2 SCLK (8MHz)
	return 1;
}

extern char const __flash _EthWRlen[];
extern char const __flash _EthTS[]; 
extern char const __flash _EthCNT[];
extern char const __flash _EthpWRH[];
extern char const __flash _EthpWRL[];

extern char const __flash _EthRDlen[];
extern char const __flash _EthpRDH[];
extern char const __flash _EthpRDL[];

#pragma inline = forced
UINT32 _ETHfifo_len(void)
{
	UINT32 len;

	if(ETHfifo_pRD > ETHfifo_pWR)
	{
		len = ETHfifo_pEND-ETHfifo_pRD+1+ETHfifo_pWR-ETHfifo_pSTART;
	}
	else if(ETHfifo_pRD < ETHfifo_pWR)
	{
		len = ETHfifo_pWR-ETHfifo_pRD;
	}
	else
	{
		len = 0;
	}

	return len;
}

// Читаем пакет, длину, таймаут
#pragma inline = forced
UINT16 ETHfifo_PKT_read(UINT8* data, UINT8* TS)
{
	UINT32 pRD; //локальный указатель на запись
        UINT16 len1, len;        
	UREG c;
	UINT8 header_cnt=0;
       	pRD = ETHfifo_pRD;
        
//       len+=3; //  к длине Ethernet пакета добавим 2 байта его длины и 1 байт метки времени
//        #ifdef CONSOLE_DEBUG
//           _print_num (_EthRDlen,len); _print_num (_EthTS,(UINT16)TS);
//           _print_num (_EthpRDH,(UINT16)(pRD>>16)); _print_num (_EthpRDL,(UINT16)pRD);
//        #endif
//        len1=len;
        
  	FM_CS_ENABLE();
        SPDR=FM_READ;
        while (!(SPSR & (1<<SPIF)));
        SPDR=(pRD>>16);
        while (!(SPSR & (1<<SPIF)));
        SPDR=(pRD>>8);
        while (!(SPSR & (1<<SPIF)));
        SPDR=(pRD);
        while (!(SPSR & (1<<SPIF)));
        // Открыли сессию чтения из FIFO                         
        len1=1;
        do
        {
          UREG f=0;
          SPDR = 0xFF;
          if (++pRD>ETHfifo_pEND) f=1; // при чтении след. ячейки упремся в потолок... начнем с начала
          while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished                 
          c=SPDR; // прочитали байт из FRAM          
          switch (header_cnt)
          {
            case 0:
              len=(UINT16)(c<<8); // прочитали старший байт длины пакета
              len1++;
              header_cnt++;
              break;
            case 1:
              len|=(UINT16)c; //len1 - считали младший байт длины пакета
              len1++;
              header_cnt++;
              break;
            case 2:  
              *TS=(UINT8)c; // считали метку времени и запомнили ее
              header_cnt++; // 
              // Ограничим длину пакета на всякий случай, чтоб при отладке не запороть ОЗУ)
              if (len> ETH_MAX_PACKET_SIZE) len = ETH_MAX_PACKET_SIZE;
              if (_ETHfifo_len() < (UINT32) len) return 0; // в буфер записано меньше, чем пытаемся считать
              len1=len+1; // обновим длину цикла с учетом длины пакета+1
              #ifdef CONSOLE_DEBUG
              _print_num (_EthRDlen,len); _print_num (_EthTS,(UINT16)(*TS));
              _print_num (_EthpRDH,(UINT16)((pRD-1)>>16)); _print_num (_EthpRDL,(UINT16)(pRD-1));
              #endif
              break;
            default:
              *data++=c; // считали очередной байт пакета
              break;
          }             	
          if (f) // при чтении след. ячейки упремся в потолок... начнем с начала
          {
       	    FM_CS_DISABLE(); // закроем старую сессию    
            pRD=ETHfifo_pSTART;
  	    FM_CS_ENABLE();            
            SPDR=FM_READ;
            while (!(SPSR & (1<<SPIF)));
            SPDR=(pRD>>16);
            while (!(SPSR & (1<<SPIF)));
            SPDR=(pRD>>8);
            while (!(SPSR & (1<<SPIF)));
            SPDR=(pRD);
            while (!(SPSR & (1<<SPIF)));
        // Открыли сессию чтения из FIFO                                   
          }
        }
        while (len1--);        
        FM_CS_DISABLE();
        ETHfifo_pRD=pRD; // запомнили текущее значение указателя чтения
        ETHfifo_CNT--;// уменьшаем счетчик пакетов в FIFO
        #ifdef CONSOLE_DEBUG
            _print_num (_EthCNT,(UINT16)ETHfifo_CNT);
        #endif       
        return len;    
}



#pragma inline=forced
const char __flash *OnOff_P(UREG flag)
{
  return !flag?"On":"Off";
}

char *stradd_E(char *d, char __eeprom *s)
{
  char c;
  UREG max=15;
  do
  {
    c=*s++;
    if (!(--max)) c=0;
    *d++=c;
  }
  while(c);
  return d-1;
}

__x_z char *stradd_P(char *d, char __flash *s)
{
  char c;
  do
  {
    c=*s++;
    *d++=c;
  }
  while(c);
  return d-1;
}

void MD5cheat(char *out, unsigned char *p, UREG len);

//Секретный ключ MD5(login+':'+realm+':'+pass)+':'
#define AUTH_A1 (MD5_Buffer+0)
//nonce, который мы передаем клиенту + ':'
#define AUTH_srv_nonce (MD5_Buffer+33)
//MD5(METHOD+':'+URL), он же будет буфером исходных данных для изготовления A1, A2
#define AUTH_A2 (MD5_Buffer+66)

#define ASCIIZ_CHUNK (255)
#define FLASH_ASCIIZ_CHUNK (254)
#define EEPROM_ASCIIZ_CHUNK (253)
#define NO_CHUNK (0)


HTTP_SOCK http_sock;

#define MD5_Buffer (http_sock._MD5_Buffer)

extern void *debug_addr;

//================================================================
void AddHTTPsocket(void)
{
  __x UREG HTTP_hook(UREG state, UREG len, UINT8 *data, TCP_SOCK *_s);
  http_sock.sock.lport=htons(80);
  http_sock.sock.type=TCP_TYPE_SERVER;
  http_sock.sock.state=TCP_STATE_LISTENING;
  http_sock.sock.hook=HTTP_hook;
  http_sock.sock.win=htons(0x800);
  AddTCPsocket(&http_sock.sock);
}
//================================================================
__x_z UREG stricmp_P(const char *s, const char __flash *d);

static __x_z UREG stricmp_PSOCK(HTTP_SOCK *hs, const char __flash *d)
{
  return stricmp_P(hs->http_hdr_item,d);
}

__x_z UREG stricmp_P(const char *s, const char __flash *d)
{
  UREG cs;
  UREG cd;
  do
  {
    cs=*s++;
    cd=*d++;
    if (cs>='a'&&cs<='z') cs+='A'-'a';
    if (cd>='a'&&cd<='z') cd+='A'-'a';
    if (!cs) break;
    if (!cd) break;
  }
  while(cs==cd);
  return cs-cd;
}


__z void _i2a(char *s, UINT16 v);

#pragma optimize=no_inline
__z void i2a(char *s, UINT16 v)
{
  _i2a(s,v);
}

static const char __flash * __flash HTTP_LEVELS[]={http_root_level1,http_root_level2,http_root_level3,http_eeprom_refr_button};

enum _LIST_OF_HTTP_REQS
{
  _NOT_A_GOOD_HTTP_REQ=0,
  _GET_root,
  _GET_z,
  _GET_s,
  _POST_w,
  _POST_m,
  _POST_b,
  _POST_e,
  _POST_r,
  _POST_z,
  _POST_f,
  _POST_p,
  _POST_s,
};

/*static const char __flash SELTAGS_1[]=" SELECTED>";

#pragma optimize=no_inline
static const char __flash * __get_SELECTED(void)
{
  return SELTAGS_1;
}

#pragma optimize=no_inline
static const char __flash * __get_ETAG(void)
{
  return SELTAGS_1+9;
}

#pragma optimize=no_inline
static const char __flash * __get_CHECKED(void)
{
  return " CHECKED>";
}*/

#pragma optimize=no_inline
static const char __flash *__get_SVN_Revision(void)
{
  return "2";
}

__z UREG IsGoodHTTPreq(HTTP_SOCK *hs)
{
  UREG c=hs->req[0];
  UREG c5=hs->req[5];
  UREG c6=hs->req[6];
  if (c=='G')
  {
    if (c5=='\0') return _GET_root;
    if (c5=='z') return _GET_z;
    if (c5=='s') return _GET_s;
  }
  else
    if (c=='P')
    {
      if (c6=='w') return _POST_w;
      if (c6=='m') return _POST_m;
      if (c6=='e') return _POST_e;
      if (c6=='r') return _POST_r;
      if (c6=='z') return _POST_z;
      if (c6=='f') return _POST_f;
      if (c6=='p') return _POST_p;
      if (c6=='s') return _POST_s;
    }
  return _NOT_A_GOOD_HTTP_REQ;
}

//#pragma optimize=no_inline
#pragma inline=forced
static const char *__get_AUTH_srv_nonce(void)
{
  return AUTH_srv_nonce;
}

#pragma segment="EEPROM_I"
void REFRESH_EEPROM(void)
{
  if (EEPROM_RESTORED)
  {
    //volatile char __eeprom *p=(volatile char __eeprom *)__segment_begin("EEPROM_I");
    volatile char __eeprom *p=(volatile char __eeprom *)MAC_EEPROM;
    do
    {
      char c;
      EEPROM_RESTORED=0;
      c=*p;
      if (EEPROM_RESTORED)
      {
	*p=c;
	EEPROM_RESTORED=0;
	c=*p;
	if (EEPROM_RESTORED) break; //Не восстанавливается
      }
    }
    while(++p!=(volatile char __eeprom *)__segment_end("EEPROM_I"));
  }
}

void SYNC_EE_(void)
{
}

static __z void SetupRegister(UREG select_n, HTTP_SOCK *s)
{
  void StartGET(void);
  UINT16 value=s->value;
  
  if (select_n>=243)//IsGoodHTTPreq(s)==_POST_z)
  {
    select_n-=243;
    if (select_n<4)
    {
      ((char __eeprom *)(&IP_EEPROM))[select_n]=value;
    }
    else
    {
      select_n-=4;
      if (select_n<ETH_HWA_LEN)
      {
	MAC_EEPROM[select_n]=value;
      }
    }
    return;
  }
  select_n-=1;
  if (select_n<8)
  {
    ((char __eeprom *)(&OUTG_IP))[select_n]=value;
    if (select_n==7) StartGET();
    return;
  }
  select_n-=8;
}

static void StoreDeviceName(const char *s, UREG adr)
{
}

enum HTTP_SRV_STATE
{
  _HTTP_REQ=0,
  _HTTP_LF,
  _HTTP_CR,
  _HTTP_PROP,
  _HTTP_SUBPROP,
  _HTTP_URI,
  _HTTP_RESP,
  _HTTP_CONTENTLEN,
  _HTTP_CONTENTLEN_OK,
  _HTTP_BODY, //Последний \n заголовка, все что дальше - контент
  _HTTP_NAME,
  _HTTP_EQU,
  _HTTP_VALUE,
  _HTTP_BRK,
  _HTTP_AMPERSAND,
  _HTTP_STRING,
  _HTTP_IP,
  _HTTP_STRIP,
  _HTTP_BIN,
  _HTTP_BIN_STAGE2,
  _HTTP_WAIT_USART, //Состояние ожидания готовности USART
  _HTTP_PREP_SEND,
  _HTTP_SEND,
};

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

#pragma optimize=no_code_motion
static __x UREG HTTP_hook_DATA_RX(UREG len, UINT8 *data, HTTP_SOCK *s)
{
  UREG state=s->state;
  UREG val2=s->val2;
  UREG c;
  UREG i;
  UREG c2;
  const char __flash *p;
  if (state==_HTTP_WAIT_USART) return 0; //Ничего не передаем в ожидании
  if (state>=_HTTP_PREP_SEND) return 1;
  if (!len) return 0;
  do // начало мегацикла
  {
    c=*data;
    switch(state) // пройдемся по состояниям
    {
    case _HTTP_REQ:
      if ((val2>=5)&&(c<=' '))
      {
	state=_HTTP_LF;
	continue;
      }
      if (c==' ') c=':';
      s->req[val2++]=c;
      if (val2==(sizeof(s->req)-1))
      {
	state=_HTTP_LF;
	continue;
      }
      break;
    case _HTTP_LF:
      if (c==10)
      {
	val2=0;
	state=_HTTP_CR;
      }
      break;
    case _HTTP_CR:
      if (c==13)
      {
	state=_HTTP_BODY;
	break;
      }
      state=_HTTP_PROP;
      continue;
    case _HTTP_URI:
    case _HTTP_RESP:
      if (c=='"')
      {
	if (!val2) break;
	goto L_endkey;
      }
    case _HTTP_SUBPROP:
      if (c=='=') goto L_endkey;
      if (c==',') goto L_endkey; //Запятой тоже разделяется
    case _HTTP_PROP:
      if ((c2=c)<=' ')
      {
      L_endkey:
	c2=0;
      }
      s->http_hdr_item[val2++]=c2;
      if (c2&&val2<sizeof(s->http_hdr_item)) break; //Следующий символ
      //Конец ключа
      val2=0;
      switch(state)
      {
      case _HTTP_PROP:
	//Обычные ключи в начале строки
	state=_HTTP_LF;
	if (!stricmp_PSOCK(s,"Content-Length:"))
	{
	  state=_HTTP_CONTENTLEN;
	  break;
	}
	if (!stricmp_PSOCK(s,"Authorization:"))
	{
	  state=_HTTP_SUBPROP;
	  break;
	}
	continue;
      case _HTTP_SUBPROP:
	if (c<' ')
	{
	  state=_HTTP_LF;
	  continue;
	}
	if (c!='=') break;
	if (!stricmp_PSOCK(s,"response"))
	{
	  state=_HTTP_RESP;
	}
	if (!stricmp_PSOCK(s,"uri"))
	{
	  state=_HTTP_URI;
	}
	break;
      case _HTTP_URI:
	AUTH_A2[0]=s->http_hdr_item[1];
	state=_HTTP_SUBPROP;
	break;
      case _HTTP_RESP:
	if (IsGoodHTTPreq(s))
	{
	  //Получили response, теперь надо проверить, правильный ли он
	  //Для начала изготавливаем A1
	  //strcpy_P(AUTH_A2,"Rst7:uNikeE:Upor2007");
	  stradd_E(stradd_P(stradd_E(AUTH_A2,HTTP_LOGIN),":uNikeE:"),HTTP_PASS);
	  MD5cheat(AUTH_A1,(UINT8*)AUTH_A2,strlen(AUTH_A2));
	  //Теперь A2
	  strcpy(AUTH_A2,s->req);
	  MD5cheat(AUTH_A2,(UINT8*)AUTH_A2,strlen(AUTH_A2));
	  //И теперь общий результат
	  AUTH_A1[32]=':';
	  AUTH_srv_nonce[32]=':';
	  MD5cheat(AUTH_A1,(UINT8*)MD5_Buffer,98);
	  if (!strncmp(AUTH_A1,s->http_hdr_item,32))
	  {
	    if (s->af.AUTH_MODE<2) s->af.AUTH_MODE++; //1 - есть авторизация, 2 - устарел nonce
	  }
	  else
	  {
	    s->af.AUTH_MODE=0; //Нет авторизации
	  }
	}
	s->af.AUTH_PRESENT=1;
	state=_HTTP_LF;
	continue;
      }
      break;
    case _HTTP_BODY:
      //Последний \n заголовка, готовимся к получению тела
      s->content_length=s->value+1;
      s->value=0;
      state=_HTTP_STRIP; //По умолчанию - пропускаем контент
#ifdef NDEBUG
      if (!s->af.AUTH_PRESENT) s->af.AUTH_MODE=0;
#else
      s->af.AUTH_PRESENT=1;
      s->af.AUTH_MODE=1;
#endif
      c=IsGoodHTTPreq(s);
      if ((s->af.AUTH_MODE==1)||(c==_POST_b))
      {
	switch(c)
	{
	case _POST_m:
	  //Чистим битмап для работы с чекбоксами, сейчас не используем
	  //ClearBitmap(RAM_BITMAP);
	case _POST_w:
	case _POST_z:
	  state=_HTTP_NAME; //Будем разбирать текст в запросе POST
	  break;
        case _POST_f: // Следующая станция
//          s->statnum++;
          if(s->statnum++ == 15) s->statnum=0;
/*          
          stationNum++;
          if (stationNum>15) stationNum=0;
          stationNum_EEPROM=stationNum;
*/          
          state=_HTTP_NAME; //Будем разбирать текст в запросе POST
	  break;  
        case _POST_p: // Предыдущая станция          
          if(s->statnum--==0) s->statnum=15;          
          state=_HTTP_NAME; //Будем разбирать текст в запросе POST
	  break;                    
        case _POST_s:
          __no_operation();
          state=_HTTP_NAME; //Будем разбирать текст в запросе POST
//        тут надо разбирать параметры POST
	  break;    
	case _POST_e:
	  REFRESH_EEPROM();
	  break;
	}
      }
      break;
    case _HTTP_BIN:
    case _HTTP_CONTENTLEN:
    case _HTTP_NAME:
    case _HTTP_VALUE:
      //Принимаем десятичный символ и накапливаем в s->value
      c-='0';
      if (c>9)
      {
	state++;//_HTTP_BRK;//state++;
	continue; //не цифра, следующее состояние с тем-же символом
      }
      s->value=s->value*10+c; //Накопление
      break;
    case _HTTP_CONTENTLEN_OK:
      //Приняли значение Content-Length
      state=_HTTP_LF;
      continue;
    case _HTTP_EQU:
      //Приняли значение NAME, тут должен быть '=', но мы пока положим на проверку
      val2=s->value;
      state=_HTTP_VALUE;
      
      if ((val2>=201)&&(val2<=208))// Station Attributes to change
      {
	val2=0;
	state=_HTTP_STRING;
	break;
      }
      if ((val2==241)/*||IsGoodHTTPreq(s)==_POST_w*/)
      {
	val2=0;
	state=_HTTP_STRING;
	break;
      }
      if (val2==242)
      {
	val2=16;
	state=_HTTP_STRING;
	break;
      }
      s->value=0;
      break;
    case _HTTP_BRK:
      if (c=='O'||c=='o')
      {
	s->value=1; //Заменяем "On/Off" на 1
      }
      /*if (val2==2)
      {
	EE_ENABLE_MODBUS=0;
	EE_ENABLE_AJAX=0;
      }*/
      //Устанавливаем данные по маске
      SetupRegister(val2,s);
      s->value=0;
      state=_HTTP_AMPERSAND;
      continue; //Стрипаем до &
    case _HTTP_AMPERSAND:
      if (c=='&') state=_HTTP_NAME; //Когда дошли до &, опять начинаем разбор NAME
      break;
    case _HTTP_STRING:
      //Принимаем строку
      if (c=='&')
      {       
        if (val2) // если параметр не пустой
        {
          // если пришло поле №201, то сохраним номер станции, которую надо поменять              
          if (s->value==201) s->statnum= a2i(AUTH_A2); 
         // если пришло поле №202, то поменяем название станции в EEPROM      
          else if (s->value==202)
          {
            char __eeprom *ep=&station_list[s->statnum].LCD_ID[0];
            char *rp=AUTH_A2;
            while (*rp)
            {
              UREG sym=*rp++;
              if (sym =='+') sym=' ';
              *ep++=sym;
            }
            *ep=0;
          }
          // поля №203-206 - поменяем IP станции в EEPROM      
          else if ((s->value>=203)&&(s->value<=206))
          {
           ((char __eeprom *)(&station_list[s->statnum].IP))[s->value-203]=(UINT8)a2i(AUTH_A2);
          }
          // если пришло поле №207, то поменяем порт станции в EEPROM      
          else if (s->value==207)
          {
            station_list[s->statnum].port=(UINT16)a2i(AUTH_A2);         
          }
          else if (s->value==208)
          {
            __no_operation();
          }
        }
	s->value=0;
        state=_HTTP_NAME; //Следующий элемент
	break;        
      }
      if ((val2&15)==15)
      {
	state=_HTTP_STRIP; //Великоват login или pass, стрипаем все остальное
	break;
      }
      {
	char *wp=AUTH_A2+val2;
	*wp++=c;
	*wp=0;
	val2++;
      }
      break;
    case _HTTP_STRIP:
      break;
    case _HTTP_BIN_STAGE2:
      s->value=0;
      state=_HTTP_BIN;
      break;
    }
    if (state>_HTTP_BODY)
    {
      UINT16 l=s->content_length;
      if (!--l) goto L_reqfinish;
      s->content_length=l;
    }
    data++;
    len--;
  }
  while(len);
  s->state=state;
  s->val2=val2;
  return 0;
  //Закончен разбор запроса
L_reqfinish:
  s->state=_HTTP_SEND;
  s->val2=val2;
  if (state==_HTTP_VALUE) //Последний параметр в POST
  {
    SetupRegister(val2,s);
  }
  if (state==_HTTP_STRING)
  {
    if (IsGoodHTTPreq(s)==_POST_s)
    {// Принимали запрос GET, надо записать в eeprom        
        // если пришло поле №208, то поменяем запрос GET в EEPROM      
//        if (s->value==208)
          char __eeprom *ep=&station_list[s->statnum].req[0];
           if (val2)
            {
            char *rp=AUTH_A2;
            while (*rp)
            {
              *ep++=*rp++;
            }
            *ep=0;
            }
          else
          {
            *ep=0;
          }
    }
    //Принимали login и pass, надо записать в eeprom        
    else if (IsGoodHTTPreq(s)==_POST_w)
    {
      //Принимали новое имя устройства
      StoreDeviceName(AUTH_A2,s->value);
    }
    else 
    {
      //Принимали страничку Z
      UINT8 __eeprom *dp=(UINT8 __eeprom *)&HTTP_LOGIN;
      UREG i=0;
      do
      {
	*dp++=AUTH_A2[i++];
	if (i==16)
	{
	  dp=(UINT8 __eeprom *)&HTTP_PASS;
	}
      }
      while(i<32);
    }
  }
  p=http_404;
  if ((i=IsGoodHTTPreq(s))!=0)
  {
    switch(s->af.AUTH_MODE)
    {
    case 0:
      //Нет авторизации
      p=http_401;
      //Готовим новый nonce
      MD5cheat(AUTH_srv_nonce,(UINT8*)MD5_Buffer,98); //Из говна пулю ;)
      break;
    case 1:
      //Есть неустаревшая авторизация
      switch(i)
      {
      case _GET_root:
	//Готовим динамические данные
	p=http_root;
	s->httpcb.select_n=0;
	break;
      case _POST_z:
	p=http_302z;
	break;
      case _POST_w:
      case _POST_m:
	SYNC_EE_();
	//PORTC=RAM_BITMAP[0];
	//DDRC=RAM_BITMAP[1];
      case _POST_r:
      case _POST_e:
	p=http_302;
	break;
      case _POST_f:        
      case _POST_p:        
      case _POST_s:        
	p=http_302s;
	break;    
      case _GET_z:
	//Готовим динамические данные
	{
	  UINT8 __eeprom *dp=(UINT8 __eeprom *)&IP_EEPROM;
	  UREG i=0;
	  do
	  {
	    i2a(AUTH_A2+i*5,*dp++);
	    i++;
	    if (i==4)
	    {
	      dp=(UINT8 __eeprom *)MAC_EEPROM;
	    }
	  }
	  while(i<10);
	}
	p=http_z;
	s->httpcb.select_n=240;
	break;
      case _GET_s:  
        /*
        //Готовим динамические данные
        {
          UINT8 __eeprom *sl=(UINT8 __eeprom *)&station_list;
          UREG i=0;
          do
          {
             i2a(AUTH_A2+i*5,*sl++);
             i++;
          }
          while(i<10);          
        }
        */
        p=http_s;
       	s->httpcb.select_n=201;
        break;
      }
      s->af.AUTH_MODE=2; //Текущая авторизация устарела
      break;      
    case 2:
      p=http_401stale;
      s->af.AUTH_MODE=0; //Нафиг текущую авторизацию
      //Готовим новый nonce
      MD5cheat(AUTH_srv_nonce,(UINT8*)MD5_Buffer,98); //Из говна пулю ;)
      break;
    }
  }
  AUTH_srv_nonce[32]=0;
  //AUTH_A2[32]=0;
  s->httpcb.html=p;
  netw_memcpy(&s->httpcb_ack,&s->httpcb,sizeof(HTTP_CB));
  return 1; //Есть данные для передачи
}

static __x UREG HTTP_hook_DATA_TX(UREG len, UINT8 *data, HTTP_CB *cb, HTTP_SOCK *s)
{
//  HTTP_CB *cb;
  //Посылка новых данных
  UREG c=s->state;
  UREG i=0;

  if (c==_HTTP_PREP_SEND)
  {
    s->state=c=_HTTP_SEND;
    s->sock.async_req=3;
  }
  if (c<_HTTP_SEND) return 0; //Ничего не передаем, не тот режим
  if (!cb->html) return 0; //Пока нет данных для передачи
//    _print_num(_sData, (UINT16) data);
 // _print_num(_sLen, (UINT16) len);
L_REPRINT_SN:
  if ((c=IsGoodHTTPreq(s))!=_GET_z)
  {
    UREG i=cb->select_n;
    i-=1;
    if (i<8)
    {
      i2a(AUTH_A2+3*5,((char __eeprom *)(&OUTG_IP))[i]);
    }
  }
  i2a(s->select_name,cb->select_n);
L_CHUNK:
  //Генерируем кусочки
  switch(cb->chunk_mode)
  {
  case ASCIIZ_CHUNK:
    {
      const char *chunk=cb->chunk_ram;
      while((c=*chunk)!=0)
      {
	if (i>=len)
	{
	  cb->chunk_ram=chunk;
	  goto L_EFRAME;
	}
	chunk++;
	if (data) *data++=c;
	i++;
      }
      if (!c) cb->chunk_mode=0;
      cb->chunk_ram=chunk;
      break;
    }
  case FLASH_ASCIIZ_CHUNK:
    {
      const char __flash *chunk=cb->chunk;
      while((c=*chunk)!=0)
      {
	if (i>=len)
	{
	  cb->chunk=chunk;
	  goto L_EFRAME;
	}
	chunk++;
	if (data) *data++=c;
	i++;
      }
      if (!c) cb->chunk_mode=0;
      cb->chunk=chunk;
      break;
    }
  case EEPROM_ASCIIZ_CHUNK:  
    {
      const char __eeprom *chunk=cb->chunk_eeprom;      
      while((c=*chunk)!=0)
      {
	if (i>=len)
	{
	  cb->chunk_eeprom=chunk;
	  goto L_EFRAME;
	}
	chunk++;
	if (data) *data++=c;
	i++;
      }
      if (!c) cb->chunk_mode=0;
      cb->chunk_eeprom=chunk;
      break;
    }
  default:
    {
      const char *chunk=cb->chunk_ram;
      while(cb->chunk_mode)
      {
	if (i>=len)
	{
	  cb->chunk_ram=chunk;
	  goto L_EFRAME;
	}
	c=*((char*)chunk++);
	if (data) *data++=c;
	i++;
      }
      cb->chunk_ram=chunk;
      break;
    }
  case NO_CHUNK:
    break;
  }
  //Разбираем пакованные данные
  for(;;)
  //while(i<len)
  {
    c=*cb->html;
    if (c>0&&c<128)
    {
      if (i>=len) goto L_EFRAME;
      cb->html++;
      if (data) *data++=c;
      i++;
      continue;
    }
    cb->html++;
/*    if (c>=_sel0_&&c<=_sel9_)
    {
      //sel0-9
      //chunk=" SELECTED>";
      cb->chunk=__get_SELECTED();
      cb->chunk_mode=FLASH_ASCIIZ_CHUNK;
      {
	UREG m=255;
	switch(cb->select_n)
	{
	case 1:
	  m=RAM_SPEED;
	  break;
	case 2:
	  m=RAM_PARITY;
	  break;
	}
	if (m!=(UREG)(c-_sel0_)) cb->chunk=__get_ETAG();
      }
      goto L_CHUNK;;	    
    }*/
    if (c>=_radid0_&&c<=_radid8_)
    {
      
      //radid0-9
     switch (c)
      {        
        case _radid0_: // выводим список радиостанций из EEPROM
//          cb->chunk_eeprom=(char const __eeprom *)&station_list[0];      
//          cb->chunk_mode=EEPROM_LIST_CHUNK;
//          goto L_CHUNK;        
          break;        
      case _radid1_: // номер текущей станции
        {          
          i2a(AUTH_A2, s->statnum);
          cb->chunk_ram=AUTH_A2;
          cb->chunk_mode=ASCIIZ_CHUNK;
          goto L_CHUNK;
        }
      case _radid2_: // station ID
        {          
          cb->chunk_eeprom=&station_list[s->statnum].LCD_ID[0];
          cb->chunk_mode=EEPROM_ASCIIZ_CHUNK;
          goto L_CHUNK;
        }  
      case _radid3_: // IP1
        {          
          i2a(AUTH_A2, (UINT8) (station_list[s->statnum].IP));
          cb->chunk_ram=AUTH_A2;
          cb->chunk_mode=ASCIIZ_CHUNK;
          goto L_CHUNK;
        }    
      case _radid4_: // IP2
        {          
          i2a(AUTH_A2, (UINT8) (station_list[s->statnum].IP>>8));
          cb->chunk_ram=AUTH_A2;
          cb->chunk_mode=ASCIIZ_CHUNK;
          goto L_CHUNK;
        }
      case _radid5_: // IP3
        {          
          i2a(AUTH_A2, (UINT8) (station_list[s->statnum].IP >>16));
          cb->chunk_ram=AUTH_A2;
          cb->chunk_mode=ASCIIZ_CHUNK;
          goto L_CHUNK;
        }    
      case _radid6_: // IP4
        {          
          i2a(AUTH_A2, (UINT8) (station_list[s->statnum].IP >> 24));
          cb->chunk_ram=AUTH_A2;
          cb->chunk_mode=ASCIIZ_CHUNK;
          goto L_CHUNK;
        }      
      case _radid7_: // station port
        {          
          i2a(AUTH_A2, (UINT16) (station_list[s->statnum].port));
          cb->chunk_ram=AUTH_A2;
          cb->chunk_mode=ASCIIZ_CHUNK;
          goto L_CHUNK;
        }              
      case _radid8_: // get request
        {
          cb->chunk_eeprom=&station_list[s->statnum].req[0];
          cb->chunk_mode=EEPROM_ASCIIZ_CHUNK;
          goto L_CHUNK;
        }
      }
      cb->chunk_ram=AUTH_A2+(c-_radid0_)*5;      
      cb->chunk_mode=ASCIIZ_CHUNK;
      goto L_CHUNK;
    }
    
    if (c>=_vlanid0_&&c<=_vlanid9_)
    {
      //vlanid0-9
      cb->chunk_ram=AUTH_A2+(c-_vlanid0_)*5;
      cb->chunk_mode=ASCIIZ_CHUNK;
      goto L_CHUNK;
    }
    switch(c)
    {
    case 0:
      if (cb->stk)
      {
	cb->html=cb->stk;
	cb->stk=NULL;
	continue;
      }
      else
      {
	cb->html--; //Не убегаем с 0
	if (!data)
	{
	  //Все данные нам подтвердили, выходим
	  s->state=(UREG)-1;
	  CloseTCPsocket(&s->sock);
	  goto L_EFRAME;
	}
      }
      break;
    default:
      cb->chunk=CHUNKS[c-128];
      cb->chunk_mode=FLASH_ASCIIZ_CHUNK;
      goto L_CHUNK;
/*    case _binary_out_:
      cb->chunk_ram=(char*)&http_sock._RS485_BUF;;
      cb->chunk_mode=253;
      goto L_CHUNK;*/
    case _svn_revision_:
      cb->chunk=__get_SVN_Revision();
      cb->chunk_mode=FLASH_ASCIIZ_CHUNK;
      goto L_CHUNK;
    case _nonce_:
      cb->chunk_ram=__get_AUTH_srv_nonce();
      cb->chunk_mode=ASCIIZ_CHUNK;
      goto L_CHUNK;
    case _select_name_:
      cb->select_n++;
      cb->chunk_ram=s->select_name;
      cb->chunk_mode=ASCIIZ_CHUNK;
      goto L_REPRINT_SN;
    case _check_eeprom_restored_:
      /*if (!EEPROM_RESTORED) */ continue;
      /* c=_root_level1_+3; */
    case _root_level1_:
    case _root_level2_:
    case _root_level3_:
      cb->stk=cb->html;
      cb->html=HTTP_LEVELS[c-_root_level1_];
      continue;
    case _voltage_:
      cb->chunk_ram=AUTH_A2+0;
      cb->select_n++;
      cb->chunk_mode=ASCIIZ_CHUNK;
      goto L_REPRINT_SN;
    case _logval_:
      cb->chunk_ram=AUTH_A2+5;
      cb->select_n++;
      cb->chunk_mode=ASCIIZ_CHUNK;
      goto L_REPRINT_SN;
    case _dec_select_name_:
      cb->select_n--;
      goto L_REPRINT_SN;
/*    case _is_checked_:
      cb->chunk=__get_CHECKED();
      cb->chunk_mode=FLASH_ASCIIZ_CHUNK;
      switch(cb->select_n)
      {
      case 4:
	if (EE_ENABLE_MODBUS) goto L_CHUNK;
	break;
      case 5:
	if (EE_ENABLE_AJAX) goto L_CHUNK;
	break;
      }
      cb->chunk=__get_ETAG();
      goto L_CHUNK;*/
/*    case _on_off_:
      {
	UINT16 m;
	UREG i, mask;
	i=cb->select_n-1;
	m=_fastmask(i);
	mask=m>>8;
	cb->chunk=OnOff_P(RAM_BITMAP[__multiply_unsigned(m,32)>>8]&mask);
      }
      cb->chunk_mode=FLASH_ASCIIZ_CHUNK;
      goto L_CHUNK;*/
    }
    break;
  }
L_EFRAME:
//  _print_num(_si, (UINT16)i); 
//  _print_rn();
  return i;
}

__x UREG HTTP_hook(UREG state, UREG len, UINT8 *data, TCP_SOCK *_s)
{
  HTTP_SOCK *s=(HTTP_SOCK *)_s;
  HTTP_CB *cb;
  UREG i;
  UREG j;
  switch(state)
  {
  case TCP_EVENT_CONREQ:
    j=http_q_c_pos; //А сколько народу у нас в ожидании?
    goto L1;
  case TCP_EVENT_CLOSE:
    if ((s->state==(UREG)-1)&&(IsGoodHTTPreq(s)==_POST_r)&&(s->af.AUTH_MODE))
    {
      __disable_interrupt();
      for(;;); //Reboot device
    }
  case TCP_EVENT_ABORT:
    j=http_q_c_pos;
  L1:
    {
      char *p=(char*)&s->state;
      i=sizeof(_HTTP_CONTROL)+sizeof(s->state);
      do
      {
	*p++=0;
      }
      while(--i);
      //do *p++=0; while(--i);
      s->af.AUTH_PRESENT=0;
    }
    return j; //Количество ожидающих клиентов
    //s->content_length=(UINT16)-1;
    //break;
  case TCP_EVENT_ASYNC_REQ:
    i=s->sock.async_req;
    s->sock.async_req=0;
    if (s->state==_HTTP_SEND)
    {
      if (i==3)
      {
	s->sock.txreq=TCP_TXREQ_SEND; //Специально, для досыла после первого пакета
      }
    }
    break;
  case TCP_EVENT_DATA:
    return HTTP_hook_DATA_RX(len,data,s);
  case TCP_EVENT_ACK:
    //Подтверждение данных
    cb=&s->httpcb_ack;
    goto L_SEND;
  case TCP_EVENT_REGENERATE:
    //Откат
    //netw_memcpy(&s->httpcb,&s->httpcb_ack,sizeof(HTTP_CB));
    {
      char *dst=(char*)(&s->httpcb);
      //char *src=(char*)(&s->httpcb_ack);
      i=sizeof(HTTP_CB);
      do
      {
	*dst=dst[sizeof(HTTP_CB)];//*src++;
	dst++;
      }
      while(--i);
    }
  case TCP_EVENT_SEND:
    cb=&s->httpcb;
  L_SEND:
    return HTTP_hook_DATA_TX(len,data,cb,s);
  case TCP_EVENT_QUE_ALLOC:
    //Занимаем буфер TCP_QUE_CLIENT, если это возможно
    i=http_q_c_pos;
    if (i==(sizeof(http_q_c)/sizeof(TCP_QUE_CLIENT))) return 0; //Нет места
    *((TCP_QUE_CLIENT **)data)=http_q_c+i;
    i++;
    http_q_c_pos=i;
    return i; //Сколько всего
  case TCP_EVENT_QUE_GET:
    //Получаем первый TCP_QUE_CLIENT в очереди
    *((TCP_QUE_CLIENT **)data)=http_q_c;
    return http_q_c_pos; //Сколько в очереди
  case TCP_EVENT_QUE_REMOVE:
    //Удаляем клиента из очереди
    {
      j=http_q_c_pos;
      if (j)
      {
	char *dst=(char*)(http_q_c+0);
	//char *src=(char*)(http_q_c+1);
	i=sizeof(http_q_c)-sizeof(TCP_QUE_CLIENT);
	do
	{
	  //*dst++=*src++;
	  *dst=dst[sizeof(TCP_QUE_CLIENT)];
	  dst++;
	}
	while(--i);
	j--;
	http_q_c_pos=j;
      }
      return j; //Сколько осталось
    }
  }
  return 0;
}

void InitVars(void)
{
  UREG i;
/*  i=sizeof(EEPROM_BITMAP)-1;
  //Init EE
  do
  {
    RAM_BITMAP[i]=EEPROM_BITMAP[i];
  }
  while((--i)!=(UREG)-1);*/
  //Init MAC
  i=sizeof(MAC_EEPROM);
  do
  {
    MAC0[i]=MAC_EEPROM[i];
//    MAC_GATE0[i]=MAC_GATE[i];
  }
  while((--i)!=(UREG)-1);
  //Init IP
  IP=IP_EEPROM;
  MASK_IP=MASK_IP_EEPROM;
  stationNum=stationNum_EEPROM;
}

__monitor void WDT_Prescaler_Change(UREG psr)
{
  __watchdog_reset();
  /* Start timed equence */
  //WDTCSR |= (1<<WDCE) | (1<<WDE);
  /* Set new prescaler(time-out) value = 64K cycles (~0.5 s) */
  //WDTCSR = (1<<WDE) | psr;
}

//void ExecuteETH(void);

extern void AddGETsocket(void);
extern void AddPOSTsocket(void);
extern void AddHTTPsocket(void);
extern void StartGET(void);
extern void StopGET(void);
extern void StartPOST(void);

//__no_init UCHAR VS_BUFFER32 [32];

// Быстрое считывание cчетчика пакетов
#pragma inline =forced
UREG _enc28j60Read_EPKTCNT (void)
{ 	
        UREG data;
        // set the bank (if needed)
	if((EPKTCNT & BANK_MASK) != Enc28j60Bank)
	{       // set the bank       
                cbi (ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_CS);
                // issue write command
        	SPDR = ENC28J60_BIT_FIELD_CLR | (ECON1 & ADDR_MASK);
	        while(!(SPSR & (1<<SPIF)));
          	// write data
          	SPDR = (ECON1_BSEL1|ECON1_BSEL0);
        	while(!(SPSR & (1<<SPIF)));
                sbi (ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_CS);
                
                cbi (ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_CS);
                SPDR = ENC28J60_BIT_FIELD_SET | (ECON1 & ADDR_MASK);
	        while(!(SPSR & (1<<SPIF)));
          	// write data
          	SPDR = ((EPKTCNT & BANK_MASK)>>5);
        	while(!(SPSR & (1<<SPIF)));
                sbi (ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_CS);                                
		Enc28j60Bank = (EPKTCNT & BANK_MASK);
	}
        cbi (ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_CS);
        // issue read command
	SPDR = ENC28J60_READ_CTRL_REG | (EPKTCNT & ADDR_MASK);
	while(!(SPSR & (1<<SPIF)));
	// read data
	SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF)));
	// do dummy read if needed
	data = SPDR;
        sbi (ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_CS);
        return data;
	// release CS
}

// состояние плеера
volatile UREG PLAYER_STATE=PLAYER_STATE_STOPED;

volatile UINT8 BufferPKT=0;
volatile UINT8 FIFO_pkt_TS;

#pragma inline =forced
void Try2PlayMP3 (void)
{
    //Выгребаем мп3 данные только состоянии "проигрывание"
    if (PLAYER_STATE==PLAYER_STATE_PLAYING)
    {    
        if ((VS_DREQ_PIN&(1<<VS_DREQ)))//VS_DREQ==1
        {         
          if (!_MP3fifo_read32()) // если данные в мп3 буфере исчерпаны
          {
             PLAYER_STATE=PLAYER_STATE_BUFFERING; 
             LCD_fprintline(1,"Buffering");
          }
        }
      }
      // зашли в состоянии "установлен ZeroWindow"
      if (GET_WINDOW_STATE==GET_WINDOW_STATE_ZERO)
        {              
          switch (PLAYER_STATE)
          {            
          case PLAYER_STATE_BUFFERING:
            // если плеер находился в состоянии "буферизация" то переводим в режим "воспроизведение"          
            PLAYER_STATE=PLAYER_STATE_PLAYING;
            LCD_fprintline(1,"Playing");
            get_sock.sock.async_req=1; //Запустим асинхронный старт
//            sbi (PORTD, PD3);               
            break;
          case PLAYER_STATE_PLAYING:
            // в режиме "воспроизведение" обнулилось окно - надо запустить асинхронный старт            
            get_sock.sock.async_req=1; //Запустим асинхронный старт
//            sbi (PORTD, PD3);               
            break;
          default:
            break;
          }                    
        }
}

char const __flash _Hello_string[]=\
"\r\n\
*** uEthRadio v1.00,(c)2010\r\n\
*** by Alexander Yerezeyev\r\n\
*** uNike TCP/IP stack by RST7/CBSIE\r\n\
\r\n";

#pragma inline=forced
UINT16 _READ_U16_REV(volatile UINT8 *p)
{
#pragma diag_suppress=Pa082
  return p[1]|(p[0]<<8);
#pragma diag_default=Pa082
}


__task void main(void)
{
  UINT16 ii;
  UINT8 *p;
  sbi(DDRD, PD3);  
  sbi(DDRD, PD5); 
  sbi (PORTD, PD3);  
  _delay_ms(1000);
  cbi (PORTD, PD3);
  VS_INIT(); 
  VS_VOL();  
  p=(UINT8*)&http_sock;
  ii=sizeof(http_sock);
  do
  {
    *p++=0;
  }
  while(--ii);
  uart_init();
 // for (i=0;i<64;i++) {while ( !( UCSRA & (1<<UDRE)) );UDR = 0xAA;}
   _print_fstr(_Hello_string);
  #ifdef CONSOLE_DEBUG
  _print_fstr(_Hello_string);
  #endif
  TWAR=0x00;
  InitVars();
  __disable_interrupt();
  nic_init();
  InitEthernetHW();
  fifo_init();
  //fifo_test();
  __enable_interrupt();
   AddHTTPsocket(); 
   AddGETsocket();
  // AddPOSTsocket();
  //_delay_ms(1000);
   //_delay_ms(1000);
   LCD_init_4();     
   stationNum=stationNum_EEPROM;   
   LCD_fprintlineEE(0,&station_list[stationNum].LCD_ID[0]);
   LCD_TCPstate(get_sock.sock.state); 
   LCD_PLAYERstate(PLAYER_STATE);
  //Инициализируем ADC
  //ADMUX=(0<<REFS1)|(0<<REFS0)|(1<<ADLAR)|6; //Выбираем линию 1
  //ADCSRA=(1<<ADEN)|(1<<ADSC)|(1<<ADIF)|7;//Прескаллер 1:128
  // Инициализация таймера 1
  OCR1A=0x0001; 
  TCCR1A|=(1<<COM1A1)|(1<<COM1A0)|(1<<WGM11);// 9 bit PWM
  TCCR2|=(1<<CS22)|(1<<CS21)|(1<<CS20); //1024
  TCCR1B|=(1<<WGM12)|(1<<CS10);
  TIMSK|=(1<<TOIE2);
  //UINT8 i;
  //for (i=0;i<=9;i++)    {LCD_putc ('*');}
//  station_list[stationNum].LCD_ID[0]='A';    
  for(;;)
  {  
    BufferPKT=0; // Сбросим флаг того, что пакет пришел из буфера
    if (ETH_TASK_WAKEUP) // если таймер натикал
    {
        ETH_TASK_WAKEUP=0;         
        goto L_WAKE_UP; // принудительно вызовем стек
    }
    // иначе вызываем только после того, как что то пришло в буфер
    if((_enc28j60Read_EPKTCNT()!=0)&&(nic_poll() != 0))
    {
L_WAKE_UP:
      INT_ETH_PROCESS_PKT2();   //Позвать стек (в RAM-буфере лежит пакет)      
      UREG fc=ETHfifo_CNT;
      while(fc--)   // пока есть пакеты в буфере      
      {        
        cbi (TIMSK, TOIE2);
        UINT8 TS;
        INT8 CTS;
        UINT16 PKT_len;
        UINT8 irshigh8;
        PKT_len=ETHfifo_PKT_read((unsigned char *)&ETH_PKT.hdr.dst_mac, &TS);//Считаем пакет, и таймаут    
        FIFO_pkt_TS=TS; // сохраним таймаут в переменной для последующего использования  
        irshigh8=(UINT8) (irs_high&0x00FF);
        CTS = (INT8) TS;
        CTS-=irshigh8;
        if (CTS<0) 
        {
          #ifdef CONSOLE_DEBUG
          _print_num("Old PKT:",(UINT16)(irshigh8));
          _print_rn();
          #endif
          continue;  // пакет устарел - пропускаем его                     
        }         
        #ifdef CONSOLE_DEBUG        
        IP_FRAME *ip;
        ip=(IP_FRAME*)(&ETH_PKT); 
        _print_num("FFlen:",PKT_len);
        _print_num("IDFF:",_READ_U16_REV (((UINT8*)(&ip->ip.id))));
        #endif
        BufferPKT=1;  // поднимем флаг - "есть пакет из буфера"
        ETH_PKT_mode=1;
        ETH_PKT_len=PKT_len;
        INT_ETH_PROCESS_PKT2(); // позовем стек
        UREG i;
        for (i=0;i<=4;i++) Try2PlayMP3();
      }   
    }
  sbi (TIMSK, TOIE2);
  UREG i;
  for (i=0;i<=3;i++) Try2PlayMP3();
//  Try2PlayMP3();
  OCR1A = (UINT16)(_MP3fifo_len()>>8);
  if (Keyboard_task)
  {
    switch (Keyboard_task)
    {
      case Keyboard_task_startget:
        LCD_fprintline(1,"Connecting");
        StartGET();
        break;
      case Keyboard_task_stopget:
         PLAYER_STATE= PLAYER_STATE_STOPREQ;
         LCD_fprintline(1,"Stopping");
        break;
      case Keyboard_task_stationchange:
        stationNum++;
        if (stationNum>15) stationNum=0;
        stationNum_EEPROM=stationNum;
        LCD_fprintlineEE(0,&station_list[stationNum].LCD_ID[0]);
        break;          
    }
    Keyboard_task=0;
  }
}
};
