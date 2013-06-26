include ../../.linux_config

OBJS		=	ivrclient.o ivrserver.o ivrhandler.o ivrnetcfg.o 
LDFLAGS		= -L../../voip_manager/ -lvoip_manager

ifeq ($(CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT), y)
LDFLAGS		+=	-ldl -lpthread
endif

DEPENDENCY	= $(OBJS:.o=.d)
CFLAGS		= -I../../includes/ 
# 865x only
CFLAGS		+= -I../../../../include/ -fPIC 

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS 		+= -I../../ui/include
endif

ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
LDFLAGS	+= -lpthread -ldl
endif

IVR_CLIENT	= libivrclient.a
IVR_SERVER	= libivrserver.so 

all: $(OBJS) $(IVR_CLIENT)
	@#$(CC) $(LDFLAGS) $(CFLAGS) -o $(IVR_SERVER) ivrserver.o ivrhandler.o ivrnetcfg.o ../../flash/libvoip_flash_client.a 
	$(LD) -o $(IVR_SERVER) -shared ivrserver.o ivrhandler.o ivrnetcfg.o ../../flash/libvoip_flash_client.a 

$(IVR_CLIENT): ivrclient.o
	$(AR) rcs $(IVR_CLIENT) $<

#voip_manager.o: ../../voip_manager/voip_manager.c
#	$(CC) $(LDFLAGS) $(CFLAGS) -c -o $@ $<

romfs:
	$(ROMFSINST) $(IVR_SERVER) /lib/

nfs:
	cp ivrserver /work/nfs/ap/ivrserver

clean: 
	rm -f $(OBJS) $(DEPENDENCY)
	rm -f $(IVR_CLIENT) $(IVR_SERVER)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)

