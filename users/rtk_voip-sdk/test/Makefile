include ../.linux_config

CFLAGS += -I../includes
ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
LDFLAGS		=	-lpthread -ldl
endif
ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
LDFLAGS += --static
endif
#LIBS = ../voip_manager/libvoip_manager.a
LDFLAGS += -L../voip_manager/ -lvoip_manager

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS 		+= -I../ui/include
endif

VOIPBOX_DBG=1
VOIPBOX_FSKGEN=1
VOIPBOX_PULSE_DIAL=1
VOIPBOX_CP3_MEASURE=1
#VOIPBOX_LED=1
#VOIPBOX_PHONERECORD=1
VOIPBOX_REG=1
VOIPBOX_RAM=1
VOIPBOX_SLIC_RESET=1
VOIPBOX_SWITCHMII=1
#VOIPBOX_TEST=1
VOIPBOX_VMWIGEN=1
#VOIPBOX_VOICERECORD=1
#VOIPBOX_VOICEPLAY=1
#VOIPBOX_EC_TEST=1
VOIPBOX_RTCP_STATISTIC=1
VOIPBOX_RTCP_LOGGER=1
#VOIPBOX_RING_TEST=1
VOIPBOX_CLONE_MAC=1
VOIPBOX_GPIO=1
VOIPBOX_BANDWIDTH_MGR=1
VOIPBOX_SEND_2833=1
#VOIPBOX_POWER=1
ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
VOIPBOX_CLI=1	#//thlin disable for 8672 shell login work correctly
else
VOIPBOX_CLI=1
endif
VOIPBOX_TONE=1
VOIP_BOX_NETMASK=1
VOIPBOX_IVR=1
VOIPBOX_LBTEST=1
VOIP_DBG_UTIL=1
VOIPBOX_VOIPEVENT=1
VOIPBOX_DTMF_DET_CFG=1

objs-shell = voipbox.o
TARGET = voipbox

ifdef VOIPBOX_DBG
objs += dbg.o
CFLAGS += -DVOIPBOX_DBG
endif

ifdef VOIPBOX_FSKGEN
objs += fskgen.o
CFLAGS += -DVOIPBOX_FSKGEN
endif

ifdef VOIPBOX_PULSE_DIAL
objs += pulse_dial.o
CFLAGS += -DVOIPBOX_PULSE_DIAL
endif

ifdef VOIPBOX_CP3_MEASURE
objs += cp3_measure.o
CFLAGS += -DVOIPBOX_CP3_MEASURE
endif

ifdef VOIPBOX_LED
objs += led.o
CFLAGS += -DVOIPBOX_LED
endif

ifdef VOIPBOX_PHONERECORD
objs += phonerecord.o
CFLAGS += -DVOIPBOX_PHONERECORD
endif

ifdef VOIPBOX_REG
objs += reg.o
CFLAGS += -DVOIPBOX_REG
endif

ifdef VOIPBOX_RAM
objs += ram.o
CFLAGS += -DVOIPBOX_RAM
endif

ifdef VOIPBOX_SLIC_RESET
objs += slic_reset.o
CFLAGS += -DVOIPBOX_SLIC_RESET
endif

ifdef VOIPBOX_SWITCHMII
objs += switchmii.o
CFLAGS += -DVOIPBOX_SWITCHMII
endif

ifdef VOIPBOX_TEST
objs += test.o
CFLAGS += -DVOIPBOX_TEST
endif

ifdef VOIPBOX_VMWIGEN
objs += vmwigen.o
CFLAGS += -DVOIPBOX_VMWIGEN
endif

ifdef VOIPBOX_VOICERECORD
objs += voicerecord.o
CFLAGS += -DVOIPBOX_VOICERECORD
endif

ifdef VOIPBOX_VOICEPLAY
objs += voiceplay.o
CFLAGS += -DVOIPBOX_VOICEPLAY
endif

ifdef VOIPBOX_EC_TEST
objs += ec_test.o
CFLAGS += -DVOIPBOX_EC_TEST
endif

ifdef VOIPBOX_RTCP_STATISTIC
objs += rtcp_statistic.o
CFLAGS += -DVOIPBOX_RTCP_STATISTIC
endif

