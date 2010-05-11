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

typedef struct
{
  TCP_SOCK sock;
  UREG state;
  UINT8 header_pos;
  UINT16 block_length;
  UINT16 metadata_interval;
  char req[32];
}POST_SOCK;

POST_SOCK post_sock;

extern __eeprom UINT32 OUTG_IP[2];
//extern char i2a_buf [5];

__z void _i2a(char *s, UINT16 v);
extern __z void i2a(char *s, UINT16 v);
extern void *debug_addr;

void AddPOSTsocket(void)
{
  __x UREG POST_hook(UREG state, UREG len, UINT8 *data, TCP_SOCK *_s);
  post_sock.sock.type=TCP_TYPE_CLIENT;
  post_sock.sock.state=TCP_STATE_CLOSED;
  post_sock.sock.hook=POST_hook;
  post_sock.sock.win=htons(0x800);
  debug_addr=&post_sock;
  AddTCPsocket(&post_sock.sock);
}

void StartPOST(void)
{
  if (post_sock.sock.state==TCP_STATE_CLOSED) TCPconnect(&post_sock.sock,OUTG_IP[0],80);  
}

enum HTTP_POST_STATE
{
   _POST_STOP=0, //Остановленно
  _POST_CONREQ, //Запрос соединения
  _POST_SENDREQ, //Посылка HTTP-запроса
  _POST_WAITACK, //Ожидание подтверждения посылки
  _POST_RESP // Ответ сервера
};

static __x UREG POST_hook(UREG state, UREG len, UINT8 *data, TCP_SOCK *_s)
{
  POST_SOCK *s=(POST_SOCK *)_s;
//  UREG i;
//  UREG j;
  switch(state)
  {
  case TCP_EVENT_CLOSE:
  case TCP_EVENT_ABORT:
    s->state=_POST_STOP;
    s->header_pos=0;
    s->block_length=0;
    s->metadata_interval=0;
    return 0;
  case TCP_EVENT_CONREQ:
    s->state=_POST_CONREQ;
    return 0;
  case TCP_EVENT_ASYNC_REQ:
    s->sock.async_req=0;
    return 0;
  case TCP_EVENT_DATA:
    if (s->state==_POST_CONREQ)
    {
      s->state=_POST_SENDREQ;
      return 1; //Запрос передачи
    }
    if (s->state<_POST_RESP) return 0;
   // return GET_hook_DATA_RX(len,data,s);
  case TCP_EVENT_ACK:
    //Подтверждение данных
    if (s->state==_POST_WAITACK&&len)
    {
      s->state=_POST_RESP;
    }
    return 0;
  case TCP_EVENT_REGENERATE:
    if (s->state==_POST_WAITACK) s->state=_POST_SENDREQ;
  case TCP_EVENT_SEND:
    if (s->state==_POST_SENDREQ)
    {
      s->state=_POST_WAITACK;
      static __flash char req1[]="POST /sample.php HTTP/1.1\r\nHost: 127.0.0.1\r\nReferer: 127.0.0.1/\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
      static __flash char nn[]="\r\n\r\n"; 
      static char reqbody[]="T1=-13.4&T2=23.4&T3=12.1&H1=88&H2=60&P=940";      
      __no_init char i2a_buf[5];
      i2a(i2a_buf, strlen(reqbody));          
      UREG i=sizeof(req1)-1;
      char __flash *s=req1;
      UINT8 *d=data;
      do *d++=*s++;
      while(--i); // Записали req1
      char *s1 = i2a_buf;
      while (*s1) *d++=*s1++; // Записали длину POST посылки
      s=nn;
      while (*s) *d++=*s++; // Записали \r\n\r\n      
      s1=reqbody;
      i =strlen(reqbody);
      do *d++=*s1++;
      while (--i); // Записали POST посылку      
      s=nn;
      while (*s) *d++=*s++; // Записали \r\n\r\n      
      return sizeof(req1)-1+strlen(i2a_buf)+4+strlen(reqbody)+4;
    }
    return 0;
  }
  return 0;
}