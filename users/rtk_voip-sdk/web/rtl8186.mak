include ../.linux_config

ifndef WEB_FLAGS
target = error
else
target = build
endif

LIBA = libweb_voip.a
OBJS = web_voip.o web_voip_general.o web_voip_tone.o web_voip_ring.o web_voip_other.o web_voip_config.o web_voip_network.o
CFLAGS += $(WEB_FLAGS) -Werror
DEPENDENCY	= $(OBJS:.o=.d)

ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	OBJS += web_voip_ivr_req.o
endif

ifneq ($(BUILD_DIALPLAN), 0)
CFLAGS += -DCONFIG_RTK_VOIP_DIALPLAN
endif

ifeq ($(CONFIG_RTK_VOIP_SIP_TLS),y)
	CFLAGS += -I../../openssl-0.9.7d/include
	OBJS += web_voip_tls.o 
endif
VOIP_SRC		=	$(VOIP_APP)/src
all: $(target)

error: 
	@echo "Please make in WEB directory."

build: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBA) $(OBJS)
	-cp voip_script.js $(WEB_DIR)/web-ap/
	-cp voip_general.asp $(WEB_DIR)/web-ap/
	-cp voip_tone.asp $(WEB_DIR)/web-ap/
	-cp voip_ring.asp $(WEB_DIR)/web-ap/
	-cp voip_other.asp $(WEB_DIR)/web-ap/
	-cp voip_config.asp $(WEB_DIR)/web-ap/
	-cp voip_network.asp $(WEB_DIR)/web-ap/
	-cp voip_script.js $(WEB_DIR)/web-gw/
	-cp voip_general.asp $(WEB_DIR)/web-gw/
	-cp voip_tone.asp $(WEB_DIR)/web-gw/
	-cp voip_ring.asp $(WEB_DIR)/web-gw/
	-cp voip_other.asp $(WEB_DIR)/web-gw/
	-cp voip_config.asp $(WEB_DIR)/web-gw/
	-cp voip_network.asp $(WEB_DIR)/web-gw/
	-cp voip_script.js $(WEB_DIR)/web-vpn/
	-cp voip_general.asp $(WEB_DIR)/web-vpn/
	-cp voip_tone.asp $(WEB_DIR)/web-vpn/
	-cp voip_ring.asp $(WEB_DIR)/web-vpn/
	-cp voip_other.asp $(WEB_DIR)/web-vpn/
	-cp voip_config.asp $(WEB_DIR)/web-vpn/
	-cp voip_network.asp $(WEB_DIR)/web-vpn/
  ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	-cp voip_ivr_req.asp $(WEB_DIR)/web-ap/
	-cp voip_ivr_req.asp $(WEB_DIR)/web-gw/
	-cp voip_ivr_req.asp $(WEB_DIR)/web-vpn/
  endif
  ifeq ($(CONFIG_RTK_VOIP_SIP_TLS),y)
	-cp voip_tls.asp $(WEB_DIR)/web-ap/
	-cp voip_tls.asp $(WEB_DIR)/web-gw/
	-cp voip_tls.asp $(WEB_DIR)/web-vpn/
  endif
	-cp voip_sip_status.asp $(WEB_DIR)/web-ap/
	-cp voip_sip_status.asp $(WEB_DIR)/web-gw/
	-cp voip_sip_status.asp $(WEB_DIR)/web-vpn/

clean:
	rm -f $(LIBA) $(OBJS) *.d
	rm -f $(WEB_DIR)/web-ap/voip_script.js
	rm -f $(WEB_DIR)/web-ap/voip_general.asp
	rm -f $(WEB_DIR)/web-ap/voip_tone.asp
	rm -f $(WEB_DIR)/web-ap/voip_ring.asp
	rm -f $(WEB_DIR)/web-ap/voip_other.asp
	rm -f $(WEB_DIR)/web-ap/voip_config.asp
	rm -f $(WEB_DIR)/web-ap/voip_network.asp
	rm -f $(WEB_DIR)/web-gw/voip_script.js
	rm -f $(WEB_DIR)/web-gw/voip_general.asp
	rm -f $(WEB_DIR)/web-gw/voip_tone.asp
	rm -f $(WEB_DIR)/web-gw/voip_ring.asp
	rm -f $(WEB_DIR)/web-gw/voip_other.asp
	rm -f $(WEB_DIR)/web-gw/voip_config.asp
	rm -f $(WEB_DIR)/web-gw/voip_network.asp
	rm -f $(WEB_DIR)/web-vpn/voip_script.js
	rm -f $(WEB_DIR)/web-vpn/voip_general.asp 
	rm -f $(WEB_DIR)/web-vpn/voip_tone.asp
	rm -f $(WEB_DIR)/web-vpn/voip_ring.asp
	rm -f $(WEB_DIR)/web-vpn/voip_other.asp
	rm -f $(WEB_DIR)/web-vpn/voip_config.asp
	rm -f $(WEB_DIR)/web-vpn/voip_network.asp
  ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	rm -f $(WEB_DIR)/web-ap/voip_ivr_req.asp
	rm -f $(WEB_DIR)/web-gw/voip_ivr_req.asp
	rm -f $(WEB_DIR)/web-vpn/voip_ivr_req.asp
  endif
  ifeq ($(CONFIG_RTK_VOIP_SIP_TLS),y)
	rm -f $(WEB_DIR)/web-ap/voip_tls.asp
	rm -f $(WEB_DIR)/web-gw/voip_tls.asp
	rm -f $(WEB_DIR)/web-vpn/voip_tls.asp
  endif
	rm -f $(WEB_DIR)/web-ap/voip_sip_status.asp
	rm -f $(WEB_DIR)/web-gw/voip_sip_status.asp
	rm -f $(WEB_DIR)/web-vpn/voip_sip_status.asp
	rm -f $(DEPENDENCY)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)
