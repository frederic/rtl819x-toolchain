obj-y = aipc_data_tocpu_mbox_recv.o aipc_data_tocpu_recv.o aipc_data_todsp_mbox_send.o aipc_data_todsp_send.o 

EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc/coprocessor_dsp/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc/coprocessor_dsp/cpu/include/

EXTRA_CFLAGS += -DMODULE_NAME=\"IPC\"

include $(DIR_VOIP)/rules.mak

