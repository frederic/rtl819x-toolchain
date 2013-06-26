obj-y := aes.o aes_icm.o aes_cbc.o cipher.o null_cipher.o 

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include

include $(DIR_VOIP)/rules.mak
