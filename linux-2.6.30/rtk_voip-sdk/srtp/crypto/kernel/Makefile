obj-y := crypto_kernel.o alloc.o key.o err.o

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include

include $(DIR_VOIP)/rules.mak
