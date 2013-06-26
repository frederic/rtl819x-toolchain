# $Id: Makefile.in,v 1.59.2.15 2005/02/22 03:02:40 jnelson Exp $

ifeq ($(wildcard $(DIR_LINUX)/.config),)
include ../../../linux-2.6.30/.config
else
include $(DIR_LINUX)/.config
endif

ifeq ($(wildcard $(DIR_USERS)/.config),)
include ../../.config
else
include $(DIR_USERS)/.config
endif

ifdef CONFIG_RTL_8198_AP_ROOT
AP=1
SKUTAG=ap
else
GW=1
SKUTAG=gw
endif

#------------------------------------------------------------------------------
ifndef COMMON_CFLAGS
$(error COMMON_CFLAGS is not defined)
endif

ifndef APMIB_SHARED
$(error APMIB_SHARED is not defined)
endif

ifndef APMIB_LIB
$(error APMIB_LIB is not defined)
endif

#------------------------------------------------------------------------------
.SUFFIXES:
.SUFFIXES: .o .c
.PHONY: clean mrclean distclean all dist depend

# The following gcc warning switches are too noisy to be useful checking
# Boa for lint:
#  -Wtraditional -Wconversion -Wredundant-decls -Wunreachable-code
# The following gcc warning switches should generate no warnings:
GCC_FLAGS = -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual \
  -Wshadow -Waggregate-return -Wmissing-prototypes -Wnested-externs \
  -Wall -W -Wno-unused -Winline -Wwrite-strings -Wundef -pedantic


#srcdir = .
#VPATH = ./../extras
LDFLAGS =  -g
#LIBS = 
CFLAGS = -Os -pipe
CPPFLAGS =  -I. -I.
DEPEND = ./.depend

#------------------------------------------------------------------------------
APMIB_DIR=../apmib

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =

#------------------------------------------------------------------------------
# Files
#------------------------------------------------------------------------------
REALTEK_FILES = fmtcpip.c fmfwall.c utility.c fmget.c fmwlan.c fmmgmt.c version.c
ifeq ($(GW),1)
REALTEK_FILES += fmddns.c fmroute.c fmipv6.c fmbt.c fmpocketap.c
endif

ifeq ($(CONFIG_HTTP_FILE_SERVER_SUPPORT),y)
#REALTEK_FILES += http_files.c
#REALTEK_FILES += fmhttp_files.c
endif

ifeq ($(CONFIG_DOMAIN_NAME_QUERY_SUPPORT),y)
REALTEK_FILES += fmdomain_query.c
endif

ifeq ($(CONFIG_RTL_P2P_SUPPORT),y)
REALTEK_FILES += fmdomain_query.c
endif

ifeq ($(CONFIG_RTL_ULINKER),y)
REALTEK_FILES += fmdomain_query.c fmulinker.c
endif

ifeq ($(CONFIG_APP_TR069),y)
REALTEK_FILES += fmtr069.c
endif

ifeq ($(CONFIG_APP_RTK_VOIP),y)
include $(VOIP_APP)/web/voip_var.mak
REALTEK_FILES += $(VOIP_FILES)
CFLAGS += $(VOIP_CFLAGS)
LDFLAGS += $(VOIP_LDFLAGS)
CFLAGS += -DCONFIG_APP_BOA
endif

#------------------------------------------------------------------------------
# CFLAGS
#------------------------------------------------------------------------------
DEBUG	= -g -Wall
IFLAGS 	= -I. -I$(APMIB_DIR)
CFLAGS += $(COMMON_CFLAGS) $(EXTRA_CFLAGS)
#include $(DIR_USERS)/.config
ifdef	CONFIG_APP_MULTIPPPOE
		CFLAGS += -D MULTI_PPPOE
endif

SOURCES = alias.c boa.c buffer.c cgi.c cgi_header.c config.c escape.c \
	get.c hash.c ip.c log.c mmap_cache.c pipe.c queue.c range.c \
	read.c request.c response.c signals.c util.c sublog.c \
	select.c \
	asp_page.c \
	$(REALTEK_FILES)

OBJS = $(SOURCES:.c=.o) timestamp.o 
ifeq ($(CONFIG_HTTP_FILE_SERVER_SUPPORT),y)
OBJS += http_files.o fmhttp_files.o
endif
#------------------------------------------------------------------------------
# targets
#------------------------------------------------------------------------------
all: fileserver  depend boa 

#boa:	$(OBJS) 
#	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) 
#	
#boa_indexer:	index_dir.o escape.o   
#	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

boa: Makefile $(OBJS) $(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o boa $(OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) boa core *~ boa_indexer
	
distclean:	mrclean

mrclean:	clean
	rm -f config.status config.cache config.h Makefile config.log

# timestamp

timestamp.o:	$(SOURCES)

#------------------------------------------------------------------------------
# depend stuff
#------------------------------------------------------------------------------
#depend: $(SOURCES)
#	$(CPP) $(CPPFLAGS) -MM $^ select.c poll.c access.c > $(DEPEND)
fileserver:
ifeq ($(CONFIG_HTTP_FILE_SERVER_SUPPORT),y)
ifeq ($(CONFIG_RTL_8198), y)
	if [ ! -e http_files.o ]; then \
		cp http_files_98 http_files.o;\
		cp fmhttp_files_98 fmhttp_files.o;\
	fi
endif
ifeq ($(CONFIG_RTL_8196C),y)
	if [ ! -e http_files.o ]; then \
		cp http_files_4181 http_files.o;\
		cp fmhttp_files_4181 fmhttp_files.o;\
	fi
endif
ifeq ($(CONFIG_RTL_8196E),y)
	if [ ! -e http_files.o ]; then \
		cp http_files_4181 http_files.o;\
		cp fmhttp_files_4181 fmhttp_files.o;\
	fi
endif
ifeq ($(CONFIG_RTL_819XD),y)
	if [ ! -e http_files.o ]; then \
		cp http_files_9xD http_files.o;\
		cp fmhttp_files_9xD fmhttp_files.o;\
	fi
endif
endif

depend: $(SOURCES)
	if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(DEBUG) $(CFLAGS) $(IFLAGS) -MM $^ > $(DEPEND); \
	fi
	
-include $(DEPEND)


# tags
tags:	$(SOURCES)
	ctags -o tags $^ *.h

release:
	./mksvnver DIR_USERS=$(DIR_USERS) DIR_LINUX=$(DIR_LINUX)

# object dump
boa.objdump:    boa
	objdump --disassemble-all --source boa > $@

.c.o:
	${CC} -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<
