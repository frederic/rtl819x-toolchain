#CFLAGS = -Wall -g -Os
#CFLAGS += -DRT_WLAN
#include $(DIR_LINUX)/.config

SRCS = hcd.c mib.c cmd.c wlan_if.c sys_init.c

EXEC = hcd
OBJS = $(SRCS:.c=.o)

WLANSUPP= -DUNIVERSAL_REPEATER -DWLAN_WPA -DWLAN_WDS -DWLAN_8185AG -DWLAN_WPA2 $(TLS)
#WLANSUPP += -DWIFI_SIMPLE_CONFIG -DCONFIG_RTL8196C_AP_ROOT
WLANSUPP += -DWIFI_SIMPLE_CONFIG

CFLAGS += $(WLANSUPP)

CFLAGS += -DCONFIG_RTL8196B -DCONFIG_RTL8196C -DCONFIG_RTL8198
#CFLAGS += -DCONFIG_HCD_FLASH_SUPPORT
#CFLAGS += -DINBAND_WPS_OVER_HOST

ifeq ($(CONFIG_RTL_92D_SUPPORT),y)
CFLAGS += -DCONFIG_RTL_92D_SUPPORT
endif

CC = rsdk-linux-gcc
AR = rsdk-linux-ar
#mark_issue
LDFLAGS                =  -static
#LDLIBS += ../../inband_lib/inband.a
LDLIBS += ../../rtk_inbandLib/inband.a

all: $(EXEC)

$(EXEC): $(OBJS) ../../rtk_inbandLib/inband.a 
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(CFLAGS) $(LDLIBS)

#ioh.a: ioh.o
#	$(AR) rcs $@ ioh.o

#inband.a: ioh.o inband_if.o
#	$(AR) rcs $@ ioh.o inband_if.o

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o *.a

romfs:
	$(ROMFSINST) /bin/$(EXEC)
