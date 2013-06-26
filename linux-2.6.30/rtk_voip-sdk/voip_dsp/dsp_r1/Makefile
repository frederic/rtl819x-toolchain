
obj-y    :=

#CONFIG_VOIP_SDK=y

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y += r1_main.obj
obj-y += dtmf.obj
obj-y += aes.obj
obj-y += table.obj
obj-y += jbc.obj
obj-y += jbc2.obj

obj-y    += g7111.obj
obj-y    += common/common.obj
obj-y    += wsola.obj

obj-y += r1_helpG711.obj

else

obj-y += r1_main.o
obj-y += dtmf.o
obj-y += aes.o
obj-y += table.o
obj-y += jbc.o
obj-y += jbc2.o

obj-y    += g711/
obj-y    += common/
obj-y    += wsola/
obj-y    += help/

endif

subdir-y += common/util
obj-y += common/util/codec_mem_api.o
obj-y += common/util/codec_mem.o

# For SDK not including g729/7231/726/722

ifeq ($(CONFIG_RTK_VOIP_G722),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g7222.obj
obj-y    += r1_helpG722.obj
else
#obj-y    += g722/			# rtk_voip.h, undef SUPPORT_G722_ITU
obj-y   += g722_itu/
endif
endif

ifeq ($(CONFIG_RTK_VOIP_G7111),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y	+= g711.1.obj
obj-y   += r1_helpG711.1.obj
else
obj-y	+= g711.1/
endif
endif

ifeq ($(CONFIG_RTK_VOIP_G7231),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g723.obj
obj-y    += r1_helpG723.obj
else
obj-y    += g723/
endif
endif

ifeq ($(CONFIG_RTK_VOIP_G729AB),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g729.obj
obj-y    += r1_helpG729.obj
else
obj-y    += g729/
endif
endif
#obj-y	+= g7291_itu_c_codeC/

ifeq ($(CONFIG_RTK_VOIP_G726),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g726.obj
obj-y    += r1_helpG726.obj
else
obj-y    += g726/
endif
else
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += g726_api.obj
endif
endif

ifeq ($(CONFIG_RTK_VOIP_GSMFR),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += gsm_fr.obj
obj-y    += r1_helpGSMfr.obj
else
obj-y    += gsm_fr/
endif
endif

ifeq ($(CONFIG_RTK_VOIP_AMR_NB),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += amr_nb.obj
obj-y    += r1_helpAMRNB.obj
else
obj-y    += amr_nb/
endif
endif

#subdir-y += speex
#obj-y    += speex/speex_out.o
ifeq ($(CONFIG_RTK_VOIP_SPEEX_NB),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y	+= speex.obj
obj-y	+= r1_helpSPEEXNB.obj
else
obj-y    += speex/
endif
endif


ifeq ($(CONFIG_RTK_VOIP_ILBC),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += iLBC.obj
obj-y    += r1_helpiLBC.obj
else
obj-y    += iLBC/
endif
endif

ifeq ($(CONFIG_RTK_VOIP_T38),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += t38.obj
obj-y    += r1_helpT38.obj
else
obj-y    += t38/
endif
endif


ifeq ($(CONFIG_RTK_VOIP_SILENCE),y)
ifeq ($(CONFIG_VOIP_SDK),y)
obj-y    += r1_helpSilence.obj
endif
endif

#subdir-y += speex
#obj-y    += speex/speex_out.o

# tone_.o is not used now!
#obj-y += tone_.o

###### RULES

#.PHONY: all clean

#all: r1_main

#clean:
#	-rm -f r1_main.exe r1_main.bin *.o
#	-rm -f *.o
EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
ifeq ($(CONFIG_VOIP_SDK),y)
else
EXTRA_CFLAGS +=  -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/basic_op
EXTRA_CFLAGS +=  -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/util
endif
EXTRA_CFLAGS +=	-I$(DIR_VOIP)/voip_dsp/cp3

ifeq "$(CONFIG_VOIP_COP3_PROFILE)" "y"
EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
EXTRA_CFLAGS+=-DFEATURE_COP3_G711DECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G722DECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G729DECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G7231DECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G726DECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_GSMFRDECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G711ENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G722ENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G729ENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G7231ENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G726ENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_GSMFRENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_ILBC30MSENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_ILBC30MSDECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_ILBC20MSENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_ILBC20MSDECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_T38ENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_T38DECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_AMRNBENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_AMRNBDECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_SPEEX_NB_ENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_SPEEX_NB_DECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G7111NBENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G7111NBDECPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G7111WBENCPHASE
EXTRA_CFLAGS+=-DFEATURE_COP3_G7111WBDECPHASE
#EXTRA_CFLAGS+=-DFEATURE_COP3_G729JBC
#EXTRA_CFLAGS+=-DFEATURE_COP3_G729DEC
#EXTRA_CFLAGS+=-DFEATURE_COP3_PRE_PROCESS
#EXTRA_CFLAGS+=-DFEATURE_COP3_AES
#EXTRA_CFLAGS+=-DFEATURE_COP3_PLAYTONE
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(DIR_VOIP)/rules.mak
