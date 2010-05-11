/* Name: icmp.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#ifndef ICMP_H
#define ICMP_H
#include "ip.h"

#define ICMP_HEADER_LEN		(sizeof(ICMP_HEADER))
#define ICMP_FRAME_LEN		(sizeof(ICMP_FRAME))

#define ICMP_ECHO_REQUEST			8
#define ICMP_ECHO_RESPONSE			0

typedef struct
{	
  UINT8 type;	/* packet type */
  UINT8 code;	/* code */
  UINT16 checksum;	/* packet checksum */
  UINT16 id;	/* packet ID */	
  UINT16 seqno;	/* sequence number */
} ICMP_HEADER;

typedef struct
{
  ICMP_HEADER icmp;		/* ICMP header */
  UINT8 data[ETH_MAX_PACKET_SIZE - ETH_HEADER_LEN - IP_HEADER_LEN - ICMP_HEADER_LEN];
}
ICMP_PKT;

typedef struct
{	
  ETH_HEADER eth;	/* Ethernet header */
  IP_HEADER ip;	/* IP header */
  ICMP_HEADER icmp;		/* ICMP header */
  UINT8 data[ETH_MAX_PACKET_SIZE - ETH_HEADER_LEN - IP_HEADER_LEN - ICMP_HEADER_LEN];
} ICMP_FRAME;

#endif

