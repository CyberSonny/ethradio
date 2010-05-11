/* Name: tcp.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */


#ifndef TCP_H
#define TCP_H
/*****************************************************************************
******************************************************************************/

#include "ip.h"

/*****************************************************************************
defs
******************************************************************************/
/* sizes */
#define	TCP_HEADER_LEN			(sizeof(TCP_HEADER))
#define TCP_DATA_OFFSET			(IP_DATA_OFFSET + TCP_HEADER_LEN)
#define TCP_MAX_DATA_LEN		(ETH_MAX_PACKET_SIZE - TCP_DATA_OFFSET)
#define	TCP_MAX_OPT_LEN			40

/*  */
#define	TCP_DEF_RETRIES			7	/* Number of attempted TCP retransmissions before giving up */		
#define TCP_CON_ATTEMPTS		7	/* Number of connection-establishment attempts */
#define TCP_TOS_NORMAL			0	/* Defines normal type of service for TCP socket */

/* Timeouts */
#define TCP_DEF_RETRY_TIMEOUT	400	/* Default data-retransmission period (in tens ms) */
#define TCP_INIT_RETRY_TIMEOUT	100	/* Initial retransmission period (in tens ms) */
#define TCP_SYN_RETRY_TIMEOUT	200	/* Retranmission period for SYN packet */
#define TCP_DEF_TIMEOUT			3000	/* Default idle timeout (in tens ms)  */

/* Flags */
#define	TCP_FLAG_ACK			0x10
#define TCP_FLAG_PUSH			0x08
#define TCP_FLAG_RESET			0x04
#define TCP_FLAG_SYN			0x02
#define TCP_FLAG_FIN			0x01

/* Internal flags			*/
#define TCP_INTFLAGS_CLOSEPENDING	0x01

/* Socket types */
#define	TCP_TYPE_NONE			(0x00)	/* TCP socket is nor a client nor a server */
#define	TCP_TYPE_SERVER			(0x01)	/* TCP socket represents a server application */
#define	TCP_TYPE_CLIENT			(0x02)	/* TCP socket represents a client application */
#define	TCP_TYPE_CLIENT_SERVER	(0x03)	/* TCP socket can act as client or as server */
#define TCP_TYPE_FLAG_DELAYED_ACK (0x80)
/* Socket states. For more detailed descriptions see RFC793 */
//#define	TCP_STATE_FREE			1	/* Entry is free and unused */
//#define	TCP_STATE_RESERVED		2	/* Entry is reserved for use */
//#define	TCP_STATE_CLOSED		3	/* Entry allocated, socket still closed	*/
//#define TCP_STATE_LISTENING		4	/* Socket in listening state, waiting for incoming connections */
//#define TCP_STATE_SYN_RECEIVED	5	/* SYN packet received (either first SYN packet or response to SYN that we have previously sent) */
//#define	TCP_STATE_SYN_SENT		6	/* SYN packet sent as an attempt to establish a connection */
//#define TCP_STATE_FINW1			7	/* User issued TCP_close request issued so FIN packet was sent */
//#define	TCP_STATE_FINW2			8	/* Received ACK of our FIN, now waiting for other side to send FIN */
//#define TCP_STATE_CLOSING		9	/* Received FIN independently of our FIN */
//#define	TCP_STATE_LAST_ACK		10	/* Waiting for last ACK packet as a response to our FIN */
//#define TCP_STATE_TIMED_WAIT	11	/* Waiting for 2MSL to prevent erroneous connection duplication */
//#define	TCP_STATE_CONNECTED		12	/* Connection established and data flowing freely to both sides :-) */
//#define TCP_STATE_CHEAT1		13

#define SPEC_MAX_PKT  10
#define SPEC_MAX_WIN (SPEC_MAX_PKT*TCP_MAX_DATA_LEN)

enum TCP_STATES
{
  TCP_STATE_CLOSED=0,
  TCP_STATE_LISTENING,
  TCP_STATE_SYN_RECEIVED,
  TCP_STATE_CONNECTING,
  TCP_STATE_SYN_SENT,
  TCP_STATE_FINW1,
  TCP_STATE_CONNECTED,
  TCP_STATE_CHEAT1,
  TCP_STATE_QUE_CLIENT
};

