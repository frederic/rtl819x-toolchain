
EXEC = dhrystone
OBJS = dhry_1.o dhry_2.o

CROSS_COMPILE ?= rsdk-linux-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc

CFLAGS += -DNO_PROTOTYPES=1 -Os
LDLIBS += -lm

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/dhrystone

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

$(OBJS): dhry.h

