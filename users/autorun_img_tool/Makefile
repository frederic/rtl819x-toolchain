CC=gcc
DEBUG = -g
CFLAGS += -Os -Wall
CFLAGS += $(EXTRA_CFLAGS) -I ../../linux-2.6.30/include
LDFLAGS += -s

EXEC = gen_autorun_img
OBJS = gen_autorun_img.o
all:  $(EXEC)

$(EXEC): $(OBJS)

$(OBJS): gen_autorun_img.c \
	../../linux-2.6.30/drivers/usb/gadget_cathy/usb_ulinker.h \
	../../linux-2.6.30/include/linux/autoconf.h	

romfs:
	-./gen_autorun_img
	$(ROMFSINST) autorun.img /etc/autorun.img

clean:
	-rm -f $(EXEC) *.o Setup.ini
	-rm -fr files mount_img
