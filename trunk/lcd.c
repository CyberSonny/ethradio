
#include "compiler.h"
#include "stdafx.h"
#include "lcd.h"

#define LCD_CLR               0      /* DB0: clear display                  */
#define LCD_HOME              1      /* DB1: return to home position        */

#define LCD_ENTRY_MODE        2      /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         1      /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       0      /*   DB2: 1=display shift on           */

#define LCD_ON                3      /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        2      /*   DB2: turn display on              */
#define LCD_ON_CURSOR         1      /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          0      /*     DB0: blinking cursor ?          */

#define LCD_MOVE              4      /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         3      /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        2      /*   DB2: move right (0-> left) ?      */

#define LCD_FUNCTION          5      /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   3      /*   DB3: two lines (0->one line)      */
#define LCD_FUNCTION_10DOTS   2      /*   DB2: 5x10 font (0->5x7 font)      */

#define LCD_CGRAM             6      /* DB6: set CG RAM address             */
#define LCD_DDRAM             7      /* DB7: set DD RAM address             */
#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define LCD_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */
#define LCD_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */
#define LCD_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */
#define LCD_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */

/* display on/off, cursor on/off, blinking char at cursor position */
#define LCD_DISP_OFF             0x08   /* display off                            */
#define LCD_DISP_ON              0x0C   /* display on, cursor off                 */
#define LCD_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */
#define LCD_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */
#define LCD_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */

/* move cursor/shift display */
#define LCD_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */
#define LCD_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */
#define LCD_MOVE_DISP_LEFT       0x18   /* shift display left                     */
#define LCD_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */

/* function set: set interface data length and number of display lines */
#define LCD_FUNCTION_4BIT_1LINE  0x20   /* 4-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_4BIT_2LINES 0x28   /* 4-bit interface, dual line,   5x7 dots */
#define LCD_FUNCTION_8BIT_1LINE  0x30   /* 8-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_8BIT_2LINES 0x38   /* 8-bit interface, dual line,   5x7 dots */

#define LCD_MODE_DEFAULT     ((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC) )

#define LCD_PORT  PORTC
#define LCD_DDR   DDRC
#define LCD_RS    PC4
#define LCD_RW    PC5
#define LCD_E     PC6
#define LCD_PIN   PINC

#define LCD_toggle_e()  sbi(LCD_PORT, LCD_E); __delay_cycles (XTALL/2);  cbi(LCD_PORT, LCD_E);

#pragma inline = forced
UREG LCD_RD (UREG mode)
{  
  //LCD_PORT|=0x0F;
  if (mode==0)
  {
    cbi (LCD_PORT, LCD_RS); //  Read busy flag (DB7) and address counter (DB0 to DB7)
  }
  else
  {
    sbi (LCD_PORT, LCD_RS); //  Read data from DDRAM or CGRAM (DDRAM or CGRAM to DR)
  }    
  sbi (LCD_PORT, LCD_RW); // read mode
  
  LCD_DDR&= ~(0x0F); // D3-D0 as input
  
  sbi (LCD_PORT, LCD_E);
  __delay_cycles (XTALL/2);
  UREG data = LCD_PIN<<4; //read high nibble
  cbi (LCD_PORT, LCD_E);  
  __delay_cycles (XTALL/2);
  sbi (LCD_PORT, LCD_E);
  __delay_cycles (XTALL/2);
  data |= (LCD_PIN&0x0F); //read low nibble  
  cbi (LCD_PORT, LCD_E);  
    __delay_cycles (XTALL/2);
  LCD_PORT|=0x0F;
  return data;        
}

#pragma inline = forced
void LCD_WR (UREG data, UREG mode)
{
  if (mode) //mode == 1 "Data write"
  {
   sbi (LCD_PORT, LCD_RS);
  }
  else //mode == 0 "Instruction write"
  {
   cbi (LCD_PORT, LCD_RS);
  }
  cbi (LCD_PORT, LCD_RW);   
  LCD_DDR|= 0x0F; // D3-D0 as outputs
  LCD_PORT = (LCD_PORT & (0xF0))|(data>>4); //output MSB nibble
  LCD_toggle_e();
  LCD_PORT = (LCD_PORT & (0xF0))|(data&0x0F); //output LSB nibble
  LCD_toggle_e();
  LCD_PORT|=0x0F;
}

