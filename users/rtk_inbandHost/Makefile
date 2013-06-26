CC		= rsdk-linux-gcc
COPT		= -O2 -Wall
CFLAGS		= ${COPT}
#CFLAGS		+= -DINBAND_HOST
#CFLAGS		+= -DINBAND_WPS_OVER_HOST
CFLAGS		+= -DMULTI2UNI
#LDFLAGS	=  -static
#STRIP		= mips-uclibc-strip
LIBS		= ../rtk_inbandLib/inband.a

all: inband
	make -C eventd


inband_sources = main.c 

inband:	$(inband_sources:.c=.o)
	$(CC) -s $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)	

clean:
	make -C eventd clean
	rm -f inband *.o *.a

romfs:
	$(ROMFSINST) /bin/inband
	$(ROMFSINST) mib_sample/ /etc
	make -C eventd romfs


