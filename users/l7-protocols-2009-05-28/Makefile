CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip

PROG        = l7-protocols
VERS        = 20090528
#RM          = /bin/rm -f

romfs:
	rm -fr ../../romfs/etc/l7-protocols >/dev/null 2>&1
	mkdir -p ../../romfs/etc/l7-protocols
	cp -fr * ../../romfs/etc/l7-protocols

clean:
	#$(RM) -f $(EXEC) *.elf *.o *.a