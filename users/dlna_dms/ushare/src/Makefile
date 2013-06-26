ifeq (,$(wildcard ../config.mak))
$(error "../config.mak is not present, run configure !")
endif
include ../config.mak

OBJAVI= ./spca/grab_avi.o ./spca/spcav4l.o ./spca/utils.o ./spca/tcputils.o ./spca/picture.o ./spca/pargpio.o ./spca/encoder.o ./spca/huffman.o ./spca/marker.o ./spca/quant.o ./spca/avilib.o
LDFLAGS += -L$(DIR_USERS)/libiconv-1.14/lib/.libs

PROG = ushare

EXTRADIST = ushare.1 \
	cds.h \
	cms.h \
	msr.h \
	http.h \
	presentation.h \
	metadata.h \
	mime.h \
	services.h \
	buffer.h \
	content.h \
	cfgparser.h \
	trace.h \
	redblack.h \
	osdep.h \
	ctrl_telnet.h \
	ushare.h \
	gettext.h \
	minmax.h \
	util_iconv.h \
	iconv.h \
	ifaddrs.h 
#iutil_iconv.h \

SRCS = \
	cds.c \
	cms.c \
	msr.c \
	http.c \
	presentation.c \
	metadata.c \
	mime.c \
	services.c \
	buffer.c \
	content.c \
	cfgparser.c \
	trace.c \
	redblack.c \
	osdep.c \
	ctrl_telnet.c \
	ushare.c \
	util_iconv.c \
	ifaddrs.c \
	#iconv.c

OBJS = $(SRCS:.c=.o)

.SUFFIXES: .c .o

all: depend spcacat $(PROG)

spcacat: 
	$(MAKE) -C spca all
.c.o:
	$(CC) -c $(CFLAGS) $(OPTFLAGS) -o $@ $<

	#ar src myifaddrs.a ifaddrs.o
$(PROG): $(OBJS)
	$(CC) -o $@ -static -Wall -Wl,--start-group $(OBJAVI) $(OBJS)  $(LDFLAGS) $(EXTRALIBS) -I$(DIR_RSDK)/config/uclibc/include -liconv -lc -ldl -lm -W1,--end-group

#	$(CC) ar crv myifaddrs.a ifaddrs.o
#-I/home/wenjian_jia/jungle/toolchain/rsdk-1.3.6-4181-EB-2.6.30-0.9.30/config/uclibc/include
#-L/home/wenjian_jia/jungle/toolchain/rsdk-1.3.6-4181-EB-2.6.30-0.9.30/libiconv -liconv -ldl -lm -Wl,--end-group
clean:
	-$(RM) -f *.o $(PROG)
	-$(RM) -f .depend
	-$(RM) -f ./spca/*.o

distclean:

install: $(PROG)
	$(INSTALL) -d $(bindir)
	$(INSTALL) $(PROG) $(bindir)
	$(STRIP) $(INSTALLSTRIP) $(bindir)/$(PROG)

depend:
	$(CC) -I.. -MM $(CFLAGS) $(SRCS) 1>.depend

.PHONY: clean distclean install depend

dist-all:
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist-all

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
