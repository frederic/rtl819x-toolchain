obj-y = voip_info_proc.o ethernet_dsp_proc.o cpu_info_proc.o voip_counter_proc.o
obj-y += generic.o 
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_manager
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc 
EXTRA_CFLAGS += -I$(DIR_VOIP)/include

include $(DIR_VOIP)/rules.mak
