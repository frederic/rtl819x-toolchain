obj-y := rand_source.o prng.o ctr_prng.o
#obj-y += rand_linux_kernel.o

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include

include $(DIR_VOIP)/rules.mak
