CFLAGS += -Os -Wall
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += -s

EXEC = mnet 
OBJS = mnet.o
all: BUILT_TIME $(EXEC)

$(EXEC): $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o built_time

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time