//*****************************************************************************
//
// File Name	: 'enc28j60.c'
// Title		: Microchip ENC28J60 Ethernet Interface Driver
// Author		: Pascal Stang (c)2005
// Created		: 9/22/2005
// Revised		: 9/22/2005
// Version		: 0.1
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
// Description	: This driver provides initialization and transmit/receive
// functions for the Microchip ENC28J60 10Mb Ethernet Controller and PHY.
// This chip is novel in that it is a full MAC+PHY interface all in a 28-pin
// chip, using an SPI interface to the host processor.
//
// patched by Alexander Yerezeyev, 2010
// wapbox@bk.ru
//*****************************************************************************


#include "nike_e.h"
#include "compiler.h"
#include "enc28j60.h"
#include "io.h"
extern UINT8 MAC0[];

// include configuration
//#include "ax88796conf.h"
u8 Enc28j60Bank;
u16 NextPacketPtr;

#pragma inline = forced
u8 enc28j60ReadOp(u8 op, u8 address)
{
	u8 data;
        
    	//sbi(SPSR, SPI2X);
	// assert CS
	ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);
	
	// issue read command
	SPDR = op | (address & ADDR_MASK);
	while(!(SPSR & (1<<SPIF)));
	// read data
	SPDR = 0x00;
	while(!(SPSR & (1<<SPIF)));
	// do dummy read if needed
	if(address & 0x80)
	{
		SPDR = 0x00;
		while(!(SPSR & (1<<SPIF)));
	}
	data = SPDR;
	
	// release CS
	ENC28J60_CONTROL_PORT |= (1<<ENC28J60_CONTROL_CS);
        //cbi(SPSR, SPI2X);
	return data;
}
#pragma inline = forced
void enc28j60WriteOp(u8 op, u8 address, u8 data)
{
      //  sbi(SPSR, SPI2X);
	// assert CS
	ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);

	// issue write command
	SPDR = op | (address & ADDR_MASK);
	while(!(SPSR & (1<<SPIF)));
	// write data
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));

	// release CS
	ENC28J60_CONTROL_PORT |= (1<<ENC28J60_CONTROL_CS);
       // cbi(SPSR, SPI2X);
}
#pragma inline = forced
void enc28j60ReadBuffer(u16 len, u8* data)
{
         //if ((len==0)||(len>=255)) return;
	// assert CS
       // sbi(SPSR, SPI2X);
	ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);
	
	// issue read command
	SPDR = ENC28J60_READ_BUF_MEM;
	while(!(SPSR & (1<<SPIF)));
	while(len--)
	{
		// read data
		SPDR = 0x00;
		while(!(SPSR & (1<<SPIF)));
		*data++ = SPDR;
	}	
	// release CS
	ENC28J60_CONTROL_PORT |= (1<<ENC28J60_CONTROL_CS);
      //  cbi(SPSR, SPI2X);
}
#pragma inline = forced
void enc28j60WriteBuffer(u16 len, u8* data)
{
       // if ((len==0)||(len>=255)) return;
	// assert CS
       // sbi(SPSR, SPI2X);
	ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);
	
	// issue write command
	SPDR = ENC28J60_WRITE_BUF_MEM;
	while(!(SPSR & (1<<SPIF)));
	while(len--)
	{
		// write data
		SPDR = *data++;
		while(!(SPSR & (1<<SPIF)));
	}	
	// release CS
	ENC28J60_CONTROL_PORT |= (1<<ENC28J60_CONTROL_CS);
      //  cbi(SPSR, SPI2X);
}
#pragma inline = forced
void enc28j60SetBank(u8 address)
{
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank)
	{
		// set the bank
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
	}
}
#pragma inline = forced
u8 enc28j60Read(u8 address)
{
	// set the bank
	enc28j60SetBank(address);
	// do the read
	return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}
#pragma inline = forced
void enc28j60Write(u8 address, u8 data)
{
	// set the bank
	enc28j60SetBank(address);
	// do the write
	enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}
#pragma inline = forced
u16 enc28j60PhyRead(u8 address)
{
	u16 data;

	// Set the right address and start the register read operation
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MICMD, MICMD_MIIRD);
//        _delay_us(11);
	// wait until the PHY read completes
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);
  
	// quit reading
	enc28j60Write(MICMD, 0x00);
	
	// get data value
	data  = enc28j60Read(MIRDL);
	data |= enc28j60Read(MIRDH);
	// return the data
	return data;
}
#pragma inline = forced
void enc28j60PhyWrite(u8 address, u16 data)
{
	// set the PHY register address
	enc28j60Write(MIREGADR, address);
	
	// write the PHY data
	enc28j60Write(MIWRL, data);	
	enc28j60Write(MIWRH, data>>8);
//        _delay_us(11);
	// wait until the PHY write completes
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);
}

