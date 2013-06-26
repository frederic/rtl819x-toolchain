include ../.linux_config

ifndef WEB_FLAGS
target = error
else
target = build
endif

LIBA = libweb_voip.a
OBJS = web_voip.o web_voip_general.o web_voip_tone.o web_voip_ring.o web_voip_other.o web_voip_config.o
CFLAGS += $(WEB_FLAGS)
DEPENDENCY	= $(OBJS:.o=.d)

ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	OBJS += web_voip_ivr_req.o
endif

ifneq ($(BUILD_DIALPLAN), 0)
CFLAGS += -DCONFIG_RTK_VOIP_DIALPLAN
endif

all: $(target)

error: 
	@echo "Please make in WEB directory."

build: $(OBJS)
	$(AR) rs $(LIBA) $(OBJS)

romfs:
	$(ROMFSINST) voip_script.js /www/voip_script.js
	$(ROMFSINST) voip_general.asp /www/voip_general.asp
	$(ROMFSINST) voip_tone.asp /www/voip_tone.asp
	$(ROMFSINST) voip_ring.asp /www/voip_ring.asp
	$(ROMFSINST) voip_other.asp /www/voip_other.asp
	$(ROMFSINST) voip_config.asp /www/voip_config.asp
  ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	$(ROMFSINST) voip_ivr_req.asp /www/voip_ivr_req.asp
  endif
	$(ROMFSINST) voip_sip_status.asp /www/voip_sip_status.asp
	$(ROMFSINST) -s /var/config_voip.dat /www/config_voip.dat

clean:
	rm -f $(LIBA) $(OBJS) *.d

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)
