copy mac.txt mac.tmp
avreal32 -ab -p1 +MEGA168 -cRelease\exe\NikeE1B.hex -dRelease\exe\NikeE1B_eeprom.hex -d*serno3=4,mac.txt -e -w -v
avreal32 -ab -p1 +MEGA168 -dmac.eep -d*serno3=00AE,mac.tmp -w -v -fBODLEVEL=4,SUT=3,CKSEL=7,CKDIV=1,CKOUT=1,BLB0=3,BLB1=3,BOOTSZ=0,BOOTRST=1,RSTDISBL=1,EESAVE=1,DWEN=1,WDTON=0 -l2
del mac.tmp