void enc28j60Init(void)
{
	u8 i,j;
	// initialize I/O
	sbi(ENC28J60_CONTROL_DDR, ENC28J60_CONTROL_CS);
	sbi(ENC28J60_CONTROL_PORT, ENC28J60_CONTROL_CS);

	// setup SPI I/O pins	
/*	
        sbi(ENC28J60_CONTROL_DDR, ENC28J60_SCK_PIN);	// set SCK as output
	cbi(ENC28J60_CONTROL_DDR, ENC28J60_MISO_PIN);	// set MISO as input
	sbi(ENC28J60_CONTROL_DDR, ENC28J60_MOSI_PIN);	// set MOSI as output
	sbi(ENC28J60_CONTROL_DDR, ENC28J60_SS_PIN);	// SS must be output for Master mode to work
        sbi(ENC28J60_CONTROL_DDR, ENC28J60_RST_PIN);    // // set RST as output
	// initialize SPI interface
	// master mode
        SPCR|=(1<<MSTR)|(1<<SPE); 
        sbi (SPSR, SPI2X);
        // perform hardware reset
        //
 */      
        sbi(ENC28J60_CONTROL_DDR, ENC28J60_RST_PIN);    // // set RST as output
        ENC28J60_RESET();
	// perform system reset
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
        _delay_ms(100);
        // The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
        //while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));
	// initialize receive buffer
	// 16-bit transfers, must write low byte first
	// set receive buffer start address
	NextPacketPtr = RXSTART_INIT;
	enc28j60Write(ERXSTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXSTH, RXSTART_INIT>>8);
	// set receive pointer address
	enc28j60Write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXRDPTH, RXSTART_INIT>>8);
	// set receive buffer end
	enc28j60Write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60Write(ERXNDH, RXSTOP_INIT>>8);      
	// set transmit buffer start
	enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60Write(ETXSTH, TXSTART_INIT>>8);
      // do bank 1 stuff, packet filter:
      // For broadcast packets we allow only ARP packtets
      // All other packets should be unicast only for our mac (MAADR)
       //
       // The pattern to match on is therefore
       // Type     ETH.DST
       // ARP      BROADCAST
       // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
        // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
          enc28j60Write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
          enc28j60Write(EPMM0, 0x3f);
          enc28j60Write(EPMM1, 0x30);
          enc28j60Write(EPMCSL, 0xf9);
          enc28j60Write(EPMCSH, 0xf7);
	// do bank 2 stuff
	// enable MAC receive
	enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// bring MAC out of reset
	enc28j60Write(MACON2, 0x00);
	// enable automatic padding and CRC operations
//	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	enc28j60Write(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
//When the medium is occupied, the MAC will wait indefinitely for it to become free when attempting to transmit (use this setting for IEEE 802.3. compliance)
        enc28j60Write(MACON4, MACON4_DEFER);
	// set inter-frame gap (non-back-to-back)
	enc28j60Write(MAIPGL, 0x12);
	enc28j60Write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	enc28j60Write(MABBIPG, 0x12);
	// Set the maximum packet size which the controller will accept
	enc28j60Write(MAMXFLL, MAX_FRAMELEN&0xFF);	
	enc28j60Write(MAMXFLH, MAX_FRAMELEN>>8);

	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
//	enc28j60Write(MAADR5, UIP_ETHADDR0);
	enc28j60Write(MAADR5, MAC0[0]);
//	enc28j60Write(MAADR4, UIP_ETHADDR1);
	enc28j60Write(MAADR4, MAC0[1]);
//	enc28j60Write(MAADR3, UIP_ETHADDR2);
	enc28j60Write(MAADR3, MAC0[2]);        
//	enc28j60Write(MAADR2, UIP_ETHADDR3);
	enc28j60Write(MAADR2, MAC0[3]);        
//	enc28j60Write(MAADR1, UIP_ETHADDR4);
	enc28j60Write(MAADR1, MAC0[4]);        
