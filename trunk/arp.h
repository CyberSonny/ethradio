/* Name: arp.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */


#ifndef ARP_H
#define ARP_H
/*****************************************************************************
******************************************************************************/

/*****************************************************************************
Setup
******************************************************************************/

#define ARP_FRAME_LEN		(sizeof(ARP_FRAME))

#define ARP_TABLE_START		1	/* ARP cache first index. Zero is reserved for broadcast IP address  */
#define ARP_TABLE_SIZE		10	/* ARP cache size (number of entries) */
#define ARP_DEF_TTL			60	/* ARP cache entry refresh period in TTLs */
#define ARP_RESEND			60	/* ARP Request resend period (in seconds) */
#define ARP_MAXRETRY		5	/* Number of IP address resolving retires */

/* System constants, don't modify	*/
#define ARP_HARDWARE		SWAP16(0x0001)	/* Ethernet hardware type code */
#define ARP_ETHCODE			SWAP16(0x0800)	/* ARP over Ethernet */

#define ARP_REQUEST			SWAP16(0x0001)		/* ARP Request to resolve address	*/
#define ARP_RESPONSE		SWAP16(0x0002)		/* Response to resolve request			*/

#define	ARP_MNG_TIMEOUT		100		/* Interval between ARP cache menegment. (in tens ms) */

/*****************************************************************************
Internal structures
******************************************************************************/

/* ARP header */
typedef struct
{	
  UINT16 hw_type;	/* hardware type */
  UINT16 proto;	/* protocol type */
  UCHAR hw_len;	/* hardware address lenght */
  UCHAR protocol_len;	/* protocol type lenght */
  UINT16 opcode;	/* operation code */
  UINT8  src_mac[ETH_HWA_LEN];	/* source MAC */
  UINT32 src_ip;	/* source IP */
  UINT8  dst_mac[ETH_HWA_LEN];	/* dest MAC */
  UINT32 dst_ip;	/* dest IP */
} ARP_HEADER;

/* ethernet packet header for ARP protocol */
typedef struct
{	
  ETH_HEADER eth;
  ARP_HEADER arp;
} ARP_FRAME;

#endif





