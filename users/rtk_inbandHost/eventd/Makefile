INBAND_LIB = ../../rtk_inbandLib/inband.a

all:    iweventd
#all:   eventd

CC      = rsdk-linux-gcc
STRIP   = rsdk-linux-strip

CFLAGS  =
IFLAGS  =
LDFLAGS = --static

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += --static -Wl,--gc-sections
endif


DEBUG   = -Wall -Os -DDEBUG
#CFLAGS += $(DEBUG)

iweventd: Makefile iweventd.o
	$(CC) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $(LDFLAGS) iweventd.o $(INBAND_LIB)
	$(STRIP) $@

#eventd: Makefile eventd.o
#	$(CC) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $(LDFLAGS) eventd.o $(INBAND_LIB)
#	$(STRIP) $@

clean:
#	rm -f *.o eventd
	rm -f *.o iweventd

romfs:
	$(ROMFSINST) /bin/iweventd
#	$(ROMFSINST) /bin/eventd
