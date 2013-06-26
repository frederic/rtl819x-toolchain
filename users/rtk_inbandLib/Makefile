#CFLAGS = -Wall -g -Os
#CFLAGS += -DRT_WLAN
CFLAGS = -I./
#include ../../linux-2.4.18/.config

CFLAGS += -Os -DINBAND_HOST

SRCS = ioh.c inband_if.c inband_api.c

OBJS = $(SRCS:.c $(CFLAGS) =.o)

#CC = mips-linux-gcc
#LD = mips-linux-gcc
CC = rsdk-linux-gcc
LD = rsdk-linux-gcc

all: inband.a 

inband.a:inband_api.o ioh.o inband_if.o
	$(AR) rcs $@ inband_api.o ioh.o inband_if.o

clean:
	-rm -f *.elf *.gdb *.o *.a $(EXEC)
