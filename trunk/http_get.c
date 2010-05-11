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
    if ((IP&MASK_IP)==(IPsrc&MASK_IP)) get_sock.sock.ACKNO=IPsrc;// без шлюза     
     else get_sock.sock.ACKNO=OUTG_IP[1]; // идем через шлюз
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
  _GET_STOP=0, //Остановленно
  _GET_CONREQ, //Запрос соединения
  _GET_SENDREQ, //Посылка HTTP-запроса
  _GET_WAITACK, //Ожидание подтверждения посылки
  _GET_HEADER, //Прием заголовка
  _GET_BODY, //Прием тела
  _GET_METADATA //Прием метаданных
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
  pWR = MP3fifo_pWR; // Голова    
    
  if (state==_GET_BODY) // Заходим сюда - если идет прием тела
  {    
    if (PLAYER_STATE!=PLAYER_STATE_PLAYING) 
    {      
      if (PLAYER_STATE!=PLAYER_STATE_BUFFERING) LCD_fprintline(1,"Buffering");     
      PLAYER_STATE=PLAYER_STATE_BUFFERING;
    }
L_BODY:
  {
    UREG addr;
    FM_CS_ENABLE();   // Выбираем FRAM
    SPDR=FM_WREN;     // Разрешаем запись
    while (!(SPSR & (1<<SPIF)));               
    FM_CS_DISABLE();
    FM_CS_ENABLE();   // Выбираем FRAM
    SPDR=FM_WRITE;    // Операция - "запись"
    addr=(UREG)(pWR>>16);
    while (!(SPSR & (1<<SPIF)));
    SPDR=addr;  // Начальный адрес - голова [24..16]
    addr=(UREG)(pWR>>8);
    while (!(SPSR & (1<<SPIF)));       
    SPDR=addr;  // Начальный адрес - голова [15..8]
    addr=(UREG)(pWR);
    while (!(SPSR & (1<<SPIF)));
    SPDR=addr;    //Начальный адрес - голова [7..0]    
    while (!(SPSR & (1<<SPIF)));    
  }
L_BODY2:    
    //Быстрая обработка данных тела
    if (!len) goto GET_hook_DATA_RX_exit;
    bl=s->block_length; // "вспомним" размер блока мета-данных        
    do
    {            
      UREG c=*data++; //читаем очередной байт тела
      if (bl) //Только если мы знаем, что метаданные присутствуют
      {	
	if (!(--bl))
	{
	  //Блок закончился, в c - размер метаданных
          if (!c)                          // Длина метаданных 0 (их нет)
          {
//             META_len=0;
//             pMETA_buf=&META_buf[0];
             bl=s->metadata_interval;
             s->block_length=bl;
             continue;
          }
          s->state=state=_GET_METADATA;
          s->block_length=c*16;           // рассчитаем и запомним длину блока с мп3 данными
	  len--;
          LCD_putMETA(0,0); 
	  goto L_METADATA;                // идем на обработку метаданных
	}
      }      
        UREG f=0;
        SPDR = c; // Записали байт в FIFO      
        pWR += 1; // Сдвигаем голову        
        if (pWR > MP3fifo_pEND) f=1;
	while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished 
	if (f) 
        {
          UREG addr;
          pWR=MP3fifo_pSTART; // дошли до крайней ячейки - надо перепрыгивать вниз                
          FM_CS_DISABLE();
          FM_CS_ENABLE();   // Выбираем FRAM
          SPDR=FM_WREN;     // Разрешаем запись
          while (!(SPSR & (1<<SPIF)));               
          FM_CS_DISABLE();
          FM_CS_ENABLE();   // Выбираем FRAM
          SPDR=FM_WRITE;    // Операция - "запись"
          addr=(UREG)(pWR>>16);
          while (!(SPSR & (1<<SPIF)));
          SPDR=addr;  // Начальный адрес - голова [24..16]
          addr=(UREG)(pWR>>8);
          while (!(SPSR & (1<<SPIF)));       
          SPDR=addr;  // Начальный адрес - голова [15..8]
          addr=(UREG)(pWR);          
          while (!(SPSR & (1<<SPIF)));
          SPDR=addr;    //Начальный адрес - голова [7..0]    
          while (!(SPSR & (1<<SPIF)));  
        }	
    }
    while(--len);
    s->block_length=bl;    

GET_hook_DATA_RX_exit:
    
    FM_CS_DISABLE();
    MP3fifo_pWR = pWR;        
    // Записали пакет
    // Теперь высчитываем оставшееся место в буфере FRAM 
    // Устанавливаем размер окна сокета равный числу свободных байт в буфере FRAM
    UINT32 freeFIFO=_MP3fifo_free();
    if (freeFIFO < TCP_MAX_DATA_LEN)
    {   
      // считаем что в FIFO места уже нет - обнуляем окно
      // соответственно можно начать играть в основном цикле
      GET_WINDOW_STATE=GET_WINDOW_STATE_ZERO;
      freeFIFO=0; 
      #ifdef CONSOLE_DEBUG
      _print_fstr("\r\nZEROWIN");
      #endif
    }    
    else if (freeFIFO>=SPEC_MAX_WIN) freeFIFO=SPEC_MAX_WIN;
//    else if (freeFIFO>=(0xFFFF-512)) freeFIFO=0xFFFF;
//    else freeFIFO+=512;
    s->sock.win=htons((UINT16)(freeFIFO)); // меняем размер окна
    return 0; // Данные тела кончились - выходим...
  }
  if (state==_GET_METADATA)
  {
  L_METADATA:
    //Быстрая обработка метаданных
    if (!len) goto GET_hook_DATA_RX_exit;//{FM_CS_DISABLE(); return 0;}
    bl=s->block_length;
    do
    {
      if (!(--bl))
      {
	//Блок закончился, в c - размер метаданных
	s->state=state=_GET_BODY;
	s->block_length=s->metadata_interval;
	len--;
	goto L_BODY2;
      }
      UREG c=*data++;
     //Тут можно накапливать байты метаданных, если надо
     //UDR=c; // выводим в консоль
//     if (META_len++<31) *pMETA_buf++=c;
     LCD_putMETA(1,c); 
    }
    while(--len);
    s->block_length=bl;
  }
  // Идет прием... 
  if (!len) goto GET_hook_DATA_RX_exit;//{FM_CS_DISABLE();return 0;}
  UREG pos=s->header_pos; // pos - текущая позиция в строке-заголовке
  // Пробежимся по заголовкам
  do 
  {
    c=*data++;
    switch(state)
    {
        case _GET_HEADER: 
        // Если сейчас идет прием заголовка (Header)  
          if (c==10) // поймали '\n'  (Конец строки)
          {      
            s->req[pos]=0;
            if (pos) // если позиция в строке не нулевая
            {       
              if (!stricmp_P(s->req,"icy-metaint")) // поймали укзатель длины блока метаданных?
              {
                //Размер блока между метаданными
                s->metadata_interval=a2i(s->req+12)+1; //Запомним размер блока метаданных (еще вычитываем байт длинны метаданных)
              }
              else if (!stricmp_P(s->req,"icy-br")) // поймали укзатель битрейта
              {
                //Битрейт
                LCD_putBR(s->req+7);
              }
            }   
            else // поймали \r\n (Конец заголовков, переключаемся на тело)
            {              
              //__no_operation();
              s->state=state=_GET_BODY;
              s->block_length=s->metadata_interval; // Запоминаем размер блока между метаданными
              len--;
              goto L_BODY; // Перейдем на обработку метаданных
            }
            pos=0; //Начинаем новое накопление строки
            break;
         }
         if (c==13) break; //Пропускаем
         if (pos<(sizeof(s->req)-1))
         {
            //Если есть куда сохранять символы, сохраняем название заголовка
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
      // пока нет места для приема полноценного окна не делаем асинхронный старт      
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
    s->sock.win=htons((UINT16)freeFIFO); // запишем фактическое окно
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
      return 1; //Запрос передачи
    }
    if (s->state<_GET_HEADER) return 0;
    return GET_hook_DATA_RX(len,data,s);
  case TCP_EVENT_ACK:
    //Подтверждение данных
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
            *d++=*es++;//  "запрос"
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