This is project of DIY Ethernet radio player based on Atmel AVR ATmega32 MCU with Microchip ENC28j60 Ethernet controller and VLSI vs1053b hardware codec.
As the additional RAM for mp3/aac ring buffer it is used FM25V10 (1Mb Serial 3V F-RAM Memory). All the peripheral is connected to the one common SPI bus.

Also it is added 16\*4 Winstar LCD module for information and MP3 tags printing.

The key advantages of this project is: built-in control through the web server w/o external memory usage and the TCP/IP Fast Retransmit mechanism realization that is really useful for operation with long distance radio station where some packets could be lost.

Currently it is only Shoutcast/Icecast protocol supported.

The TCP/IP stack used is uNike by Dmitry Oparin a.k.a. RST7/CBSIE.

See my web site and my blog for details: http://alyer.frihost.net, http://sonny80.livejournal.com.
