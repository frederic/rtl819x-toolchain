ifeq ($(wildcard $(DIR_LINUX)/.config),)
include ../../../linux-2.6.30/.config
else
include $(DIR_LINUX)/.config
endif

ifeq ($(wildcard $(DIR_USERS)/.config),)
include ../../.config
else
include $(DIR_USERS)/.config
endif

ifdef CONFIG_RTL_8198_AP_ROOT
AP=1
SKUTAG=ap
else
GW=1
SKUTAG=gw
endif

#------------------------------------------------------------------------------
ifndef COMMON_CFLAGS
$(error COMMON_CFLAGS is not defined)
endif

ifndef APMIB_SHARED
$(error APMIB_SHARED is not defined)
endif

ifndef APMIB_LIB
$(error APMIB_LIB is not defined)
endif

ifeq ($(APMIB_SHARED),1)
ifneq ($(CONFIG_SYSVIPC), y)
$(error You have to turn on the kernel option "System V IPC" for shared APMIB)
endif
endif

#------------------------------------------------------------------------------
.SUFFIXES:
.SUFFIXES: .o .c
.PHONY: clean all depend

LDFLAGS =  -g
CFLAGS = -Os -pipe
DEPEND = ./.depend


#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =

#------------------------------------------------------------------------------
# CFLAGS
#------------------------------------------------------------------------------
DEBUG	= -g -Wall
IFLAGS 	= -I.
CFLAGS += $(COMMON_CFLAGS) $(EXTRA_CFLAGS)

SOURCES = apmib.c mibtbl.c

ifeq ($(CONFIG_APP_RTK_VOIP),y)
SOURCES += $(VOIP_APP)/flash/voip_flash.c $(VOIP_APP)/flash/voip_flash_mib.c \
	$(VOIP_APP)/flash/voip_flash_server.c $(VOIP_APP)/flash/voip_flash_client.c \
	$(VOIP_APP)/flash/voip_flash_tool.c $(VOIP_APP)/voip_manager/voip_manager.c
endif

OBJS = $(SOURCES:.c=.o)

#------------------------------------------------------------------------------
# targets
#------------------------------------------------------------------------------
all: depend $(APMIB_LIB)

ifeq ($(APMIB_SHARED),1)
$(APMIB_LIB): $(OBJS)
	$(CC) -s -shared -o $@ $(OBJS)
else
$(APMIB_LIB): $(OBJS)
	$(AR) rcs $(APMIB_LIB) $(OBJS)
endif

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.so *.a

depend: $(SOURCES)
	if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(DEBUG) $(CFLAGS) $(IFLAGS) -MM $^ > $(DEPEND); \
	fi
	
-include $(DEPEND)

ifeq ($(APMIB_SHARED),1)
.c.o:
	$(CC) -c -o $@ -fpic $(DEBUG) $(CFLAGS) $(IFLAGS) $<
else
.c.o:
	${CC} -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<
endif