ifdef VOIPBOX_RTCP_LOGGER
objs += rtcp_logger.o
CFLAGS += -DVOIPBOX_RTCP_LOGGER
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
objs += iphone_test.o
CFLAGS += -DCONFIG_RTK_VOIP_DRIVERS_IP_PHONE
endif

ifdef VOIPBOX_RING_TEST
objs += ring_test.o
CFLAGS += -DVOIPBOX_RING_TEST
endif

ifdef VOIPBOX_CLONE_MAC
objs += clone_mac.o
CFLAGS += -DVOIPBOX_CLONE_MAC
endif

ifdef VOIPBOX_BANDWIDTH_MGR
objs += bandwidth_mgr.o
CFLAGS += -DVOIPBOX_BANDWIDTH_MGR
endif

ifdef VOIPBOX_GPIO
objs += gpio.o
CFLAGS += -DVOIPBOX_GPIO
endif

ifdef VOIPBOX_SEND_2833
objs += send_2833.o
CFLAGS += -DVOIPBOX_SEND_2833
endif

ifdef VOIPBOX_POWER
objs += power.o
CFLAGS += -DVOIPBOX_POWER
endif

ifdef VOIP_BOX_NETMASK
objs += netmask.o
CFLAGS += -DVOIPBOX_NETMASK
endif

ifdef VOIPBOX_CLI
LIBS += voipcli/voipcli.a
CFLAGS += -DVOIPBOX_CLI
endif

ifdef VOIPBOX_TONE
objs += tone.o
CFLAGS += -DVOIPBOX_TONE
endif

ifdef VOIPBOX_LBTEST
objs += lb_test.o
CFLAGS += -DVOIPBOX_LBTEST
endif

ifdef VOIPBOX_VOIPEVENT
objs += voip_event.o 
CFLAGS += -DVOIPBOX_VOIPEVENT
endif

CFLAGS += $(ECOS_SHELL_CFLAGS)

ifdef VOIPBOX_DTMF_DET_CFG
objs += dtmf_det_cfg.o
CFLAGS += -DVOIPBOX_DTMF_DET_CFG
endif

#----------------------------------------------------------------------
all: $(objs) $(objs-shell)
	#make -C pcm-talk
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	make -C dspg_cmbs_app
endif
ifdef VOIPBOX_CLI
	make -C voipcli
endif
	$(CC) -o $(TARGET) $(objs) $(objs-shell) $(CFLAGS) $(LIBS) $(LDFLAGS)
	- mkdir build
ifdef VOIPBOX_DBG
	ln -fs $(TARGET) build/dbg
endif
ifdef VOIPBOX_FSKGEN
	ln -fs $(TARGET) build/fskgen
endif
ifdef VOIPBOX_PULSE_DIAL
	ln -fs $(TARGET) build/pulse_dial
endif
ifdef VOIPBOX_CP3_MEASURE
	ln -fs $(TARGET) build/cp3_measure
endif
ifdef VOIPBOX_LED
	ln -fs $(TARGET) build/led
endif
ifdef VOIPBOX_PHONERECORD
	ln -fs $(TARGET) build/phonerecord
endif
ifdef VOIPBOX_REG
	ln -fs $(TARGET) build/reg
endif
ifdef VOIPBOX_RAM
	ln -fs $(TARGET) build/ram
endif
ifdef VOIPBOX_SLIC_RESET
	ln -fs $(TARGET) build/slic_reset
endif
ifdef VOIPBOX_SWITCHMII
	ln -fs $(TARGET) build/switchmii
endif
ifdef VOIPBOX_TEST
	ln -fs $(TARGET) build/crash
endif
ifdef VOIPBOX_VMWIGEN
	ln -fs $(TARGET) build/vmwigen
endif
ifdef VOIPBOX_VOICERECORD
	ln -fs $(TARGET) build/voicerecord
endif
ifdef VOIPBOX_VOICEPLAY
	ln -fs $(TARGET) build/voiceplay
endif
ifdef VOIPBOX_EC_TEST
	ln -fs $(TARGET) build/ec_test
