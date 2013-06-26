WSCSRC = src
 
all:
	@echo "builded"
clean:
	@echo "It's cleaned"
romfs:
	$(ROMFSINST) $(WSCSRC)/wscd_$(TOOLCHAIN) /bin/wscd
	$(ROMFSINST) $(WSCSRC)/wscd.conf /etc/wscd.conf
