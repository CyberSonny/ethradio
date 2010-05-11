/* Name: md5cheat.c
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#include "stdafx.h"

#define F1(x, y, z) ( ((x) & (y)) | ((~x) & (z)) )
#define F2(x, y, z) ( ((x) & (z)) | ((y) & (~z)) )
#define F3(x, y, z) ( (x) ^ (y) ^ (z) )
#define F4(x, y, z) ( (y) ^ ((x) | (~z)) )

typedef struct 
{
  unsigned long g;
  unsigned char f;
  unsigned char e;
}MD5STEP_DATA;

static const __flash MD5STEP_DATA table[]=
{
  {0xd76aa478, 7,  0 },  
  {0xe8c7b756, 12, 1 }, 
  {0x242070db, 17, 2 }, 
  {0xc1bdceee, 22, 3 }, 
  {0xf57c0faf, 7,  4 },  
  {0x4787c62a, 12, 5 }, 
  {0xa8304613, 17, 6 }, 
  {0xfd469501, 22, 7 }, 
  {0x698098d8, 7,  8 },  
  {0x8b44f7af, 12, 9 }, 
  {0xffff5bb1, 17, 10},
  {0x895cd7be, 22, 11},
  {0x6b901122, 7,  12}, 
  {0xfd987193, 12, 13},
  {0xa679438e, 17, 14},
  {0x49b40821, 22, 15},
  
  {0xf61e2562, 5,  1 },  
  {0xc040b340, 9,  6 },  
  {0x265e5a51, 14, 11},
  {0xe9b6c7aa, 20, 0 }, 
  {0xd62f105d, 5,  5 },  
  {0x02441453, 9,  10}, 
  {0xd8a1e681, 14, 15},
  {0xe7d3fbc8, 20, 4 }, 
  {0x21e1cde6, 5,  9 },  
  {0xc33707d6, 9,  14}, 
  {0xf4d50d87, 14, 3 }, 
  {0x455a14ed, 20, 8 }, 
  {0xa9e3e905, 5,  13}, 
  {0xfcefa3f8, 9,  2 },  
  {0x676f02d9, 14, 7 }, 
  {0x8d2a4c8a, 20, 12},
  
  {0xfffa3942,  4, 5 },  
  {0x8771f681, 11, 8 }, 
  {0x6d9d6122, 16, 11},
  {0xfde5380c, 23, 14},
  {0xa4beea44,  4, 1 },  
  {0x4bdecfa9, 11, 4 }, 
  {0xf6bb4b60, 16, 7 }, 
  {0xbebfbc70, 23, 10},
  {0x289b7ec6, 4,  13}, 
  {0xeaa127fa, 11, 0 }, 
  {0xd4ef3085, 16, 3 }, 
  {0x04881d05, 23, 6 }, 
  {0xd9d4d039, 4,  9 },  
  {0xe6db99e5, 11, 12},
  {0x1fa27cf8, 16, 15},
  {0xc4ac5665, 23, 2 }, 
  
  {0xf4292244,  6, 0 },  
  {0x432aff97, 10, 7 }, 
  {0xab9423a7, 15, 14},
  {0xfc93a039, 21, 5 }, 
  {0x655b59c3,  6, 12}, 
  {0x8f0ccc92, 10, 3 }, 
  {0xffeff47d, 15, 10},
  {0x85845dd1, 21, 1 }, 
  {0x6fa87e4f,  6, 8 },  
  {0xfe2ce6e0, 10, 15},
  {0xa3014314, 15, 6 }, 
  {0x4e0811a1, 21, 13},
  {0xf7537e82,  6, 4 },  
  {0xbd3af235, 10, 11},
  {0x2ad7d2bb, 15, 2 }, 
  {0xeb86d391, 21, 9 }
};

/*=============================================*/
static __x_z void _move(char *d, const char *s, UREG l);
__x_z void back_memmove(void *d, const void *s, UREG l)
{
  _move((char*)d,(char*)s,l);
}

static __x_z void _move(char *d, const char *s, UREG l)
{
  do
  {
    *--d=*--s;
  }
  while(--l);
}

#pragma inline=forced
static __x_z void _memcpy_flash(char *d, const char __flash *s, UREG l);
#pragma inline=forced
static __x_z void memcpy_flash(void *d, const void __flash *s, UREG l)
{
  _memcpy_flash((char*)d,(char __flash *)s,l);
}

#pragma inline=forced
static __x_z void _memcpy_flash(char *d, const char __flash *s, UREG l)
{
  do
  {
    *d++=*s++;
  }
  while(--l);
}


static __x_z void sum(unsigned long *state, unsigned long *a)
{
  UREG i=4;
  unsigned long t;
  do
  {
    t=*a++;
    t+=*state;
    *state++=t;
  }
  while(--i);
}

/*=============================================*/

void MD5cheat(char *out_, unsigned char *_p, UREG len)
{
  unsigned long state[4];
  static __flash unsigned long state_init[4]={0x67452301,0xefcdab89,0x98badcfe,0x10325476};
  memcpy_flash(state,state_init,16);
  {  
    unsigned char *wp=_p+len;
    unsigned long *p=(unsigned long *)_p;
    UREG i=len;
    if ((i&63)!=56)
    {
      *wp++=0x80;
      i++;
    }
    while((i&63)!=56)
    {
      *wp++=0;
      i++;
    }
    {
      UINT16 v=__multiply_unsigned(len,8);
      *wp++=v; //len<<3;
      *wp++=v>>8; //len>>5;
    }
    *wp++=0;
    *wp++=0;
    *wp++=0;
    *wp++=0;
    *wp++=0;
    *wp++=0;
    i+=8;
    i>>=6;
    do
    {
      volatile struct 
      {
	unsigned long a, b, c, d;
      };
      UREG i;
      UREG n;
      UREG k;
      unsigned long tt;
      unsigned long const __flash *up;
      unsigned char const __flash *cp;
      back_memmove((void*)(&a+4),state+4,16);
      up=&table->g;
      i=0;
      do
      {
#pragma diag_suppress=Pa082
	if (!(i&32))
	{
	  if (!(i&16))
	  {
	    tt=a+F1(b,c,d);
	  }
	  else
	  {
	    tt=a+F2(b,c,d);
	  }
	}
	else
	{
	  if (!(i&16))
	  {
	    tt=a+F3(b,c,d);
	  }
	  else
	  {
	    tt=a+F4(b,c,d);
	  }
	}
#pragma diag_default=Pa082
	tt+=*up++;
	cp=(unsigned char const __flash *)up;
	n=*cp++;
	k=*cp++;
	up=(unsigned long const __flash *)cp;
	tt+=p[k];
	do
	{
	  tt<<=1;
	  if (SREG_Bit0) tt|=1;
	}
	while(--n);
	a=tt+b;
	tt=d;
	back_memmove((void*)(&a+4),(void*)(&a+3),12);
	a=tt;
	i++;
      }
      while(!(i&64));
      sum(state,(unsigned long *)&a);
      p+=16;
    }
    while(--i);
    i=16;
    {
      char *out=out_;
      wp=(unsigned char*)state;
      do
      {
	UREG c;
	UREG c2;
	c2=c=*wp++;
	c>>=4;
	c+='0';
	if (c>'9') c+='a'-10-'0';
	*out++=c;
	c2&=0x0F;
	c2+='0';
	if (c2>'9') c2+='a'-10-'0';
	*out++=c2;
      }
      while(--i);
    }
  }
}