//	enc28j60Write(MAADR0, UIP_ETHADDR5);
	enc28j60Write(MAADR0, MAC0[5]);        
        

        
	// no loopback of transmitted frames
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);

	// switch to bank 0
	enc28j60SetBank(ECON1);
	// enable interrutps
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_RXERIE);
	// enable packet reception
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
/*
	// setup duplex ----------------------
	// Disable receive logic and abort any packets currently being transmitted                
	enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS|ECON1_RXEN);
        //switch to bank 2
        
        enc28j60SetBank(MACON3);
	_delay_ms(10);
	{
		u16 temp;
		// Set the PHY to the proper duplex mode
		temp = enc28j60PhyRead(PHCON1);
		temp &= ~PHCON1_PDPXMD;
		enc28j60PhyWrite(PHCON1, temp);
           	// Set the MAC to the proper duplex mode
                enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_FULDPX);
                //Allow the MAC to transmit pause control frames (needed for flow control in full duplex)
               // enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON1, MACON1_TXPAUS);
	}
	// Set the back-to-back inter-packet gap time to IEEE specified 
	// requirements.  The meaning of the MABBIPG value changes with the duplex
	// state, so it must be updated in this function.
	// In full duplex, 0x15 represents 9.6us; 0x12 is 9.6us in half duplex
	enc28j60Write(MABBIPG, 0x15);		
       	// switch to bank 0
        enc28j60SetBank(ECON1);
	// Reenable receive logic
        
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
*/      
         // configure LEDs
        enc28j60PhyWrite(PHLCON, 0x0122);	
}


void enc28j60BeginPacketSend(unsigned int packetLength)
{
//
}

unsigned char enc28j60PacketSend(unsigned char * packet, unsigned int len)
{
       /* wait until previous packet was sent */
        while(enc28j60Read(ECON1) & ECON1_TXRTS);
        if ((!packet)||(!len)) return 1;
        // make a TX reset (errata workaround)
       /*
       enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
       enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
       enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF);     
       */
        if( (enc28j60Read(EIR) & EIR_TXERIF) )
        {
          // if there was an error during transmission
          // make a TX reset
              enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
              enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
              enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF);
              return 1;
            }       
     
         /* set start of packet */
        enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60Write(ETXSTH, TXSTART_INIT>>8);
	// Set the write pointer to start of transmit buffer area        
	enc28j60Write(EWRPTL, TXSTART_INIT&0xff);
	enc28j60Write(EWRPTH, TXSTART_INIT>>8);
	// Set the TXND pointer to correspond to the packet size given
	enc28j60Write(ETXNDL, (TXSTART_INIT+len));
	enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);
      	// write per-packet control byte
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);        
	// copy the packet into the transmit buffer
	enc28j60WriteBuffer(len, packet);	
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
        return 0;
}
	
  // Gets a packet from the network receive buffer, if one is available.
  // The packet will by headed by an ethernet header.
  //      maxlen  The maximum acceptable length of a retrieved packet.
  //      packet  Pointer where packet data should be stored.
  // Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
u16 enc28j60PacketReceive(u16 maxlen, u8* packet)
 {
   u16 rxstat;
   u16 len;
//Вынесли в главный цикл   
/*   if( enc28j60Read(EPKTCNT)==0 )
   {
        return(0);
   }
*/
   if( (enc28j60Read(EIR) & EIR_RXERIF) ) // RX buffer overflow is detected!
   {
    #ifdef CONSOLE_DEBUG
     _print_fstr("EncOVF!\r\n");
    #endif
    sbi (PORTD, PD3);
    enc28j60SetBank(EFLOCON);
     enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EFLOCON, 1);// enable flow control
     enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_RXERIF);     
   }
   else
   {
     enc28j60SetBank(EFLOCON);
     enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, EFLOCON, 1);// disable flow control
     cbi (PORTD, PD3);
   };   
   
  // Set the read pointer to the start of the received packet
  enc28j60Write(ERDPTL, (NextPacketPtr));
  enc28j60Write(ERDPTH, (NextPacketPtr)>>8);
  
  // read the "next packet pointer"
  NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
  // read the packet length (see datasheet page 43)
  len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
  len-=4; //remove the CRC count
   // read the receive status (see datasheet page 43)
  rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  rxstat |= ((u16)enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0))<<8;
  // limit retrieve length
   if (len>maxlen){
       len=maxlen;
      }
   // check CRC and symbol errors (see datasheet page 44, table 7-3):
   // The ERXFCON.CRCEN is set by default. Normally we should not
   // need to check this.
   if ((rxstat & 0x80)==0)
   {
      // invalid reception
      len=0;
    }
   else
   {
        // copy the packet from the receive buffer
         enc28j60ReadBuffer(len, packet);
   }
     
