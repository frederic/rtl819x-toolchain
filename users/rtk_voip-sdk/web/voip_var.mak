VOIP_FILES +=  $(VOIP_APP)/web/web_voip.c \
	 $(VOIP_APP)/web/web_voip_general.c \
	 $(VOIP_APP)/web/web_voip_tone.c \
	 $(VOIP_APP)/web/web_voip_ring.c \
	 $(VOIP_APP)/web/web_voip_other.c \
	 $(VOIP_APP)/web/web_voip_config.c \
	 $(VOIP_APP)/web/web_voip_network.c

ifeq ($(CONFIG_RTK_VOIP_IVR),y)
VOIP_FILES +=  $(VOIP_APP)/web/web_voip_ivr_req.c
endif

ifneq ($(BUILD_DIALPLAN), 0)
VOIP_CFLAGS += -DCONFIG_RTK_VOIP_DIALPLAN
endif

ifeq ($(CONFIG_RTK_VOIP_SIP_TLS),y)
VOIP_CFLAGS += -I../../openssl-0.9.7d/include
VOIP_FILES +=  $(VOIP_APP)/web/web_voip_tls.c
endif

VOIP_LDFLAGS += -lpthread
ifeq ($(x86),1)
# do nothing
else
VOIP_LDFLAGS += -L$(VOIP_APP)/voip_manager -lvoip_manager -ldl
endif

