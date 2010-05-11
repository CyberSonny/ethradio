/* Name: network_routines_avr.c
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

//---- Процедуры всяческие, оптимизированные под AVR
static __x_z void _netw_memcpy(char *d, char *s, UREG l, UREG hl);

__x_z void netw_memcpy(void *d, void *s, unsigned int l)
{
  _netw_memcpy((char*)d,(char*)s,l,l>>8);
}

static __x_z void _netw_memcpy(char *d, char *s, UREG l, UREG hl)
{
  char c;
  if (l) hl++;
  do
  {
    do
    {
      c=*s++;
      *d++=c;
    }
    while(--l);
  }
  while(--hl);
}

static __x_z UINT32 *IPcpy(UINT32 *d, UINT32 *s);
static __x UINT32 *IPcpyIP(UINT32 *d)
{
  return IPcpy(d,&IP);
}

static __x_z UINT32 *IPcpy(UINT32 *d, UINT32 *s)
{
  *d=*s;
  return d;
}

#define INT32cpy IPcpy

//------------------------------------------------------------------------
// Процедуры 32хбитной математики
//------------------------------------------------------------------------
static __x_z int _cmp32(UINT32 *a, UINT32 *b);
#pragma optimize=no_inline
static __x_z int cmp_A_S(TCP_PKT *tcp, TCP_SOCK *s)
{
  return _cmp32(&tcp->tcp.ackno+1,&s->SEQNO+1);
}
  
#pragma optimize=no_inline
static __x_z int cmp_S_A(TCP_PKT *tcp, TCP_SOCK *s)
{
  return _cmp32(&tcp->tcp.seqno+1,&s->ACKNO+1);
}

#pragma optimize=no_inline
static __x_z int _cmp32(UINT32 *a, UINT32 *b)
{
//  asm("ADIW   R27:R26,4");
//  asm("ADIW   R31:R30,4");
  asm("LD     R16,-X");
  asm("LD     R21,-Z");
  asm("SUB    R16,R21");
  asm("LD     R17,-X");
  asm("LD     R21,-Z");
  asm("SBC    R17,R21");
  asm("LD     R18,-X");
  asm("LD     R21,-Z");
  asm("SBC    R18,R21");
  asm("LD     R19,-X");
  asm("LD     R21,-Z");
  asm("SBC    R19,R21");
//  asm("BRCS   cmp32i_1");
  asm("OR     R18,R19");
  asm("BRNE   cmp32i_1");
  asm("RET");
  asm("cmp32i_1:");
  return -1;
}

static __z void inc32i(UINT32 *d, unsigned int i);
static __z void inc32(UINT32 *a, UREG i);

static __x_z void inc32cpy(UINT32 *d, UINT32 *s)
{
  inc32(INT32cpy(d,s),1);
}

#pragma optimize=no_inline
static __z void inc32(UINT32 *a, UREG i)
{
  inc32i(a,i);
}

static __z void inc32i(UINT32 *d, unsigned int i)
{
  *d=htonl(ntohl(*d)+i);
}

//---------------------------------------------------------------------
/*
#define FAST_CRC32
//#pragma inline=forced
#ifdef FAST_CRC32
static __x UREG _CRC32(UINT8 *p, UREG c1, UREG c2, UREG c3, UREG c4, unsigned int len, UREG mv, UREG do_write);
static __x UREG CRC32(UINT8 *p, unsigned int len, UREG do_write);

#pragma optimize=no_inline
static __x UREG CRC32_M4(UINT8 *p, unsigned int len)
{
  return CRC32(p,len-4,0);
}

#pragma optimize=no_inline
static __x UREG CRC32(UINT8 *p, unsigned int len, UREG do_write)
{
  return _CRC32(p,0xFF,0xFF,0xFF,0xFF,len,4,do_write);
}
//#pragma diag_suppress=Pe940  
static __x UREG _CRC32(UINT8 *p, UREG c1, UREG c2, UREG c3, UREG c4, unsigned int len, UREG mv, UREG do_write)
{
  extern char *CRC32_TABLE;
  //const char __flash *z;
  __require(CRC32_TABLE);
//  do
//  {
  asm("_CRC32_1:");
//    z=(const __flash char *)(__multiply_unsigned(*p++^c1,mv));
  asm("LD      R3, X+");
  asm("EOR     R3, R16");
  asm("MUL     R3, R22");
  asm("MOVW    R31:R30, R1:R0");
//    z+=(int)CRC32_TABLE;
  asm("SUBI   R31,-(CRC32_TABLE>>8)");
//    c1=*z++;
  asm("LPM     R16, Z+");
//    c1^=c2;
  asm("EOR     R16,R17");
//    c2=*z++;
  asm("LPM     R17, Z+");
//    c2^=c3;
  asm("EOR     R17,R18");
//    c3=*z++;
  asm("LPM     R18, Z+");
//    c3^=c4;
  asm("EOR     R18,R19");
//    c4=*z;
  asm("LPM     R19, Z+");
//  }
  asm("SUBI     R20,1");
  asm("SBCI     R21,0");
//  while(--len);
  asm("BRNE    _CRC32_1");
//  c1=~c1;
  asm("COM     R16");
//  c2=~c2;
  asm("COM     R17");
//  c3=~c3;
  asm("COM     R18");
//  c4=~c4;
  asm("COM     R19");
  if (do_write)
  {
    *p++=c1;
    *p++=c2;
    *p++=c3;
    *p++=c4;
  }
  else
  {
    c1-=*p++; if (c1) return 1;
    c2-=*p++; if (c2) return 1;
    c3-=*p++; if (c3) return 1;
    //Для RTL8201BL не производим расчет последнего байта
    //c4-=*p++; if (c4) return 1;
  }
  return 0;
//  return (((UINT32)c1<<0)|((UINT32)c2<<8)|((UINT32)c3<<16)|((UINT32)c4<<24));
}
//#pragma diag_default=Pe940
#else
static __z UINT32 _CRC32(UINT8 *p, UREG len, UINT32 crc_poly);

static __z UINT32 CRC32(UINT8 *p, UREG len)
{
  return _CRC32(p,len,0xEDB88320);
}
  
static __z UINT32 _CRC32(UINT8 *p, UREG len, UINT32 crc_poly)
{
  UREG bitcnt;
  UINT32 crc=0xFFFFFFFFL;
  do
  {
    crc^=*p++;
    bitcnt=2;
    do
    {
      crc>>=1;if (SREG_Bit0) crc^=crc_poly;
      crc>>=1;if (SREG_Bit0) crc^=crc_poly;
      crc>>=1;if (SREG_Bit0) crc^=crc_poly;
      crc>>=1;if (SREG_Bit0) crc^=crc_poly;
    }
    while(--bitcnt);
  }
  while(--len);
  return ~crc;
}
#endif   

*/

