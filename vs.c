
#include "stdafx.h"
#include "compiler.h"
#include "vs.h"



//UCHAR* pVS_BUFFER32=&VS_BUFFER32[0];

#pragma inline= forced
UCHAR VS_SPI_RD (void)
{
  cbr (SPSR, SPI2X); // set low freq. 1/4 SCLK (4MHz)
  SPDR = 0xAA;
  while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
  sbr (SPSR, SPI2X); // restore hi freq. 1/2 SCLK (8MHz)
  return SPDR;
}

#pragma inline= forced
void VS_SPI_WR (UCHAR DATA)
{
  cbr (SPSR, SPI2X); // set low freq. 1/4 SCLK (4MHz)
  SPDR = DATA; // set new mode
  while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished
  sbr (SPSR, SPI2X); // restore hi freq. 1/2 SCLK (8MHz)
}

//#pragma inline= forced
// Send byte to SDI of VS1053
void VS_DATA (UINT8 DATA)
{
   VS_DREQ_wait();    // be shure that FIFO is free
   VS_DS_clr();
   VS_SPI_WR(DATA); //write DATA
   VS_DS_set();
}

// Send 32 bytes to SDI of VS1053
void VS_DATA32 (UINT8* DATA)
{
  VS_DREQ_wait();   // be shure that FIFO is free
//  UINT8 i;
  VS_DS_clr();
//  for (i=0;i<=31; i++)
//  {
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);  VS_SPI_WR(*DATA++);
//  }  
   VS_DS_set();
}


#pragma inline= forced
// Write 16 bit value in SCI register of VS1053
void VS_SCI_WR (UCHAR REG, UINT16 DATA)
{
   VS_CS_clr();
   VS_SPI_WR(SCI_WR); //write mode
   VS_SPI_WR(REG); //write REG number
   VS_SPI_WR((UCHAR)(DATA>>8)); //write DATA High
   VS_SPI_WR((UCHAR)(DATA&0x00FF)); //write DATA Low
   __no_operation();
   VS_CS_set();
   VS_DREQ_wait();
}

#pragma inline= forced
// Read 16 bit value from SCI register of VS1053
UINT16 VS_SCI_RD (UCHAR REG)
{
   UINT16 DATA;
   VS_CS_clr();
   VS_SPI_WR(SCI_RD); //read mode
   VS_SPI_WR(REG); //write REG number
   DATA = VS_SPI_RD()<<8; // read most byte
   DATA |= VS_SPI_RD();   // read less byte
   __no_operation();
   VS_CS_set();
   VS_DREQ_wait();
   return DATA;
}

// VS1053 initialisation
void VS_INIT (void)
{  
  VS_DDR_CS_set(); // AVR CS_VS1053 pin as output
  VS_DDR_DS_set(); // AVR DS_VS1053 pin as output
  VS_DDR_RST_set(); // AVR RST_VS1053 as output (now VS1053 in reset state)
  VS_CS_set();  // set CS and DS high
  VS_DS_set(); 
  VS_SPI_DDR|=(1<<VS_MOSI)|(1<<VS_SCK);
  SPCR|=(1<<MSTR)|(1<<SPE); // Set MCU as Master and Enabled 
  VS_RST_clr(); // set RST VS1053 low
  _delay_ms(2);     // wait 2 ms in RESET state
  VS_RST_set();     // release RESET state
  _delay_ms(2);
    
  cbr (SPSR, SPI2X); // clear spi2x bit
  sbr (SPCR, SPR0); //  set SPI frequency as  1/16 of CPU_clk (1MHZ now)    
  
  VS_DREQ_wait();  // wait DREQ high   
 // VS_SCI_WR(MODE, (1<<SM_SDINEW); // select new comm. mode
  VS_SCI_WR (SC_CLOCKF, SC_MULT6|SC_ADD0); // set 3.5x with 1x add
  _delay_ms(2);  
  sbr (SPSR, SPI2X); // set spi2x bit
  cbr (SPCR, SPR0); // 1/2 of CPU_clk (8MHZ now!)
  VS_SCI_WR(SC_MODE, (1<<SM_SDINEW)|(1<<SM_RESET)); // activate SM_RESET bit
  _delay_ms(1);                           // wait at least 2 us
  VS_DREQ_wait(); //After DREQ is up, you may continue playback as usual.
 // VS_SCI_WR(MODE, (1<<SM_SDINEW)|(1<<SM_LAYER12)|(1<<SM_STREAM)); // activate SM_RESET bit
  VS_SCI_WR(SC_MODE, (1<<SM_SDINEW)|(1<<SM_LAYER12)); // activate SM_RESET bit
}