// Move the RX read pointer to the start of the next received packet
// This frees the memory we just read out
// Errata workaround #13. Make sure ERXRDPT is odd
//
  {
        unsigned int rstart,rend;
        rstart = (enc28j60Read(ERXSTH)<<8);
        rstart |= enc28j60Read(ERXSTL);        
        rend = (enc28j60Read(ERXNDH)<<8);
        rend |= enc28j60Read(ERXNDL);
        if (((NextPacketPtr - 1) < rstart) || ((NextPacketPtr - 1) > rend))
        {
            enc28j60Write(ERXRDPTL, (rend));
            enc28j60Write(ERXRDPTH, (rend)>>8);
        }
        else
        {
            enc28j60Write(ERXRDPTL, (NextPacketPtr-1));
            enc28j60Write(ERXRDPTH, (NextPacketPtr-1)>>8);
        }
    }
      
  // decrement the packet counter indicate we are done with this packet
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
  return(len);
 }

void enc28j60ReceiveOverflowRecover(void)
{
	// receive buffer overflow handling procedure

	// recovery completed
}

void enc28j60RegDump(void)
{
//	unsigned char macaddr[6];
//	result = ax88796Read(TR);
	
//	rprintf("Media State: ");
//	if(!(result & AUTOD))
//		rprintf("Autonegotiation\r\n");
//	else if(result & RST_B)
//		rprintf("PHY in Reset   \r\n");
//	else if(!(result & RST_10B))
//		rprintf("10BASE-T       \r\n");
//	else if(!(result & RST_TXB))
//		rprintf("100BASE-T      \r\n");
/*				
	rprintf("RevID: 0x%x\r\n", enc28j60Read(EREVID));

	rprintfProgStrM("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\r\n");
	rprintfProgStrM("         ");
	rprintfu8(enc28j60Read(ECON1));
	rprintfProgStrM("    ");
	rprintfu8(enc28j60Read(ECON2));
	rprintfProgStrM("    ");
	rprintfu8(enc28j60Read(ESTAT));
	rprintfProgStrM("    ");
	rprintfu8(enc28j60Read(EIR));
	rprintfProgStrM("   ");
	rprintfu8(enc28j60Read(EIE));
	rprintfCRLF();

	rprintfProgStrM("MAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\r\n");
	rprintfProgStrM("        0x");
	rprintfu8(enc28j60Read(MACON1));
	rprintfProgStrM("    0x");
	rprintfu8(enc28j60Read(MACON2));
	rprintfProgStrM("    0x");
	rprintfu8(enc28j60Read(MACON3));
	rprintfProgStrM("    0x");
	rprintfu8(enc28j60Read(MACON4));
	rprintfProgStrM("   ");
	rprintfu8(enc28j60Read(MAADR5));
	rprintfu8(enc28j60Read(MAADR4));
	rprintfu8(enc28j60Read(MAADR3));
	rprintfu8(enc28j60Read(MAADR2));
	rprintfu8(enc28j60Read(MAADR1));
	rprintfu8(enc28j60Read(MAADR0));
	rprintfCRLF();

	rprintfProgStrM("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\r\n");
	rprintfProgStrM("       0x");
	rprintfu8(enc28j60Read(ERXSTH));
	rprintfu8(enc28j60Read(ERXSTL));
	rprintfProgStrM(" 0x");
	rprintfu8(enc28j60Read(ERXNDH));
	rprintfu8(enc28j60Read(ERXNDL));
	rprintfProgStrM("  0x");
	rprintfu8(enc28j60Read(ERXWRPTH));
	rprintfu8(enc28j60Read(ERXWRPTL));
	rprintfProgStrM("  0x");
	rprintfu8(enc28j60Read(ERXRDPTH));
	rprintfu8(enc28j60Read(ERXRDPTL));
	rprintfProgStrM("   0x");
	rprintfu8(enc28j60Read(ERXFCON));
	rprintfProgStrM("    0x");
	rprintfu8(enc28j60Read(EPKTCNT));
	rprintfProgStrM("  0x");
	rprintfu8(enc28j60Read(MAMXFLH));
	rprintfu8(enc28j60Read(MAMXFLL));
	rprintfCRLF();

	rprintfProgStrM("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\r\n");
	rprintfProgStrM("       0x");
	rprintfu8(enc28j60Read(ETXSTH));
	rprintfu8(enc28j60Read(ETXSTL));
	rprintfProgStrM(" 0x");
	rprintfu8(enc28j60Read(ETXNDH));
	rprintfu8(enc28j60Read(ETXNDL));
	rprintfProgStrM("   0x");
	rprintfu8(enc28j60Read(MACLCON1));
	rprintfProgStrM("     0x");
	rprintfu8(enc28j60Read(MACLCON2));
	rprintfProgStrM("     0x");
	rprintfu8(enc28j60Read(MAPHSUP));
	rprintfCRLF();
*/
}