/*
__z UINT16 IPChecksum(UINT16 *data, UINT16 len)
{
  UINT32 acc=0;
  UINT16 l=len>>1;
  if (l)
  {
    do
    {
      acc+=*data++;
      acc&=(0x10000ULL<<(sizeof(len)*8))-1;
    }
    while(--l);
  }
  if (len&1)
  {
    acc+=*(UINT8 *)data;
    acc&=(0x10000ULL<<(sizeof(len)*8))-1;
  }
  UINT16 acc16=acc;
  acc16+=acc>>16;
  if (SREG_Bit0) acc16++; // Overflow, so we add the carry to acc (i.e., increase by one) 
  // return invert checksum 
  return ~acc16;
}
*/
static __x UINT16 IPChecksum(UINT16 *data, unsigned int len)
{
  UINT16 acc;
  for(acc = 0; len > 1; len -= 2)
  {
    acc += *data++;
    if (SREG_Bit0) acc++; /* Overflow, so we add the carry to acc (i.e., increase by one) */
  }
  // add up any odd byte
  if(len == 1)
  {
    acc += *(UCHAR *)data;
    if (SREG_Bit0) acc++; /* Overflow, so we add the carry to acc (i.e., increase by one) */
  }
  // return invert checksum
  return ~acc;	
}

//Обратный порядок, необходимо перед вызовом делать +ETH_HWA_LEN
static __x_z void MACcpy(UINT8	*d, UINT8 *s)
{
  *--d=*--s;
  *--d=*--s;
  *--d=*--s;
  *--d=*--s;
  *--d=*--s;
  *--d=*--s;
}

static __x_z UREG IPcmp(UINT32 *d, UINT32 *s);

static __x_z UREG MACcmp(UINT8 *d, UINT8 *s)
{
  UINT16 vd;
  UINT16 vs;
  vd=*(UINT16*)d;
  d+=2;
  vs=*(UINT16*)s;
  s+=2;
  if (vd!=vs) return 1;
  return IPcmp((UINT32*)d,(UINT32*)s);
}

static __x_z UREG IPcmp(UINT32 *d, UINT32 *s)
{
  if (*d!=*s) return 1;
  return 0;
}

static __x_z void _swapmem(char *d, char *s, UREG l);

static __x void swapmem(void *d, UREG l)
{
  _swapmem((char*)d,(char*)d+l,l);
}

static __x_z void _swapmem(char *d, char *s, UREG l)
{
  UREG c;
  do
  {
    c=*d;
    *d++=*s;
    *s++=c;
  }
  while(--l);
}

static __z _netw_memset(char *d, UREG v, UREG n);

__z netw_memset(void *d, UREG v, UREG n)
{
  _netw_memset((char*)d,v,n);
}

static __z _netw_memset(char *d, UREG v, UREG n)
{
  do
  {
    *d++=v;
  }
  while(--n);
}