/* Callback events			*/
//#define TCP_EVENT_CONREQ		0x01	/* Connection request event */
//#define TCP_EVENT_CONNECTED		0x02	/* Connection established event */
//#define TCP_EVENT_CLOSE			0x03	/* Connection closed event */
//#define TCP_EVENT_ABORT			0x04	/* Connection aborted event */
//#define TCP_EVENT_ACK			0x05	/* Data acknowledged event */
//#define TCP_EVENT_REGENERATE		0x06	/* Regenerate data event */
//#define TCP_EVENT_DATA			0x07	/* Data arrival event */
//#define TCP_EVENT_SEND			0x08	/* Send next data event */
enum TCP_EVENTS
{
  TCP_EVENT_CONREQ=1,
  TCP_EVENT_CONNECTED,
  TCP_EVENT_CLOSE,
  TCP_EVENT_ABORT,
  TCP_EVENT_ACK,
  TCP_EVENT_REGENERATE,
  TCP_EVENT_DATA,
  TCP_EVENT_SEND,
  TCP_EVENT_ASYNC_REQ,
  TCP_EVENT_QUE_ALLOC, //Получить адрес TCP_QUE_CLIENT для сохранения в конец очереди (происходит alloc)
  TCP_EVENT_QUE_GET, //Получить адрес TCP_QUE_CLIENT для извлечения из начала очереди
  TCP_EVENT_QUE_REMOVE //Удалить элемент в начале очереди
};

/*****************************************************************************
TCP types
******************************************************************************/

/* TCP header */
typedef struct
{
  UINT16 src_port;					/* Source port					*/
  UINT16 dest_port;					/* Destination port				*/
  UINT32 seqno;						/* Sequence number				*/
  UINT32 ackno;						/* Acknowledgement number		*/
  UINT8	hlen;						/* Header length an 32 bits oktets	*/
  UINT8	flags;						/* Flags	*/
  UINT16 window;						/* Size of window				*/
  UINT16 checksum;					/* TCP packet checksum			*/
  UINT16 urgent;						/* Urgent pointer				*/
} TCP_HEADER;

/* TCP frame */
typedef struct
{
  ETH_HEADER eth;	/* ethernet header */
  IP_HEADER ip;	/* IP header */
  TCP_HEADER tcp;	/* TCP header */
  UINT8 data[TCP_MAX_DATA_LEN];	/* data block */
} TCP_FRAME;

typedef struct
{
  TCP_HEADER tcp;	/* TCP header */
  UINT8 data[TCP_MAX_DATA_LEN];	/* data block */
} TCP_PKT;

struct TCP_SOCK;

typedef __x UREG SOCK_HOOK(UREG state,UREG len, UINT8 *data, struct TCP_SOCK*);

/* TCP socket */
typedef struct TCP_SOCK
{
  struct TCP_SOCK *next;
  UINT8 type;
  //UREG (*hook)(UREG state,UREG len, UINT8 *data, struct TCP_SOCK*);
  SOCK_HOOK *hook;
  UINT8	dst_mac[ETH_HWA_LEN]; //MAC
  UINT32 rip; //Remote IP
  struct //Читерская поебень. Эта структура совпадает с началом TCP_HEADER
  {
    UINT16 lport; //Local port
    UINT16 rport; //Remote port
    UINT32 SEQNO; //Мой SEQ для передачи данных
    UINT32 ACKNO; //ACK, который я буду посылать (и заодно ожидаемый SEQ от remote)
    UINT8 state; //State of stack
    UINT8 flags; //Флаги
    UINT16 win;
  };
  UINT16 rwin; //Размер окна на другой стороне
  UINT16 rmss; //MSS на другой стороне
  UINT16 sended_len; //Посланный размер непотвержденных данных
  UINT16 send_disp; //Смещение текущего посылаемого пакета относительно SEQNO (меньше или равно sended_len)
  UINT8 timer;
  UINT8 timer_init;
  UINT8 txreq; //Флаг запроса передачи
  UINT8 async_req;
} TCP_SOCK;

enum TCP_TXREQ
{
  TCP_TXREQ_NONE=0,
  TCP_TXREQ_DUPACK,
  TCP_TXREQ_STOP,
  TCP_TXREQ_SEND,
  TCP_TXREQ_DACK1,
  TCP_TXREQ_DACK2
};


//Элемент очереди ожидающих запросов на подключение
typedef struct TCP_QUE_CLIENT
{
  UINT8 dst_mac[ETH_HWA_LEN]; //МАС клиента
  UINT16 mss; //MSS клиента
  UINT16 rport; //Порт клиента
  UINT32 rip; //IP клиента
  UINT32 seqno; //SEQ с пришедшего пакета
}TCP_QUE_CLIENT;


//Начальное значение таймера. Обязательно степень двойки!
#define TCP_TIMER_INITV (1<<2)

extern TCP_SOCK *QUE_sock;
/*****************************************************************************
TCP unit API
******************************************************************************/

__z void AddTCPsocket(TCP_SOCK *s);
__x UREG RemoveTCPsocket(TCP_SOCK *s);
__x void TCPconnect(TCP_SOCK *s, UINT32 ip, UINT16 port);

#define CloseTCPsocket(SOCK) {(SOCK)->state=TCP_STATE_FINW1;}

/*****************************************************************************
******************************************************************************/
#endif	/* #ifndef TCP_H */



