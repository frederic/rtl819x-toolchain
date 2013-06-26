obj-y := datatypes.o
#obj-y += gf2_8.o math.o stat.o

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include

include $(DIR_VOIP)/rules.mak
