#include "compiler.h"
#include "stdafx.h"
#include "station.h"
#include "nike_e.h"

volatile UREG stationNum;

__eeprom char stationNum_EEPROM=1;

__eeprom STATION station_list[16] = {\
  {IP2UINT32(66,220,3,52),7016,"","1.FM TOP40"},/*1.FM TOP40 AAC+ 64*/
  {IP2UINT32(72,13,93,119),7070,"","1.FM Classical"},/*1.FM Trance AAC+ 64*/
  {IP2UINT32(72,13,93,119),7082,"","1.FM BaySm.Jazz"},/*1.FM Disco AAC+ 64*/
  {IP2UINT32(72,13,93,119),7036,"","1.FM 90's"},/*1.FM 90's AAC+ 64*/
  {IP2UINT32(72,26,204,28),5324,"","DI.FM Cl.Euro"},/*DI FM Classic Eurodance mp396*/
  {IP2UINT32(72,26,204,28),5194,"","DI.FM Ambient"},\
  {IP2UINT32(72,26,204,28),5744,"","DaTempo Lounge"},\
  {IP2UINT32(72,26,204,28),5834,"","SKY.FM Love"},\
  {IP2UINT32(74,55,180,162),8020,"","PERUFOLKRADIO"},/*DI FM Classical Trance AAC+ 24*/
  {IP2UINT32(121,98,168,24),10042,"","NZ TheFlea88.2"},/**/\
  {IP2UINT32(187,17,67,228),9413,"","BR BAND FM103,3"},/**/\
  {IP2UINT32(82,204,217,140),8000,"radiodacha_high.mp3","Radio Dacha"/*dacha96*/},\
  {IP2UINT32(62,122,213,3),8000,"obogrelov-fm","Obogrelov FM"},\
  {IP2UINT32(62,122,213,3),8000,"melodia128","Rad. Melodiya"},\
  {IP2UINT32(89,239,131,142),8080,"Radio_Russia","Radio Russia"},\
  {IP2UINT32(193,42,152,215),8006,"","WRN. Russkij"}
};
//  {IP2UINT32(64,12,61,3),80,"stream/1024"},/**/
//  {IP2UINT32(72,26,204,18),6036,""},
//  {IP2UINT32(72,26,204,18),6324,"","Cl.Eurodance"},/*DI FM Classic Eurodance MP3 96*/
//  {IP2UINT32(192,168,113,2),8021,"","192.168.113.2"},/*DI FM Classic Eurodance MP3 96*/
