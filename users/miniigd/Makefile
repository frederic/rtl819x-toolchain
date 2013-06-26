all:
	@echo "builded"
clean:
	@echo "It's cleaned"
 
romfs:
	$(ROMFSINST) miniigd_$(TOOLCHAIN) /bin/miniigd
	$(ROMFSINST) -s /var/linuxigd /etc/
	@mkdir -p $(DIR_ROMFS)/etc/tmp
	@cp ./pics* $(DIR_ROMFS)/etc/tmp
