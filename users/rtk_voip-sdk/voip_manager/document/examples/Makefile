include ../../../.linux_config

CFLAGS += -g -I../../../includes
#LDFLAGS += --static
#LIBS = ../../../voip_manager/libvoip_manager.a
LDFLAGS += -L../../../voip_manager -lvoip_manager

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS += -I../../../ui/include
endif

ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
LDFLAGS += -lpthread -ldl
endif

CFLAGS += $(ECOS_SHELL_CFLAGS)

CFLAGS += -Werror

#----------------------------------------------------------------------
all:
	$(CC) ring.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o ring
	$(CC) phone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o phone
	$(CC) rtp.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o rtp
	$(CC) daa.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o daa
	$(CC) caller_id.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o caller_id
	$(CC) vmwi.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o vmwi
	$(CC) fxo.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo
	$(CC) conference.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o conference
	$(CC) custom_tone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o custom_tone	
	$(CC) fxs.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxs
	$(CC) pulse_dial.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pulse_dial
	$(CC) pcm_dump.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o pcm_dump
	$(CC) gain_test.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o gain_test
	$(CC) sip_reg_led.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o sip_reg_led
	$(CC) fxo_callerid_det.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o fxo_callerid_det
	$(CC) dis_tone.c $(CFLAGS) $(LIBS) $(LDFLAGS) -o dis_tone

ecos-shell:
	$(CC) rtp.c $(CFLAGS) -c -o rtp.o 

romfs:
	$(ROMFSINST) fxs /bin/fxs
	$(ROMFSINST) rtp /bin/rtp 
	$(ROMFSINST) pcm_dump /bin/pcm_dump
	$(ROMFSINST) caller_id /bin/

clean:
	rm -f *.o ring phone rtp daa caller_id vmwi fxo conference fxs custom_tone pulse_dial pcm_dump gain_test sip_reg_led fxo_callerid_det dis_tone
