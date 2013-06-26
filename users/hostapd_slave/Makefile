#CFLAGS = -Wall -g -Os
#CFLAGS += -DRT_WLAN
#include ../.config
include $(DIR_LINUX)/.config

ifdef CONFIG_RTL_8196C
CFLAGS += -DCONFIG_RTL_8196C
endif

ifdef CONFIG_RTL_8198
CFLAGS += -DCONFIG_RTL_8198
endif

CFLAGS += -Os

SRCS = hapd_slave.c
SYSINIT_SRCS = sysinit.c

EXEC = hapd_slave
SYSINIT_EXEC = sysinit
OBJS = $(SRCS:.c=.o)
SYSINIT_OBJS = $(SYSINIT_SRCS:.c=.o)

LDLIBS += ../inband_lib/inband.a

CC = rsdk-linux-gcc
LD = rsdk-linux-gcc

all: $(EXEC) $(SYSINIT_EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(CFLAGS) $(OBJS) $(LDLIBS)

$(SYSINIT_EXEC): $(SYSINIT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SYSINIT_OBJS)
romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /bin/$(SYSINIT_EXEC)
	
clean:
	-rm -f $(EXEC) $(SYSINIT_EXEC) *.elf *.gdb *.o *.a
