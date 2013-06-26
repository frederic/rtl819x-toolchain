# Source Makefile for ecmh - Jeroen Massar <jeroen@unfix.org>
#
# ECMH_VERSION needs to be defined, gets done by toplevel Makefile

BINS	= ecmh
SRCS	= ecmh.c linklist.c common.c interfaces.c groups.c grpint.c subscr.c
INCS	= ecmh.h linklist.h common.h interfaces.h groups.h grpint.h subscr.h mld.h
OBJS	= ecmh.o linklist.o common.o interfaces.o groups.o grpint.o subscr.o
FLAGS	= -W -Wall -Wno-unused -D_GNU_SOURCE -O9

all:	$(BINS)

ecmh:	$(OBJS) ${INCS}
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS);
	
romfs:
	$(ROMFSINST) /bin/ecmh

clean:
	$(RM) -f $(OBJS) $(BINS)

