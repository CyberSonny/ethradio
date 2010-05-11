#ifndef __COMPILER_H__
#define __COMPILER_H__


// choose your AVR device here
#include <iom32.h>
#include <intrinsics.h>
#include <pgmspace.h>
#include <stdbool.h>


//#include <macros.h>

#define outp(val, reg)  (reg = val)
#define inp(reg)        (reg)

#define cli()           __disable_interrupt();
#define sei()           __enable_interrupt();

#define sbi(reg,bit)  (reg |= (1<<bit))   //<! set bit in port
#define cbi(reg,bit)  (reg &= ~(1<<bit))  //<! clear bit in port
#define sbr(reg,bit)  (reg |= (1<<bit))   //<! set bit in port
#define cbr(reg,bit)  (reg &= ~(1<<bit))  //<! clear bit in port

#define SIGNAL(x)       void x(void)  

#define nop() __no_operation();

#define _BV(x)	   (1<<x)

#define XTALL			16.0

#define	_delay_us(us)	__delay_cycles (XTALL * us);
#define _delay_ms(ms)	_delay_us (1000 * ms) 


//typedef unsigned char u8;
//typedef unsigned short u16;

#endif /* __COMPILER_H__ */

