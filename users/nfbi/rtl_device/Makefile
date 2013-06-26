#CFLAGS = -Wall -g -Os
#CFLAGS += -DRT_WLAN

SRCS = hcd.c mib.c cmd.c wlan_if.c sys_init.c mdio_if.c inband_if.c ioh.c

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
