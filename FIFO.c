
#include "compiler.h"
#include "stdafx.h"
#include "FIFO.h"
#include "tcp.h"
#include "io.h"

UINT32 MP3fifo_pRD;//fifo_head;
UINT32 MP3fifo_pWR;//fifo_tail;
//const UINT32 MP3fifo_pSTART;
//const UINT32 MP3fifo_pEND;

UINT32 ETHfifo_pRD;//fifo_head;
UINT32 ETHfifo_pWR;//fifo_tail;

//const UINT32 ETHfifo_pSTART;
//const UINT32 ETHfifo_pEND;

UINT8 ETHfifo_CNT;

//volatile UREG MP3FIFO_STATE=MP3FIFO_STATE_FREE;
/*
#pragma vector = TIMER1_OVF_vect
__interrupt  void T1_TASK(void)
{
 static UREG div;
 
   if ((div&(0x7F))==127)
   {
     OCR1A = (UINT16)(MP3fifo_free()>>7);
   }
 div++;
 //
}
*/

#pragma inline =forced
void fifo_spi_write(UINT8 c)
{
	SPDR = c;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished            
}

#pragma inline =forced
UINT8 fifo_spi_read(void)
{
	SPDR = 0xFF;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished            
	return SPDR;
}


UINT32 ETHfifo_free(void)
{
	UINT32 free;
	if(ETHfifo_pRD > ETHfifo_pWR)
	{
		free = ETHfifo_pRD-ETHfifo_pWR;
	}
	else if(ETHfifo_pRD < ETHfifo_pWR)
	{
		free = ETHfifo_pEND-ETHfifo_pWR+1+ETHfifo_pRD-ETHfifo_pSTART;
	}
	else
	{
		free = ETHfifo_pEND-ETHfifo_pSTART+1;
	}
	return free-1;
}

UINT32 ETHfifo_len(void)
{
	UINT32 len;

	if(ETHfifo_pRD > ETHfifo_pWR)
	{
		len = ETHfifo_pEND-ETHfifo_pRD+1+ETHfifo_pWR-ETHfifo_pSTART;
	}
	else if(ETHfifo_pRD < ETHfifo_pWR)
	{
		len = ETHfifo_pWR-ETHfifo_pRD;
	}
	else
	{
		len = 0;
	}

	return len;
}


UINT32 MP3fifo_free(void)
{
	UINT32 free;
	if(MP3fifo_pRD > MP3fifo_pWR)
	{
		free = MP3fifo_pRD-MP3fifo_pWR;
	}
	else if(MP3fifo_pRD < MP3fifo_pWR)
	{
		free = MP3fifo_pEND-MP3fifo_pWR+1+MP3fifo_pRD-MP3fifo_pSTART;
	}
	else
	{
		free = MP3fifo_pEND-MP3fifo_pSTART+1;
	}
	return free-1;
}


UINT32 MP3fifo_len(void)
{
	UINT32 len;

	if(MP3fifo_pRD > MP3fifo_pWR)
	{
		len = MP3fifo_pEND-MP3fifo_pRD+1+MP3fifo_pWR-MP3fifo_pSTART;
	}
	else if(MP3fifo_pRD < MP3fifo_pWR)
	{
		len = MP3fifo_pWR-MP3fifo_pRD;
	}
	else
	{
		len = 0;
	}

	return len;
}

void fifo_init(void)
{

        MP3fifo_pWR = MP3fifo_pSTART;
	MP3fifo_pRD = MP3fifo_pSTART;        


        ETHfifo_pWR = ETHfifo_pSTART;
	ETHfifo_pRD = ETHfifo_pSTART;
        
        sbi (FM_DDR, FM_CS);
        
	FM_CS_ENABLE();
        SPDR=FM_WREN;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished            
	FM_CS_DISABLE();

	FM_CS_ENABLE();
        SPDR=FM_WRSR;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished            
        SPDR=0x00;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished                   
	FM_CS_DISABLE();
        
	FM_CS_ENABLE();
        SPDR=FM_WRSR;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished            
        SPDR=0x00;        
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished            
	FM_CS_DISABLE();
	return;
}


char const __flash _EthWRlen[]="\r\nEFW l:";
char const __flash _EthTS[]="TS:"; 
char const __flash _EthCNT[]="EFCNT:";
char const __flash _EthpWRH[]="pWRH:";
char const __flash _EthpWRL[]="pWRL:";

char const __flash _EthRDlen[]="\r\nEFR l:";
char const __flash _EthpRDH[]="pRDH:";
char const __flash _EthpRDL[]="pRDL:";



