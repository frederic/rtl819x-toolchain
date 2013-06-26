#CFLAGS = -Wall -g -Os
#CFLAGS += -DRT_WLAN
CFLAGS = -I./
include ../.config

CFLAGS += -Os

SRCS = ioh.c inband_if.c hapd_api.c
APP_SRCS = ioh.c inband_if.c inband_cmd.c

APP_OBJS = $(APP_SRCS:.c $(CFLAGS) =.o)
OBJS = $(SRCS:.c $(CFLAGS) =.o)

EXEC = inband

CC = rsdk-linux-gcc
LD = rsdk-linux-gcc

all: inband.a $(EXEC)

inband.a: ioh.o inband_if.o hapd_api.o
	$(AR) rcs $@ ioh.o inband_if.o hapd_api.o

$(EXEC): $(APP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

ifeq ($(CONFIG_APP_HOSTAPD_HCM),y)
romfs:
	$(ROMFSINST) /bin/$(EXEC)
#	$(ROMFSINST) /bin/fw_noweb.bin
endif

clean:
	-rm -f *.elf *.gdb *.o *.a $(EXEC)
