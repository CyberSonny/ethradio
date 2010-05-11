/* Name: ip.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#ifndef IP_H
#define IP_H

#include "ethernet.h"

#define IP_ICMP			1		/* ICMP over IP */
#define IP_UDP			17		/* UDP over IP */
#define IP_TCP  		6		/* TCP over IP */

#define IP_HEADER_LEN		sizeof(IP_HEADER)	/* IP Header Length in bytes */
#define IP_DATA_OFFSET		(ETH_HEADER_LEN + IP_HEADER_LEN)	/* IP packet data offset */
#define IP_MAX_DATA_LEN		(ETH_MAX_PACKET_SIZE - IP_DATA_OFFSET)

#define IP_DEF_VIHL		0x45
#define IP_DEF_TTL		100
#define IP_DEF_TOS		0
#define IP_MAX_OPTLEN		40				/* Max IP Header option field length */

#define IP_DONT_FRAGMENT 	0x4000			/* Don't Fragment Flag			*/
#define IP_FRAGOFF		0x1FFF			/* Fragment offset mask			*/
#define IP_MOREFRAGS	 	0x2000			/* More fragments bit			*/

#define	IP_GOOD_CS		0

/* IP Option control commands	*/
#define IP_OPT_COPY		0x80			/* Copy on fragment mask		*/
#define IP_OPT_NOP		0x01			/* No operation					*/
#define IP_OPT_EOOP		0x00			/* End of options				*/

/* Reserved addresses		*/
#define	IP_BROADCAST_ADDRESS	0xFFFFFFFF	/* 255.255.255.255	*/

/*****************************************************************************
IP types
******************************************************************************/

/* IP datagram header fields */
typedef struct
{	
  UINT8	vihl;					/* Version & Header Length field	*/
  UINT8	tos;					/* Type Of Service					*/
  union
  {
    UINT16  tlen;					/* Total Length						*/
    struct
    {
      UINT8 tlen_hi;
      UINT8 tlen_low;
    };
  };
  UINT16 id;						/* IP Identification number			*/
  UINT16 frags;					/* Flags & Fragment offsett			*/
  UINT8	ttl;					/* Time to live						*/
  UINT8	proto;					/* Protocol over IP					*/
  UINT16 checksum;				/* Header Checksum					*/
  UINT32 src_ip;					/* Source IP address				*/
  UINT32 dest_ip;				/* Destination IP address			*/
} IP_HEADER;

/* IP frame */
typedef struct
{	
  ETH_HEADER eth;	/* Ethernet header */
  IP_HEADER ip;	/* IP header */
  UINT8 data[IP_MAX_DATA_LEN];
} IP_FRAME;

/* IP pseudo header
this is use for calculate checksum for TCP & UDP datagrams */
typedef struct
{
  
  UINT32 src_ip;	/* sorce IP */
  UINT32 dest_ip;	/* destination IP */
  UINT8	zero;		/* mast be zero */
  UINT8	proto;		/* mast be IP_UDP value */
  UINT16 tlen;		/* total datagram lenght (include protocol header size) */
} IP_PSEUDOHEADER;

#endif
