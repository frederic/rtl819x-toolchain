obj-y = aipc_data_tocpu_mbox_send.o aipc_data_tocpu_send.o aipc_data_todsp_mbox_recv.o aipc_data_todsp_recv.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc/coprocessor_dsp/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc/coprocessor_dsp/dsp/include/

EXTRA_CFLAGS += -DMODULE_NAME=\"IPC\"

include $(DIR_VOIP)/rules.mak

