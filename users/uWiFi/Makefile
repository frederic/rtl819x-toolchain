
ifeq ($(DIR_LINUX),)
include ../../linux-2.6.30/.config
else
include $(DIR_LINUX)/.config
endif

RSDK_LIB_DIR=$(shell  echo $(PATH) | cut -f1 -d:)

all:
	@echo "builded"
clean:
	@echo "It's cleaned"

	
romfs:
	rm -f $(RSDK_LIB_DIR)/../lib/libuWiFi.so.0
	rm -f $(RSDK_LIB_DIR)/../lib/libuWiFi.so
	cp ./lib/libuWiFi.so.0.0.1 $(RSDK_LIB_DIR)/../lib/libuWiFi.so.0.0.1
	cp ./lib/libuWiFi.a $(RSDK_LIB_DIR)/../lib/libuWiFi.a
	ln -s libuWiFi.so.0.0.1 $(RSDK_LIB_DIR)/../lib/libuWiFi.so.0
	ln -s libuWiFi.so.0.0.1 $(RSDK_LIB_DIR)/../lib/libuWiFi.so
	$(ROMFSINST) ./cmd/uWiFid_4181 /bin/uWiFid


