CFLAGS += -Os -Wall 
#CFLAGS := -Os -Wall -I../../linux-2.6.30/include -I../../linux-2.6.30/include/linux
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += -s

EXEC = mbpk_eject 
OBJS = mbpk_eject.o
all: $(EXEC)

$(EXEC): $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o

