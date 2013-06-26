all:
	@echo "builded"
clean:
	@echo "It's cleaned"

romfs:
	$(ROMFSINST) lld2d_$(TOOLCHAIN) /bin/lld2d
	$(ROMFSINST) ../src/realsil_gw.ico /etc/realsil_gw.ico
	$(ROMFSINST) ../src/realsil_gw_small.ico /etc/icon.ico
	$(ROMFSINST) ../src/lld2d.conf /etc/lld2d.conf
