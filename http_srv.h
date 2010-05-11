/* Name: http_srv.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

typedef struct
{
  UREG AUTH_MODE;
  UREG AUTH_PRESENT;
}AUTH_FLAGS;

typedef struct
{
  __flash const char *html;
  union
  {
    __flash const char *chunk;
    const char *chunk_ram;
    __eeprom const char *chunk_eeprom;
  };
  __flash const char *stk;
  UREG chunk_mode;
  UREG select_n;
}HTTP_CB;

typedef struct
{
  //UREG state;
  //struct
  //{
    HTTP_CB httpcb; //Ќе измен€ть взаимоположение структур, используетс€ в хитром копировании
    HTTP_CB httpcb_ack;
  //};
  UINT16 value;
  UREG val2;
  UINT16 content_length;
  union
  {
    char http_hdr_item[32]; //—юда же л€жет responce
    char select_name[6];
  };
  //-------------------
  char req[8];  
}_HTTP_CONTROL;

typedef struct
{
  TCP_SOCK sock;
  AUTH_FLAGS af;
  UREG state;
    struct
    {
      //struct
      //{
	HTTP_CB httpcb; //Ќе измен€ть взаимоположение структур, используетс€ в хитром копировании
	HTTP_CB httpcb_ack;
      //};
      UINT16 value;
      UREG val2;
      UINT16 content_length;
      union
      {
	char http_hdr_item[32]; //—юда же л€жет responce
	char select_name[6];
      };
      //-------------------
      char req[8];
      char _MD5_Buffer[128+2];
    };
  UREG statnum;  
} HTTP_SOCK;

extern HTTP_SOCK http_sock;
