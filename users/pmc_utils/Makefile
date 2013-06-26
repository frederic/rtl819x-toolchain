#ZLIB_FLAG=CFLAGS=-I$(DIR_USERS)/zlib-1.2.5/ APPFLAGS=-L$(DIR_USERS)/zlib-1.2.5/

all: app drv

app:
	TOOLPREFIX=$(CROSS_COMPILE) $(ZLIB_FLAG) make -C dallas_app

drv:
	TOOLPREFIX=$(CROSS_COMPILE) KERNELPATH=$(DIR_LINUX) make -C dallas_drv
clean:
	make -C dallas_app clean
	make -C dallas_drv clean
romfs:
	$(ROMFSINST) dallas_app/dallas_ctrl /bin/dallas_ctrl
	$(ROMFSINST) dallas_drv/dallas_ctrl.ko /etc/dallas_ctrl.ko
	$(ROMFSINST) dallas_lib/dallas.bin /etc/dallas.bin
	#$(ROMFSINST) dallas_lib/isr.bin /etc/isr.bin
	$(ROMFSINST) buildroot/gpon-nvdb.conf /etc/gpon-nvdb.conf
	#$(ROMFSINST) libz.so.1	/lib/libz.so.1
ifneq ($(RCSCONFIG),)
	make rcsconf
endif

rcsconf:
	echo "CONFIG_APP_PMC_UTILS=y" >> $(RCSCONFIG)


