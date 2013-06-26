CC = rsdk-linux-gcc
CXX = rsdk-linux-g++
LD = rsdk-linux-gcc
CFLAGS=-I../../linux-2.6.30/include -Os -DCONFIG_DEFAULTS_KERNEL_2_6 -D__BIG_ENDIAN_BITFIELD -DCONFIG_CPU_BIG_ENDIAN

OBJECTS = mkdosfs.o

all: mkdosfs

mkdosfs: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

install: mkdosfs
	mkdir -p $(SBINDIR) $(MANDIR)
	install -m 755 mkdosfs $(SBINDIR)
	install -m 644 mkdosfs.8 $(MANDIR)
	rm -f $(SBINDIR)/mkfs.msdos
	rm -f $(SBINDIR)/mkfs.vfat
	ln -s mkdosfs $(SBINDIR)/mkfs.msdos
	ln -s mkdosfs $(SBINDIR)/mkfs.vfat
	rm -f $(MANDIR)/mkfs.msdos.8
	ln -s mkdosfs.8 $(MANDIR)/mkfs.msdos.8
	ln -s mkdosfs.8 $(MANDIR)/mkfs.vfat.8

clean :
	echo *.o *.i *.s *~ \#*# core .#* .new*
	rm -f *.o *.i *.s *~ \#*# core .#* .new*
	rm -f mkdosfs

distclean: clean
	rm -f mkdosfs *.a *# *.orig *.rej TAGS

romfs:
	$(ROMFSINST) /bin/mkdosfs
