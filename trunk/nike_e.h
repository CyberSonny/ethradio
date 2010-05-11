/* Name: nike_e.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */


#ifndef NIKE_E_H
#define NIKE_E_H


//#define DBG_LED PORTC_Bit0

#define CPU8BIT

#include "iom32.h"

#define ETH_TX_DATA_OR (0x03)

#ifndef __IAR_SYSTEMS_ASM__


#define TIMER_TASK_LOCK TWAR_Bit2
#define TIMER_TASK_TMR TWAR_Bit3


#define NULL ((void*)0)

#include "stdafx.h"
#include "ethernet.h"

//Определяем, что мы эмулируем
// MAC (подключен PHY)
#define DEV_IS_MAC
//или PHY (switch в качестве MAC)
//#define DEV_IS_PHY

//-----------------------------------------------------------------------------
#ifdef DEV_IS_MAC


//Вроде этот бит не используется
#define TIMER_200ms TWAR_Bit4

#define ETH_INIT_TX_INT {TCCR0=1;TIMSK|=1<<OCIE0;}

//#define ETH_RX_DV_REG (nibble2&0x08)


#endif


#ifdef DEV_IS_PHY

void PHY_emul(void);

#endif


//#define ENABLE_INT_ETH()      {EECR_EERIE=1;}
//#define ENABLE_INT_ETH()      {EECR_Bit3=1;}
//#define DISABLE_INT_ETH()      {EECR_Bit3=0;}
#define WAKEUP_ETH() {ETH_TASK_WAKEUP=1;}



//#define ETH_CANT_TX PORTB_Bit1
#define ETH_TASK_WAKEUP TWAR_Bit1

#define ETH_PKT_END (RAMEND+1)
#define ETH_PKT_BASE (ETH_PKT_END-sizeof(ETH_FRAME))

#define P_HIGH(var) ((unsigned int)(var)>>8)
#define P_LOW(var) ((UINT8)(unsigned int)(var))

extern ETH_FRAME ETH_PKT;
extern /*volatile */UINT16 ETH_PKT_len;
extern /*volatile */UINT8 ETH_PKT_mode; //0 - свободен, 1 - занят приемником, 2-16 - счетчик передач

//extern ETH_FRAME *P_ETH_PKT;

#define IP2UINT32(b1,b2,b3,b4) (b1*0x1UL+b2*0x100UL+b3*0x10000UL+b4*0x1000000UL)

void InitEthernetHW(void);
__x_z void netw_memcpy(void *d, void *s, unsigned int l);

#else


#define ETH_PKT_END (RAMEND+1)

	EXTERN	COPY_EIMSK
	EXTERN	COPY_TIMSK0
	  
	EXTERN	ETH_PKT
	EXTERN	ETH_PKT_len
	EXTERN	ETH_PKT_mode

#endif


#endif


