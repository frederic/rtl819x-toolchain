CFLAGS = -I../../../includes/ -I../../../ui/include/
CFLAGS += -DCONFIG_RTK_VOIP_IVR

LDFLAGS += -L../../../voip_manager/ -lvoip_manager 
LDFLAGS += -ldl -lpthread

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_g711.o test_g711.c

romfs:
	$(ROMFSINST) test_g711.o /bin/test_ivr_g711
	$(ROMFSINST) 711_raw /usr/

nfs:
	cp -f test_g711.o /work/nfs/ivr/
	cp -f 711_raw /work/nfs/ivr/

clean:
	rm -rf *.o
