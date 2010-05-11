
//----- defines -----
//FM (F-RAM)

#define FM_PORT             PORTD
#define FM_DDR              DDRD
#define FM_CS               PD4


#define FM_CS_ENABLE()	    cbi (FM_PORT, FM_CS)
#define FM_CS_DISABLE()	    sbi (FM_PORT, FM_CS)

#define FM_SIZE				(0x00020000)

#define FM_WREN 			0x06 //Set Write Enable Latch
#define FM_WRDI 			0x04 //Write Disable
#define FM_RDSR 			0x05 //Read Status Register
#define FM_WRSR 			0x01 //Write Status Register
#define FM_READ 			0x03 //Read Memory Data
#define FM_WRITE 			0x02 //Write Memory Data

#define MP3fifo_pSTART    0
#define MP3fifo_pEND      ((FM_SIZE-1)-(SPEC_MAX_PKT)*(ETH_MAX_PACKET_SIZE+3)-2)//0x01C8C2-1;

#define ETHfifo_pSTART    (MP3fifo_pEND+2)
#define ETHfifo_pEND      (FM_SIZE-1)
/*
enum MP3FIFO_STATES
{
  MP3FIFO_STATE_FREE=0,
  MP3FIFO_STATE_FULL    
};
*/
//----- global vars -----

extern UINT32 MP3fifo_pRD;//fifo_head;
extern UINT32 MP3fifo_pWR;//fifo_tail;
//extern const UINT32 MP3fifo_pSTART;
//extern const UINT32 MP3fifo_pEND;

extern UINT32 ETHfifo_pRD;//fifo_head;
extern UINT32 ETHfifo_pWR;//fifo_tail;
//extern const UINT32 ETHfifo_pSTART;
//extern const UINT32 ETHfifo_pEND;

extern void fifo_init(void);
//extern void MP3fifo_write(UINT8 *data, UINT32 len);
extern UINT32 MP3fifo_free(void);
extern UINT32 MP3fifo_len(void);

//extern void ETHfifo_read(UINT8 *data, UINT16 len);
//extern UINT16 ETHfifo_PKT_read(UINT8* eth_data, UINT8* TS);
//extern void ETHfifo_write(UINT8 *data, UINT16 len, UINT8 TS);
extern UINT32 ETHfifo_free(void);
extern UINT32 ETHfifo_len(void);
extern UINT8 ETHfifo_CNT;
extern volatile UREG MP3FIFO_STATE;
extern void fifo_test();