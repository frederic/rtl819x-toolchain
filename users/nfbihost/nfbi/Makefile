CC		= mips-linux-gcc
COPT		= -O2 -Wall
CFLAGS		= ${COPT}
LDFLAGS		=  --static
#STRIP		= mips-uclibc-strip

all: nfbi hcd

clean:
	rm -f nfbi hcd tags *.o 

nfbi_sources = nfbi_main.c nfbi_api.c
hcd_sources = hcd_main.c nfbi_api.c

nfbi:	$(nfbi_sources:.c=.o)
	$(CC) -s $(LDFLAGS) $(LIBS) $^ -o $@	

hcd:	$(hcd_sources:.c=.o)
	$(CC) -s $(LDFLAGS) $(LIBS) $^ -o $@	

romfs:
	$(ROMFSINST)	nfbi	/bin/nfbi
	$(ROMFSINST)	hcd	/bin/hcd
	$(ROMFSINST)	../rtl8198/boot-ddr1.bin	/usr/
	$(ROMFSINST)	../rtl8198/boot-ddr2.bin	/usr/
	$(ROMFSINST)	../rtl8198/linux.bin		/usr/
