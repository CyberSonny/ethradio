extern void LCD_TCPstate(UINT8 sock_state);

extern volatile UINT8 Keyboard_task;
extern void LCD_PLAYERstate(UINT8 player_state);

enum Keyboard_tasks
{
  Keyboard_task_none=0,
  Keyboard_task_startget,
  Keyboard_task_stopget,
  Keyboard_task_stationchange
};