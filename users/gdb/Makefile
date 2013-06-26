#
# Realtek Semidonductor Corp.
#

CC = rsdk-linux-gcc
LD = rsdk-linux-gcc
PWD = $(shell pwd)
all:
	cd gdb-6.8; \
	make; make install;  
	cd gdb-6.8/gdb/gdbserver; \
	make; make install;

romfs:
	cp ./gdbserver-6.8/bin/mips-linux-gdbserver ./gdbserver;
	$(ROMFSINST) /bin/gdbserver;

clean:
	cd ./gdb-6.8; make clean; \
	cd ./gdb/gdbserver; make clean;
