#for test only
#CC = mips-uclibc-gcc
#CFLAGS = -O2  -fomit-frame-pointer   -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -fpic -fPIC -embedded-relocs -mno-abicalls -I/home/casey/uClinux-dist/lib/uClibc-0.9.15/include -I/home/casey/uClinux-dist/lib/libm -I/home/casey/uClinux-dist/lib/libcrypt_old -I/home/casey/uClinux-dist -fno-builtin -nostartfiles -I/home/casey/uClinux-dist/linux-2.4.x/include
##

#CC = gcc
#CC = rsdk-uclibc-gcc 
#LD = gcc
#LD = rsdk-uclibc-gcc -s
#LD = mips-uclibc-gcc 
CFLAGS := -pipe -Wall -Wno-unused -Os -g $(CFLAGS)
LDFLAGS = -s


OBJS=	af.o if.o input.o main.o output.o startup.o \
	tables.o timer.o trace.o inet.o

CFLAGS += -I$(ROOTDIR)/linux/include $(INCNET) -I. -DRIPVERSION=2

all: BUILT_TIME routed

routed: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) 

romfs:
	$(ROMFSINST) ./routed /bin/routed

clean:
	rm -f *.elf *.gdb *.o routed

$(OBJS): defs.h trace.h interface.h table.h af.h pathnames.h version.h

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time	