#pragma inline = forced
UREG LCD_busy_AC_RD (void)
{
  while (LCD_RD(0)&(1<<7));
//  _delay_us(4);
   return (LCD_RD(0));  // return address counter
}

//#pragma inline = forced
void LCD_CMD (UREG cmd)
{
  LCD_busy_AC_RD();
  LCD_WR(cmd,0);  
}

void LCD_DATA (UREG data)
{
}


__flash char WIN1251_tab[]={'A',0xa0,'B',0xa1,0xe0,'E',0xa3,\
0xa4,0xa5,0xa6,'K',0xa7,'M','H','O',0xa8,'P','C','T',0xa9,0xaa,'X',0xe1,0xab,0xac,0xe2,0xad,\
0xae,'b',0xaf,0xb0,0xb1,\
'a',0xb2,0xb3,0xb4,0xe3,'e',0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,'o',0xbe,'p','c',0xbf,'y',\
0xe4,'x',0xe5,0xc0,0xc1,0xe6,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7};

//#pragma inline = forced
void LCD_putc(char c)
{
    UREG sym;
    UREG pos;    
    pos = LCD_busy_AC_RD();   // read busy-flag and address counter     
    if (c>=0xc0) sym=WIN1251_tab[c-0xc0];
    else if (c==0xa8) sym=0xa2;
    else if (c==0xb8) sym=0xb5;
    else sym=c;
    LCD_WR(sym, 1);
    pos = LCD_busy_AC_RD();   // read busy-flag and address counter       

        if ( pos == LCD_START_LINE1+LCD_DISP_LENGTH ) {
            LCD_WR((1<<LCD_DDRAM)+LCD_START_LINE2,0);    
        }else if ( pos == LCD_START_LINE2+LCD_DISP_LENGTH ) {
            LCD_WR((1<<LCD_DDRAM)+LCD_START_LINE3,0);
        }else if ( pos == LCD_START_LINE3+LCD_DISP_LENGTH ) {
            LCD_WR((1<<LCD_DDRAM)+LCD_START_LINE4,0);
        }else if ( pos == LCD_START_LINE4+LCD_DISP_LENGTH ) {
            LCD_WR((1<<LCD_DDRAM)+LCD_START_LINE1,0);
        }

        LCD_busy_AC_RD();

}/* lcd_putc */



void LCD_fprintstr (char __flash* p)
{
  while (*p)
  {
    UREG a=*p++;
    LCD_putc(a);
  }
}

//#pragma inline = forced
void LCD_home(void)
{
    LCD_CMD(1<<LCD_HOME);
}

//#pragma inline = forced
void LCD_CLS(void)
{
    LCD_CMD(1<<LCD_CLR);
}

