#include "stdafx.h"
#include "compiler.h"


#define UART_TX(var) \
{\
   while ( !( UCSRA & (1<<UDRE)));\
   UDR = var;\
};


static const char __flash _sData[] = "&Data:";
static const char __flash _sLen[] = "Len:";
static const char __flash _si[] = "i:";

__z void _i2a(char *s, UINT16 v);
extern __z void i2a(char *s, UINT16 v);

void uart_init(void)
{
/* UART0 initialisation */
/* desired baud rate: 115200 */
/* actual baud rate: 115200 (0.0%) */
/* char size: 8 bit */
/* parity: Disabled */
UCSRB = 0x00; /* disable while setting baud rate */
UCSRA = 0x00;
UCSRC = 0x86; // 8 bit data
UBRRL = 8; // set baud rate lo (115200 @ 16 MHz)
UBRRH = 0x00; /* set baud rate hi */
UCSRB = 0x08; //TX are enabled
}


//Print 0x0A,0x0B
void _print_rn (void)
{ 
  UART_TX(0x0A);
  UART_TX(0x0D);
}

//extern char i2a_buf[5];

void _print_num ( const char __flash * s, UINT16 Num)
{
  char* pi2a;
  __no_init char i2a_locbuf[6];
  pi2a=&i2a_locbuf[0];
  i2a(pi2a, Num);
  while (*s) UART_TX (*s++);
//  UART_TX(':'); 
  while (*pi2a) UART_TX(*pi2a++);  
  UART_TX(',');
  UART_TX(' '); 
}

void _print_fstr (const char __flash * s)
{
  while (*s) UART_TX (*s++);
}


/*
static void put_message(char *s)
{
  while (*s)
    putchar(*s++);
}
*/


/*
void print_dump(unsigned char* p, unsigned int len)
{
 // unsigned char i=0;
    printf_P(_HexWordData, p);
  printf_P(_HexWordNum, len);
//  while (i<len)
//  {      
     //printf_P(_HexByteNum, *p);
     //i++;
     //p++;
//     putchar (0x3B);
//  }  
//  printf_P(_Razer);
//  printf_P(_Razer);
}

void print_i (unsigned char i)
{
  printf_P(_DecByteNum, i);
    printf_P(_Razer);
}
void print_lenRX(unsigned char* p, unsigned int len)
{
  if (!len) return;
  unsigned char i=0;

  printf_P(_HexWordNumRX, len);
  while (i<len)
  {      
     printf_P(_HexByteNum, *p);
     i++;
     p++;
     putchar (0x3B);
  }  
  printf_P(_Razer);
//  printf_P(_Razer);
}
*/