void fifo_test()
{
        UINT32 addr=0x00000000;
        UINT32 ENDaddr=0x0001FFFF;
        sbi(PORTD, PD5);
        FM_CS_ENABLE();
	SPDR=FM_WREN;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
	FM_CS_DISABLE();

	FM_CS_ENABLE();
	SPDR=FM_WRITE;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
        
       	SPDR=(addr>>16);
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
	SPDR=(addr>>8);
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
	SPDR=(addr);
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
        
        while (addr<=ENDaddr)
        {
          SPDR=(UREG)(addr);
          while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
          addr++;        	
  //	  addr &= (FM_SIZE-1);
        }        
	FM_CS_DISABLE();                
        UREG k, error=0;
        addr=0x00000000;
        FM_CS_ENABLE();
	SPDR=FM_READ;
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished
        SPDR=(addr>>16);
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
	SPDR=(addr>>8);
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
	SPDR=(addr);
        while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
        while (addr<ENDaddr)
        {
          SPDR=0xFF;
          while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished    
          k=SPDR;
          if (k!=(UREG)(addr)) error=1;
          addr++;        	
//  	  addr &= (FM_SIZE-1);
        }        
	FM_CS_DISABLE();
        cbi(PORTD, PD5);
        if (error) 
        {
          for (k=1; k<10;k++)
          {
            sbi (PORTD, PD5);
            _delay_ms(500);
            cbi (PORTD, PD5);
            _delay_ms(500);  
          }
        }                
}

/* 
void ETHfifo_write(UINT8 *data, UINT16 len, UINT8 TS)
{
	UINT32 pWR;
        UINT16 len1, len2, len3;
	UINT8 c;
	if(len == 0)	return;
        if (ETHfifo_free() < ETH_MAX_PACKET_SIZE+3) return; // если пакет+длина+таймаут не влезут - не пишем
	UINT8 header_cnt=0;
       	pWR = ETHfifo_pWR;
        len+=3; //  к длине Ethernet пакета добавим 2 байта его длины и 1 байт метки времени
        #ifdef CONSOLE_DEBUG
        _print_num (_EthWRlen,len); _print_num (_EthTS,(UINT16)TS);
        _print_num (_EthpWRH,(UINT16)(pWR>>16)); _print_num (_EthpWRL,(UINT16)pWR);
        #endif
        if ((pWR+len)>ETHfifo_pEND) // при записи упремся в потолок... надо разбить на 2 записи
        {          
          len1=(ETHfifo_pEND-pWR+1); // размер первой порции
          len2=len-len1; // размер второй порции

            FM_CS_ENABLE();
  	    fifo_spi_write(FM_WREN);
	    FM_CS_DISABLE();
  	    FM_CS_ENABLE();
            fifo_spi_write(FM_WRITE); 
       	    fifo_spi_write(pWR>>16);
	    fifo_spi_write(pWR>>8);
	    fifo_spi_write(pWR);	// Открыли сессию записи в FIFO                         
            // дописали до крайнего верхнего байта - двигаем голову в начало
            pWR= ETHfifo_pSTART;	  
            ETHfifo_pWR=pWR;  
	    for(; len1!=0; len1--)
	    {
               switch (header_cnt)
                {
                  case 0:
                    c=(UINT8) ((len-3) >>8);
                    header_cnt++;
                    break;
                  case 1:
                    c=(UINT8) (len-3);
                    header_cnt++;
                    break;
                  case 2:  
                    c= TS;
                    header_cnt++;
                    break;
                  default:
                    c = *data++;
                  break;
                }             		
                SPDR = c;
		while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished 
	    }
	  FM_CS_DISABLE();               

          if (len2)
          {
            // вторая запись - дописываем то, что не влезло                    
            FM_CS_ENABLE();
	    fifo_spi_write(FM_WREN);
	    FM_CS_DISABLE();
	    FM_CS_ENABLE();
            fifo_spi_write(FM_WRITE);
       	    fifo_spi_write(pWR>>16);
	    fifo_spi_write(pWR>>8);
	    fifo_spi_write(pWR);	
            // двигаем голову на след. ячейку
            pWR+= len2;
	    ETHfifo_pWR = pWR;
	    for(; len2!=0; len2--)
	    {
               switch (header_cnt)
                {
                  case 0:
                    c=(UINT8) ((len-3) >>8);
                    header_cnt++;
                    break;
                  case 1:
                    c=(UINT8) (len-3);
                    header_cnt++;
                    break;
                  case 2:  
                    c= TS;
                    header_cnt++;
                    break;
                  default:
                    c = *data++;
                  break;
                }                   
		SPDR = c;
		while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished
	    }
	  FM_CS_DISABLE();
          }
        }
        else // пакет влезет без переполнения
        {
          FM_CS_ENABLE();
	  fifo_spi_write(FM_WREN);
	  FM_CS_DISABLE();
	  FM_CS_ENABLE();
          fifo_spi_write(FM_WRITE);
       	  fifo_spi_write(pWR>>16);
	  fifo_spi_write(pWR>>8);
	  fifo_spi_write(pWR);	
          len3=len;
          pWR+= len3;
	  ETHfifo_pWR = pWR;
	  for(; len3!=0; len3--)
	  {
               switch (header_cnt)
                {
                  case 0:
                    c=(UINT8) ((len-3) >>8);
                    header_cnt++;
                    break;
                  case 1:
                    c=(UINT8) (len-3);
                    header_cnt++;
                    break;
                  case 2:  
                    c= TS;
                    header_cnt++;
                    break;
                  default:
                    c = *data++;
                  break;
                }                 
                SPDR = c;
		while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished 
	  }
	  FM_CS_DISABLE();
        }         
	ETHfifo_CNT++;// увеличиваем счетчик пакетов в FIFO
        #ifdef CONSOLE_DEBUG
        _print_num (_EthCNT,(UINT16)ETHfifo_CNT);
        #endif
}
*/