endif
ifdef VOIPBOX_RTCP_STATISTIC
	ln -fs $(TARGET) build/rtcp_statistic
endif
ifdef VOIPBOX_RTCP_LOGGER
	ln -fs $(TARGET) build/rtcp_logger
endif
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
	ln -fs $(TARGET) build/iphone_test
endif
ifdef VOIPBOX_RING_TEST
	ln -fs $(TARGET) build/ring_test
endif

ifdef VOIPBOX_CLONE_MAC
	ln -fs $(TARGET) build/clone_mac
endif

ifdef VOIPBOX_BANDWIDTH_MGR
	ln -fs $(TARGET) build/bandwidth_mgr
endif

ifdef VOIPBOX_GPIO
	ln -fs $(TARGET) build/gpio_init
	ln -fs $(TARGET) build/gpio_read
	ln -fs $(TARGET) build/gpio_write
endif

ifdef VOIPBOX_SEND_2833
	ln -fs $(TARGET) build/send_2833
endif

ifdef VOIPBOX_POWER
	ln -fs $(TARGET) build/power
endif

ifdef VOIP_BOX_NETMASK
	ln -fs $(TARGET) build/netmask
endif
ifdef VOIPBOX_CLI
	ln -fs $(TARGET) build/voipcli
endif
ifdef VOIPBOX_TONE
	ln -fs $(TARGET) build/tone
endif
ifdef VOIPBOX_LBTEST
	ln -fs $(TARGET) build/lb_test
endif
ifdef VOIPBOX_DTMF_DET_CFG
	ln -fs $(TARGET) build/dtmf_det_cfg
endif

ifdef VOIPBOX_IVR
	make -C ivr_file_play/test_g711
	make -C ivr_file_play/test_g723
	make -C ivr_file_play/test_g729
endif
ifdef VOIPBOX_VOIPEVENT
	ln -fs $(TARGET) build/voip_event
endif


	#$(CC) playtone.c $(CFLAGS) --static $(LDFLAGS) -o playtone
	#$(CC) rtptalk.c $(CFLAGS) --static $(LDFLAGS) -o rtptalk

ecos-all: $(objs)
	@# make this driven by parent 
	make -C voipcli ecos-all
	$(AR) rcs $(APP_LIB) $(objs) 

ecos-shell: $(objs-shell)
	@# make this by ecos shell 
	#$(AR) rcs $(APP_LIB) $(objs-shell)

romfs:
	$(ROMFSINST)	$(TARGET)			/bin/$(TARGET)
ifdef VOIP_DBG_UTIL
	$(ROMFSINST)    1014hz_raw 	/usr/1014hz_raw
endif
ifdef VOIPBOX_DBG
	$(ROMFSINST)	-s $(TARGET)	/bin/dbg
endif
ifdef VOIPBOX_FSKGEN
	$(ROMFSINST)    -s $(TARGET)	/bin/fskgen
endif
ifdef VOIPBOX_PULSE_DIAL
	$(ROMFSINST)    -s $(TARGET)	/bin/pulse_dial
endif
ifdef VOIPBOX_CP3_MEASURE
	$(ROMFSINST)    -s $(TARGET)	/bin/cp3_measure
endif
ifdef VOIPBOX_LED
	$(ROMFSINST)    -s $(TARGET)	/bin/led
endif
ifdef VOIPBOX_PHONERECORD
	$(ROMFSINST)    -s $(TARGET)	/bin/phonerecord
endif
ifdef VOIPBOX_REG
	$(ROMFSINST)	-s $(TARGET)	/bin/reg
endif
ifdef VOIPBOX_RAM
	$(ROMFSINST)	-s $(TARGET)	/bin/ram
endif
ifdef VOIPBOX_SLIC_RESET
	$(ROMFSINST)	-s $(TARGET)	/bin/slic_reset
endif
ifdef VOIPBOX_SWITCHMII
	$(ROMFSINST)    -s $(TARGET)	/bin/switchmii
endif
ifdef VOIPBOX_TEST
	$(ROMFSINST)    -s $(TARGET)	/bin/crash
