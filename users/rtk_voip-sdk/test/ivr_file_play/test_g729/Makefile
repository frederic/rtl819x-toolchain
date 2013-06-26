CFLAGS = -I../../../includes/ -I../../../ui/include/
CFLAGS += -DCONFIG_RTK_VOIP_IVR

LDFLAGS += -L../../../voip_manager/ -lvoip_manager
LDFLAGS += -ldl -lpthread

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_g729.o test_g729.c

romfs:
	$(ROMFSINST) test_g729.o /bin/test_ivr_g729
	$(ROMFSINST) 729_raw /usr/

nfs:
	cp -f test_g729.o /work/nfs/ivr/
	cp -f 729_raw /work/nfs/ivr/

clean:
	rm -rf *.o
