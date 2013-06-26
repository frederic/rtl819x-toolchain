CFLAGS = -I../../../includes/ -I../../../ui/include/
CFLAGS += -DCONFIG_RTK_VOIP_IVR

LDFLAGS += -L../../../voip_manager/ -lvoip_manager
LDFLAGS += -ldl -lpthread

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_g723.o test_g723.c -lpthread

romfs:
	$(ROMFSINST) test_g723.o /bin/test_ivr_g723
	$(ROMFSINST) 723_raw /usr/

nfs:
	cp -f test_g723.o /work/nfs/ivr/
	cp -f 723_raw /work/nfs/ivr/

clean:
	rm -rf *.o
