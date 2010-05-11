
#include "nic.h"
#include "nike_e.h"
#include "io.h"


void nic_init(void)
{
	NICInit();
}

unsigned int nic_poll(void)
{        
        unsigned int packetLength;         
        packetLength=NICRetreivePacketData(ETH_MAX_PACKET_SIZE/*+3*/, (unsigned char *)ETH_PKT.hdr.dst_mac);
        if (packetLength)
        {
          ETH_PKT_len=packetLength;        
          ETH_PKT_mode=1;               
        }           		        
	return packetLength;	
        
}
