CC = rsdk-linux-gcc
CXX = rsdk-linux-g++
LD = rsdk-linux-gcc
COPT = -Os 
CFLAGS +=-I../../linux-2.6.30/include -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN $(COPT)
#CFLAGS=-I -Os -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN

CFLAGS += $(EXTRA_CFLAGS)
EXEC = igmpproxy
OBJS = mroute-api.o lib.o ifvc.o syslog.o timeout.o main.o

ifeq ($(CONFIG_IGMPPROXY_MULTIWAN),y)
CFLAGS += -DCONFIG_IGMPPROXY_MULTIWAN
endif

ifeq ($(CONFIG_IGMPV3_SUPPORT),y)
CFLAGS += -DCONFIG_IGMPV3_SUPPORT
OBJS   += igmpv3.o
DEPH   += igmpproxy.h
endif
all: BUILT_TIME $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(INCLUDE) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o *.bak built_time

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time	