// VS1053 software reset
void VS_SOFTRST (void)
{
  VS_SCI_WR(SC_MODE, (1<<SM_SDINEW)|(1<<SM_RESET)); // activate SM_RESET bit
  _delay_ms(1);                           // wait at least 2 us
  VS_DREQ_wait(); //After DREQ is up, you may continue playback as usual.
  
}

// VS1053 sine test in Native mode
void VS_SINTEST (void)
{  
  while (1)
  {
  sbi (PORTD, PD5);
  //VS_INIT();   
  //VS_SOFTRST();
  //VS_SCI_WR(VOL, 0x0000); // set max volume
  VS_SCI_WR(SC_MODE, (1<<SM_TESTS)|(1<<SM_SDINEW)|(1<<SM_RESET)); // SCT: 0x02 0x00 0x8020
  VS_DATA(0x53);
  VS_DATA(0xEF);
  VS_DATA(0x6E);
  VS_DATA(0x44);
  VS_DATA(0x00);
  VS_DATA(0x00);
  VS_DATA(0x00);
  VS_DATA(0x00);
  _delay_ms(1000);
  cbi (PORTD, PD5);
  VS_DATA(0x45);
  VS_DATA(0x78);
  VS_DATA(0x69);
  VS_DATA(0x74);
  VS_DATA(0x00);
  VS_DATA(0x00);
  VS_DATA(0x00);
  VS_DATA(0x00);
   _delay_ms(1000);  
  }
}

void VS_NEW_SINTEST (void)
{
  #define sample_rate (44100|1)
  #define LFreq 440
  #define RFreq 880
  VS_SCI_WR(SC_VOL, 0x0000);
  VS_SCI_WR(SC_AUDATA, sample_rate);
  VS_SCI_WR(SC_AICTRL0,LFreq*65536/sample_rate);
  VS_SCI_WR(SC_AICTRL1,RFreq*65536/sample_rate); 
  VS_SCI_WR(SC_AIADDR, 0x4020);
  
}

void VS_SINSWEEP (void)
{
  VS_SCI_WR(SC_VOL, 0x0000);
  VS_SCI_WR(SC_AUDATA, 44101); 
  VS_SCI_WR(SC_AIADDR, 0x4022);
  
}

void VS_VOL (void)
{
    #define LVol 220
    #define RVol 220
    #define LRVol  (((256-LVol)<<8)|(256-RVol))
    VS_SCI_WR(SC_VOL, LRVol);
  //  if (VS_SCI_RD(SC_VOL)!=LRVol) while (1);    
}

UINT16 VS_endFillByte (void)
{
  VS_SCI_WR(SC_WRAMADDR, endFillByte);
  return VS_SCI_RD(SC_WRAM);
}

/*
      VS_BUFFER32[VS_BUFFER_pos++]=c; // Пока буфер не переполнился - пишем тело в буфер
      if (VS_BUFFER_pos>=32)         
      {
          // Иначе сваливаем в VS1053 и проигрываем,
          UREG *DATA=&VS_BUFFER32[0];
          VS_DREQ_wait();    // be shure that FIFO is free
          cbr (SPSR, SPI2X); // set low freq. 1/4 SCLK (4MHz)
          VS_DS_clr();
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA++; while (!(SPSR & (1<<SPIF)));
          SPDR = *DATA++; while (!(SPSR & (1<<SPIF))); SPDR = *DATA;   while (!(SPSR & (1<<SPIF)));                              
          VS_DS_set();
          sbr (SPSR, SPI2X); // restore hi freq. 1/2 SCLK (8MHz)
        VS_BUFFER_pos=0;              // Возвращаем указатель буфера на начало
      }      
*/