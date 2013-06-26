#include $(DIR_LINUX)/.config

all : $(EXEC)
	make -C hcd
	make -C wlan_app
clean:
	make -C hcd clean
	make -C wlan_app clean

romfs:
	make -C hcd romfs
	make -C wlan_app romfs
	$(ROMFSINST) script/ /bin
