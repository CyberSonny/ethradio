/* Name: ethernet.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#ifndef ETHERNET_H
#define ETHERNET_H

#define ETH_GOOD_CRC32 (~(0xDEBB20E3L))

/* packet size limits */
#define ETH_MIN_PACKET_SIZE	(60)
/* MTU=ETH_MAX_PACKET_SIZE-14 */
#define ETH_MAX_PACKET_SIZE	(1398)

/* data offset */
#define ETH_DATA_OFFSET		(ETH_HEADER_LEN)	/* IP packet data offset */

/* directions for load params for local host */
#define ETH_HOST_PARAMS_DEFAULT	0

/* max address lenght */
#define ETH_HWA_LEN			6	/* Ethernet MAC address length */
#define ETH_PRA_LEN			4	/* Protocol address lenght (4 for IPv4)*/

/* ethernet packet header length */
#define	ETH_HEADER_LEN		(sizeof(ETH_HEADER)) /* - Ethernet_Header_t->size */

/* protocol types */
#define	ETH_PROTOCOL_IP		SWAP16(0x0800)			/* IP over Ethernet	*/
#define ETH_PROTOCOL_ARP	SWAP16(0x0806)			/* ARP over Ethernet */

/*****************************************************************************
structures
******************************************************************************/

/* ethernet packet header */
typedef struct
{
  	UINT8	dst_mac[ETH_HWA_LEN];	/* destination hardware address as read from the received ethernet packet */
	UINT8	src_mac[ETH_HWA_LEN];	/* source hardware address as read from the received ethernet packet */
	UINT16	type;			/* protocol field of the Ethernet header. For now we work with:
											ETH_PROTOCOL_IP	- 0x0800
											ETH_PROTOCOL_ARP - 0x0806 */
} ETH_HEADER;

/* ethernet packet type */
typedef struct
{	
  ETH_HEADER hdr;
//  UINT8 data[ETH_MAX_PACKET_SIZE - ETH_HEADER_LEN + 4];
   UINT8 data[ETH_MAX_PACKET_SIZE - ETH_HEADER_LEN]; // enc28j60 port
} ETH_FRAME;

#endif	/* #ifndef ETHERNET_H */