endif
ifdef VOIPBOX_VMWIGEN
	$(ROMFSINST)    -s $(TARGET)	/bin/vmwigen
endif
ifdef VOIPBOX_VOICERECORD
	$(ROMFSINST)    -s $(TARGET)	/bin/voicerecord
endif
ifdef VOIPBOX_VOICEPLAY
	$(ROMFSINST)    -s $(TARGET)	/bin/voiceplay
endif
	#$(ROMFSINST)	itu_speech.snd	/etc/itu_speech.snd
ifdef VOIPBOX_EC_TEST
	$(ROMFSINST)	-s $(TARGET)	/bin/ec_test
endif
ifdef VOIPBOX_RTCP_STATISTIC
	$(ROMFSINST)    -s $(TARGET)	/bin/rtcp_statistic
endif
ifdef VOIPBOX_RTCP_LOGGER
	$(ROMFSINST)    -s $(TARGET)	/bin/rtcp_logger
endif
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
	$(ROMFSINST)    -s $(TARGET)	/bin/iphone_test
endif
ifdef VOIPBOX_RING_TEST
	$(ROMFSINST)    -s $(TARGET)	/bin/ring_test
endif
ifdef VOIPBOX_CLONE_MAC
	$(ROMFSINST)    -s $(TARGET)	/bin/clone_mac
endif

ifdef VOIPBOX_BANDWIDTH_MGR
	$(ROMFSINST)    -s $(TARGET)	/bin/bandwidth_mgr
endif

ifdef VOIPBOX_GPIO
	$(ROMFSINST)    -s $(TARGET)	/bin/gpio_init
	$(ROMFSINST)    -s $(TARGET)	/bin/gpio_read
	$(ROMFSINST)    -s $(TARGET)	/bin/gpio_write
endif

ifdef VOIPBOX_SEND_2833
	$(ROMFSINST)    -s $(TARGET)	/bin/send_2833
endif

ifdef VOIPBOX_POWER
	$(ROMFSINST)    -s $(TARGET)   /bin/power
endif

ifdef VOIPBOX_NETMASK
	$(ROMFSINST)    -s $(TARGET)	/bin/netmask
endif
ifdef VOIPBOX_CLI
	$(ROMFSINST)    -s $(TARGET)   /bin/voipcli
endif
ifdef VOIPBOX_TONE
	$(ROMFSINST)	-s $(TARGET)	/bin/tone
endif
ifeq ($(KILLHUB),1)
	@echo -e "\033[31;1mbuild rpcapd\033[0m"
	$(ROMFSINST)	rpcapd_8954c	/bin/rpcapd
endif
	#$(ROMFSINST)	rpcapd_8672_1.3.6-5181-2.6.30 /bin/rpcapd
	#$(ROMFSINST)    playtone      /bin/playtone
	#$(ROMFSINST)    rtptalk      /bin/rtptalk
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	#$(ROMFSINST)	dspg_cmbs_app/dspg_cmbs_tcx	/bin/cmbs_tcx #TODO should be enabled in MP tool
	#$(ROMFSINST)	dspg_cmbs_app/dspg_fw_flow_115200.bin	/bin/cmbs_fw_flow_115200.bin
endif
ifdef VOIPBOX_IVR
	make -C ivr_file_play/test_g711 romfs
	make -C ivr_file_play/test_g723 romfs
	make -C ivr_file_play/test_g729 romfs
endif
ifdef VOIPBOX_LBTEST
	$(ROMFSINST)	-s $(TARGET)	/bin/lb_test
endif
ifdef VOIPBOX_VOIPEVENT
	$(ROMFSINST)	-s $(TARGET)	/bin/voip_event
endif
ifdef VOIPBOX_DTMF_DET_CFG
	$(ROMFSINST)	-s $(TARGET)	/bin/dtmf_det_cfg
endif

clean:
	make -C pcm-talk clean
ifdef VOIPBOX_CLI
	make -C voipcli clean
endif
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	make -C dspg_cmbs_app clean
endif
	rm -f *.o a.out $(TARGET)
	rm -rf build

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
