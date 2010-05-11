#define VS_DREQ  PIND7
#define VS_DREQ_PIN  PIND
#define VS_CS PB4
#define VS_CS_PORT PORTB
#define VS_CS_DDRPORT DDRB
#define VS_SPI_DDR  DDRB
#define VS_MOSI PB5
#define VS_MISO PB6
#define VS_SCK PB7
#define VS_DS PB3
#define VS_DS_PORT PORTB
#define VS_DS_DDRPORT DDRB
#define VS_RST PB0
#define VS_RST_PORT PORTB
#define VS_RST_DDRPORT DDRB

#define VS_RST_set() sbr (VS_RST_PORT, VS_RST)
#define VS_RST_clr() cbr (VS_RST_PORT, VS_RST)
#define VS_DDR_RST_set() sbr (VS_RST_DDRPORT, VS_RST)

#define VS_CS_set() sbr (VS_CS_PORT, VS_CS)
#define VS_CS_clr() cbr (VS_CS_PORT, VS_CS)
#define VS_DDR_CS_set() sbr (VS_CS_DDRPORT, VS_CS)

#define VS_DS_set() sbr (VS_DS_PORT, VS_DS)
#define VS_DS_clr() cbr (VS_DS_PORT, VS_DS)
#define VS_DDR_DS_set() sbr (VS_DS_DDRPORT, VS_DS)

#define VS_DREQ_wait() while (!(VS_DREQ_PIN&(1<<VS_DREQ)))

//SCI registers
#define SC_MODE        0x00
#define SC_STATUS      0x01
#define SC_BASS        0x02
#define SC_CLOCKF      0x03
#define SC_DECODE_TIME 0x04
#define SC_AUDATA      0x05
#define SC_WRAM        0x06
#define SC_WRAMADDR    0x07
#define SC_HDAT0       0x08
#define SC_HDAT1       0x09
#define SC_AIADDR      0x0A
#define SC_VOL         0x0B
#define SC_AICTRL0     0x0C
#define SC_AICTRL1     0x0D
#define SC_AICTRL2     0x0E
#define SC_AICTRL3     0x0F

// SCI instructions
#define SCI_RD      0x03 
#define SCI_WR      0x02


// MODE (0x00) register bitfields
#define SM_DIFF	 	        0	// Differential 0 normal in-phase audio1 left channel inverted
#define SM_LAYER12		1	// Allow MPEG layers I & II 0 no 1 yes
#define SM_RESET		2	//Soft reset 0 no reset 1 reset
#define SM_CANCEL		3	//Cancel decoding current file 0 no 1 yes
#define SM_EARSPEAKER_LO	4	// EarSpeaker low setting 0 off 1 active
#define SM_TESTS		5	//Allow SDI tests 0 not allowed 1 allowed
#define SM_STREAM		6	//Stream mode 0 no 1 yes
#define SM_EARSPEAKER_HI	7	//EarSpeaker high setting 0 off 1 active
#define SM_DACT                 8	// DCLK active edge 0 rising 1 falling
#define SM_SDIORD		9	// SDI bit order 0 MSb first 1 MSb last
#define SM_SDISHARE		10	// Share SPI chip select 0 no 1 yes
#define SM_SDINEW		11	//VS1002 native SPI modes 0 no 1 yes
#define SM_ADPCM		12	//PCM/ADPCM recording active 0 no 1 yes
#define SM_LINE1  		14	// MIC / LINE1 selector 0 MICP 1 LINE1
#define SM_CLK  		15	//RANGE Input clock range 0 12..13 MHz 1 24..26 MHz

// STATUS (0x01) register bitfields
#define SS_DO_NOT_JUMP 15	// Header in decode, do not fast forward/rewind
#define SS_SWING	12      //14:12 		//Set swing to +0 dB, +0.5 dB, .., or +3.5 dB
/*
// Although the range of the register is upto 7, higher settings than 2 do not work and should not be used.
#define SS_SWING0	0x0000 		
#define SS_SWING1	0x001<<12	// +0.5dB
#define SS_SWING2	0x010<<12	// +1.0 dB
*/
#define SS_VCM_OVERLOAD 11 	//GBUF overload indicator ’1’ = overload
#define SS_VCM_DISABLE 10 	//GBUF overload detection ’1’ = disable 
//9:8 reserved
#define SS_VER 4		//Version 7:4
#define SS_APDOWN2 3 		//Analog driver powerdown
#define SS_APDOWN1 2 		//Analog internal powerdown
#define SS_AD_CLOCK 1 		//AD clock select, ’0’ = 6 MHz, ’1’ = 3MHz
#define SS_REFERENCE_SEL 0 	//Reference voltage selection, ’0’ = 1.23V, ’1’ = 1.65V

//CLOCKF (0x03) register bitfields
#define SC_MULT0  0x0000  //x1
#define SC_MULT1  0x2000  //x2
#define SC_MULT2  0x4000  //x2.5
#define SC_MULT3  0x6000  //x3
#define SC_MULT4  0x8000  //x3.5
#define SC_MULT5  0xa000  //x4
#define SC_MULT6  0xc000  //x4.5
#define SC_MULT7  0xe000  //x5

#define SC_ADD0  0x0000   // disabled
#define SC_ADD1  0x0800   // 1.0x
#define SC_ADD2  0x1000   // 1.5x
#define SC_ADD3  0x1800   // 2x

// XRAM addresses
#define endFillByte 0x1e06


extern void VS_DATA (UINT8 DATA);
extern void VS_SINTEST (void);
extern void VS_SOFTRST (void);
extern void VS_INIT (void);
extern void VS_DATA32 (UINT8* DATA);
extern void VS_DATA_TX (UINT8* DATA, UREG len);
extern void VS_NEW_SINTEST (void);
extern void VS_SINSWEEP (void);
extern void VS_SCI_RD_TEST (void);
extern UINT16 VS_endFillByte (void);

extern void VS_VOL (void);
//extern UCHAR VS_BUFFER32 [32];
//extern UCHAR* pVS_BUFFER32;
//extern UCHAR VS_BUFFER_pos;