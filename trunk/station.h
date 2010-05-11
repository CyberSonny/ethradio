typedef struct
{
  UINT32 IP;
  UINT16 port; 
  char req[32];
  char LCD_ID[16];  
}STATION;

extern __eeprom STATION station_list[16];

extern __eeprom char stationNum_EEPROM;

extern volatile UREG stationNum;