CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip

PROG        = usb-modeswitch-data
VERS        = 20100623
RM          = /bin/rm -f

romfs:
	$(ROMFSINST) ./usb_modeswitch.d /etc/usb_modeswitch.d
	$(ROMFSINST) ./custom.d /etc/usb_modeswitch.d

clean:
	$(RM) -f $(EXEC) *.elf *.o *.a
