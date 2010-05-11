#include "compiler.h"
#include "stdafx.h"
#include "tcp.h"
#include "http_get.h"
#include "station.h"
#include "FIFO.h"
#include "lcd.h"
#include "player.h"
#include "kb.h"


extern volatile UREG PLAYER_STATE;
void LCD_TCPstate(UINT8 sock_state)
{   
  switch (sock_state)
    {
      case TCP_STATE_CLOSED:
        LCD_fprintline(1,"Closed");
        //LCD_putMETA(0,0);
        break;
      case TCP_STATE_CONNECTING:
        LCD_fprintline(1,"Connecting");
        break;
      case TCP_STATE_FINW1:
        LCD_fprintline(1,"Closing");
        break;
      case TCP_STATE_CONNECTED:
        LCD_fprintline(1,"Connected");
        break;
    }
}      

void LCD_PLAYERstate(UINT8 player_state)
{
   switch (player_state)
   {
        case PLAYER_STATE_STOPED:
        LCD_fprintline(1,"Stoped");
        //LCD_putMETA(0,0);
        break;
      case PLAYER_STATE_BUFFERING:
        LCD_fprintline(1,"Buffering");
        break;
      case PLAYER_STATE_PLAYING:
        LCD_fprintline(1,"Playing");
        break;
     case PLAYER_STATE_STOPREQ:
        LCD_fprintline(1,"Stop Request");
        break;
   }
}

volatile UINT8 Keyboard_task;

#pragma vector = TIMER2_OVF_vect
__interrupt  void T2_TASK(void)
{
  static UINT8 T2INTdiv;
  if (T2INTdiv++ != 3) return;
  T2INTdiv=0;
  /*
  if (get_sock.sock.state != sock_state)
  {
      LCD_TCPstate(get_sock.sock.state);   
      sock_state=get_sock.sock.state;
  }
  if (player_state!=PLAYER_STATE)
  {
    LCD_PLAYERstate(PLAYER_STATE);
    player_state=PLAYER_STATE;
  }
    */
  if (!PORTA_Bit0)
  {
    if (!PINA_Bit0) // but1 is pressed
    {
      sbi(PORTA, PA0);
    }
  }
  else
  {
    if (PINA_Bit0) //but1 was released
    {
      cbi(PORTA, PA0);      
      if (get_sock.sock.state==TCP_STATE_CLOSED)
      {
        /*
        LCD_fprintline(1,"Connecting");
        StartGET();
        */
        Keyboard_task=Keyboard_task_startget;
      }
      else
      {
        //если сокет в любом состоянии кроме "закрыт" то даем запрос на закрытие
        // и останавливаем воспроизведение
//        PLAYER_STATE= PLAYER_STATE_STOPREQ;
        Keyboard_task=Keyboard_task_stopget;
      }
    }
  }
  if (!PORTA_Bit1)
  {
    if (!PINA_Bit1) // but1 is pressed
    {
      sbi(PORTA, PA1);
    }
  }
  else
  {
    if (PINA_Bit1) // but1 is pressed
    {
      cbi(PORTA, PA1);      
      if (PLAYER_STATE==PLAYER_STATE_STOPED)
      {
        Keyboard_task=Keyboard_task_stationchange;
        /*
        stationNum++;
        if (stationNum>20) stationNum=0;
        stationNum_EEPROM=stationNum;
        LCD_fprintline(0,&station_list[stationNum].LCD_ID[0]);
        */
      }
    }
  }
}
