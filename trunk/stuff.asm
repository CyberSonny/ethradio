/* Name: stuff.asm
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#include "nike_e.h"

#ifdef CPU8BIT
	RSEG CODE:CODE:NOROOT(1)
	PUBLIC _i2a
//   13 __z void i2a_9999max(char *s, UINT16 v)
_i2a:
//   14 {
//   15   UINT8 m0; //R16
//   16   UINT8 m1; //R17
//R18-R20 - 24bit fmul result
//R21 - c,b,a ->06 8D B9
//R22 - zero reg
	CLR	R22
	LDI	R21,0x06
//  v=__multiply_unsigned(m0,0x06)+3;
	MUL	R16,R21
	MOVW	R19:R18,R1:R0
	SUBI	R18,0xFD
	SBCI	R19,0xFF
//  v+=__multiply_unsigned(m1,0x06)<<8;
	MUL	R17,R21
	MOV	R20,R1
	ADD	R19,R0
	ADC	R20,R22
//  v+=__multiply_unsigned(m1,0x8D);
        LDI     R21, 0x8D
        MUL     R17, R21
        ADD     R18, R0
        ADC     R19, R1
	ADC	R20, R22
//  v+=__multiply_unsigned(m0,0x8D)>>8;
        MUL     R16, R21
        ADD     R18, R1
        ADC     R19, R22
	ADC	R20, R22
//  v+=__multiply_unsigned(m1,0xB9)>>8;
	LDI	R16,0x10		; Counter & flags
	LDI	R21,0xB9
        MUL     R17, R21
        LDI     R21, 10			; Next multiplicand
        ADD     R18, R1
        ADC     R19, R22
	ADC	R20, R22
	BREQ	??i2a_0
	SUBI	R20,208
	ST	Z+,R20
	INC	R16
??i2a_0:
//   39     UINT16 hv;
//   40     UINT8 bv;
//   41     bv=v>>8;
        MOV     R17, R19
//   42     v=__multiply_unsigned(v,10);
        MUL     R18, R21
        MOVW    R19:R18, R1:R0
//   43     hv=__multiply_unsigned(bv,10);
        MUL     R17, R21
//   44     v+=(hv&0xFF)<<8;
        ADD     R19, R0
//   45     if (SREG_Bit0) hv+=0x100;
	ADC	R1, R22
//   46     bv=hv>>8;
        MOV     R17, R1
//   47     if ((i|bv)&0x8F)
        MOV     R20, R1
        OR      R20, R16
        ANDI    R20, 0x8F
        BREQ    ??i2a_1
//   48     {
//   49       *s++=bv+'0';
	SUBI	R17,208
	ST	Z+,R17
//   50       i|=1;
//        ORI     R18, 0x01
??i2a_1:
//   51     }
//   52     i<<=1;
	ROL	R16
//   54   while(!SREG_Bit0);
        BRBC    0, ??i2a_0
//   55   *s=0;
        ST      Z, R22
//   56 }
        RET

#endif
/*
CRC32polyInv	var	0xEDB88320	;CRC32 polynom (0x04C11DB7) in inverted bits order


COMPUTE_CRC32 MACRO byte
Message	var	byte
	rept	8
	if ((CRC32^Message)&1)		;if xor of CRC32[MSB] and Message[LSB] is one (note: CRC32 has now inverted bit order)
CRC32	var	((CRC32>>1)^CRC32polyInv)	;shift the CRC and make xor with polynom
	else					;if not
CRC32	var	(CRC32>>1)			;only shift the CRC
	endif
Message	var	(Message>>1)		;go to next bit in Message
	endr
	endm


	ASEGN ABSOLUTE:CODE:NOROOT,FLASHEND-0x3FF

	PUBLIC	CRC32_TABLE
CRC32_TABLE:

v	var	0
	REPT	256
CRC32	var	0
	COMPUTE_CRC32 v
	DD	CRC32
v	var	v+1
	ENDR

//Compute CRC32_GOOD
CRC32	var	0xFFFFFFFF
	COMPUTE_CRC32 0
	COMPUTE_CRC32 0
	COMPUTE_CRC32 0
	COMPUTE_CRC32 0

CRC32_GOOD	EQU	CRC32
*/
	END
        
	