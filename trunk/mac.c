/* Name: mac.c
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#include "nike_e.h"

#ifdef DEV_IS_MAC

__no_init ETH_FRAME ETH_PKT;// @ ETH_PKT_BASE;

//volatile char ETH_RX_HARDWARE_ERROR1;
//volatile char ETH_RX_HARDWARE_ERROR2;

//volatile char COPY_EIMSK;
//volatile char COPY_TIMSK0;

/* * commented for ENC28J60 port

*/
__monitor void ETH_STOP_BACK_PRESSURE(void)
{
  /* commented for ENC28J60 port
  if (ETH_PKT_mode) return;
  if (ETH_TXEN)
  {
    ETH_DIR_RX;
    ETH_TXEN=0;
  }*/
}

void InitEthernetHW(void)
{
  UINT8 *p=(UINT8 *)(&ETH_PKT);
  do
  {
    *p++=0;
  }while(P_HIGH(p)!=P_HIGH(&ETH_PKT+1));
  ETH_INIT_TX_INT;
    /* ENC28j60 patch
  ETH_INIT_PORT_1;
  ETH_INIT_PORT_2;
  ETH_INIT_RX_INT;
  ETH_INIT_TX_INT;
  ETH_INIT_NETWORK_INT; // start-up Timer
   */
}

#endif






