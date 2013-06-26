#include ../../linux-2.6.30/.config

#CFLAGS +=  -D_GNU_SOURCE
CFLAGS += -Os 
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += -s

EXEC = comgt 
OBJS = comgt.o
all: $(EXEC)

$(EXEC): $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o

