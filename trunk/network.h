#include "stdafx.h"
extern volatile union
{
  struct
  {
    UINT8 count_200ms_low;
    UINT8 count_200ms_high;
    UINT16 irs_high;
  };
  struct
  {
    UINT16 IRS_L;
    UINT16 IRS_H;
  };
};
//extern volatile UINT8 ETH_RXQ0,ETH_RXQ1; //Очередь указателей на прием
//extern UINT8 ETH_TXQ0,ETH_TXQ1; //Очередь указателей на передачу
extern void INT_ETH_PROCESS_PKT2(void);