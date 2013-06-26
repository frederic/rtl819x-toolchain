obj-y    :=

obj-y += RtpPacket.o rtpTools.o RtpReceiver.o RtpTransmitter.o rtpterm.o rtp_main.o\
         RtcpReceiver.o RtcpTransmitter.o RtcpPacket.o NtpTime.o
obj-$(CONFIG_RTK_VOIP_RTCP_XR) += RtcpTransmitterXR.o RtcpReceiverXR.o RtpReceiverXR.o RtcpXR_RLE.o 
#EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/v152
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/energy
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

# rock: add module name for PRINT
EXTRA_CFLAGS += -DMODULE_NAME=\"RTP\"

include $(DIR_VOIP)/rules.mak