//#pragma inline = forced
void LCD_XY(UINT8 x,UINT8 y)
{
    if ( y==0 )
        LCD_CMD((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else if ( y==1)
        LCD_CMD((1<<LCD_DDRAM)+LCD_START_LINE2+x);
    else if ( y==2)
        LCD_CMD((1<<LCD_DDRAM)+LCD_START_LINE3+x);
    else /* y==3 */
        LCD_CMD((1<<LCD_DDRAM)+LCD_START_LINE4+x);
}

__z void LCD_fprintlineEE (UINT8 line, char __eeprom* p)
{
  UINT8 len, k;
  switch (line)
  {
  case 0:
    len=16;
    break;
  case 1:
    len=10;
    break;
  case 2:
    break;
  case 3:
    break;
  default:
    return;    
  }
  k=0;
  LCD_XY(0,line);
  /*
  UINT8 i=16;
  while (i--)
  {
    LCD_putc(' ');
  }
  */
  LCD_XY(0,line);
  while (*p&&(k<(len-1)))
  {
    LCD_putc(*p++);
    k++;
  }
  while ((len-k)>0)
  {
    LCD_putc(' ');
    k++;
  }
}


__z void LCD_fprintline (UINT8 line, char __flash* p)
{
  UINT8 len, k;
  switch (line)
  {
  case 0:
    len=16;
    break;
  case 1:
    len=10;
    break;
  case 2:
    break;
  case 3:
    break;
  default:
    return;    
  }
  k=0;
  LCD_XY(0,line);
  /*
  UINT8 i=16;
  while (i--)
  {
    LCD_putc(' ');
  }
  */
  LCD_XY(0,line);
  while (*p&&(k<(len-1)))
  {
    LCD_putc(*p++);
    k++;
  }
  while ((len-k)>0)
  {
    LCD_putc(' ');
    k++;
  }
}

void LCD_init_4 (void)
{
  LCD_DDR=(1<<LCD_DDR)|(1<<LCD_RS)|(1<<LCD_RW)|(1<<LCD_E);
  _delay_ms(20);
  LCD_PORT=(LCD_FUNCTION_8BIT_2LINES>>4);
  LCD_toggle_e();
  _delay_ms(5);
  LCD_toggle_e();
  _delay_us(100); 
  LCD_toggle_e();
  _delay_us(100); 
  LCD_PORT=(LCD_FUNCTION_4BIT_2LINES>>4);
  LCD_toggle_e();
  _delay_us(100); 
  LCD_CMD(LCD_FUNCTION_4BIT_2LINES);
  LCD_CMD (1<<LCD_CLR);
  LCD_CMD ((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC));
  LCD_CMD (LCD_DISP_ON);
  LCD_CLS();
}

void LCD_putMETA (UINT8 mode, char c)
{
  static UINT8 pos, prevchar;
  UINT8 y, pos2;
  if (mode==0)            // сначала сотрем место под теги
  {    
    pos=0; prevchar=0;
    LCD_XY(0,2);
    for (y=0;y<=31;y++)
    {
       LCD_putc(' ');
    }
    return;
  }
  if (pos==255) return;
  if (pos++<13) return; // пропустим первые 13 символов 
  if (c=='\'')            // пришла одинарная кавычка
  {
    prevchar = c;         // запомнили, что приходила кавычка
    pos--;
    return;
  }
  if (prevchar=='\'') 
  {
    if (c==';') pos =255; // '; ставим маркер - больше не печатать
    else // напечатаем кавычку с символом
    {
         pos2=pos-14;         
         if (pos2>31) return;
         cbi (TIMSK, TOIE2);  
         LCD_XY((pos2)&0x0F,(pos2>>4)+2); // переместим курсор        
         LCD_putc(prevchar);         
         pos++;
         sbi (TIMSK, TOIE2);
         pos2=pos-14;         
         if (pos2>31) return;
         cbi (TIMSK, TOIE2);  
         LCD_XY((pos2)&0x0F,(pos2>>4)+2); // переместим курсор        
         LCD_putc(c);
         prevchar=0x00;
//         pos++;
        //  if (pos2<(31+13)) pos++;
         sbi (TIMSK, TOIE2);
         return;
    }
  }
  // нормальная работа - печатаем 1 символ
 else
 {
         pos2=pos-14;         
         if (pos2>31) return;
         cbi (TIMSK, TOIE2);  
         LCD_XY((pos2)&0x0F,(pos2>>4)+2); // переместим курсор        
         LCD_putc(c);         
//         pos++;
         sbi (TIMSK, TOIE2);
 }
}

__z void LCD_putBR (char* s)
{
  LCD_XY(10,1);
  while (*s)
  {
    LCD_putc(*s++);
  }
   LCD_fprintstr ("Kbs");
}

void LCD_softCLR()
{
  UREG i;
  LCD_XY(10,1);
  for (i=0;i<=37;i++) LCD_putc(' ');
}