/*
#pragma inline=forced
void ETHfifo_read(UINT8 *data, UINT16 len)
{
	UINT32 pRD;
        UINT16 len1, len2;
	UINT8 c;	
	if(len == 0)	return;
        if (ETHfifo_len() < (UINT32) len) return; // в буфер записано меньше, чем пытаемся считать
	pRD = ETHfifo_pRD;
        #ifdef CONSOLE_DEBUG
        _print_num (_EthRDlen,len);
        _print_num (_EthpRDH,(UINT16)(pRD>>16)); _print_num (_EthpRDL,(UINT16)pRD);
        #endif
        if ((pRD+len)>ETHfifo_pEND) // при чтении упремся в потолок... надо разбить на 2 записи
        {          
          len1 =(ETHfifo_pEND-pRD+1); // размер первой порции
          len2=len-len1; // размер второй порции

	  FM_CS_ENABLE();
          fifo_spi_write(FM_READ);
       	  fifo_spi_write(pRD>>16);
	  fifo_spi_write(pRD>>8);
	  fifo_spi_write(pRD);	                    
	  for(; len1!=0; len1--)
	  {
		SPDR=0xFF;
         	while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished 
                c=SPDR;
		*data++=c;
	  }
	  FM_CS_DISABLE();          
          pRD= ETHfifo_pSTART;	  
	  ETHfifo_pRD = pRD;  //двигаем хвост в начало
          // второе чтение -считываем снизу           
          if (len2)
          {
	    FM_CS_ENABLE();
            fifo_spi_write(FM_READ);
       	    fifo_spi_write(pRD>>16);
	    fifo_spi_write(pRD>>8);
	    fifo_spi_write(pRD);	
            pRD+= len2;	  
	    ETHfifo_pRD = pRD;
	    for(; len2!=0; len2--)
	    {
		SPDR = 0xFF;
                while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished 
                c=SPDR;
		*data++=c;
	    }
	    FM_CS_DISABLE();    
          }
        }
        else // пакет можно читать целиком
        {
	  FM_CS_ENABLE();
          fifo_spi_write(FM_READ);
       	  fifo_spi_write(pRD>>16);
	  fifo_spi_write(pRD>>8);
	  fifo_spi_write(pRD);	
          pRD+= len;
          ETHfifo_pRD = pRD; 
	  for(; len!=0; len--)
	  {
		SPDR = 0xFF;
           	while (!(SPSR & (1<<SPIF))); // wait SPI comm. finished 
                c=SPDR;
		*data++=c;
	  }
	  FM_CS_DISABLE();
        }            
}
*/
/*
// Читаем 3 байта заголовка (длина, таймаут)
UINT16 ETHfifo_PKT_read(UINT8* eth_data, UINT8* TS)
{ 
  UINT8 data[3]; // Временное хранилище
  UINT16 len;  
  ETHfifo_read(&data[0], 3); // Прочитаем 3 байта LH,LL,TS
  len = (UINT16)(((data[0]<<8)|data[1]));//len=LHLL
  *TS=data[2];// *TS=TS
  #ifdef CONSOLE_DEBUG
  _print_num (_EthTS,(UINT16) (*TS));
  #endif
  // Ограничим длину пакета на всякий случай, чтоб при отладке не запороть ОЗУ)
  if (len> ETH_MAX_PACKET_SIZE) len = ETH_MAX_PACKET_SIZE;
  ETHfifo_read(eth_data, len);
  if (ETHfifo_CNT) ETHfifo_CNT--;// уменьшаем счетчик пакетов в FIFO
  #ifdef CONSOLE_DEBUG
  _print_num (_EthCNT,(UINT16)ETHfifo_CNT);
  #endif
  return len;
}
*/