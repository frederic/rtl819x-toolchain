#CFLAGS = -Wall -g -Os
#CFLAGS += -DRT_WLAN
ifeq "$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP)" "y"
CFLAGS += -DCONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
endif

#SRCS = hcd.c mib.c wlan_if.c
SRCS = hcd.c mib.c

EXEC = hcd
OBJS = $(SRCS:.c=.o)

CC = rsdk-linux-gcc
LD = rsdk-linux-gcc

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /bin/reset
	
clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
