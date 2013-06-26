# Makefile.in generated automatically by automake 1.4-p6 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..

prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/dnrd
pkglibdir = $(libdir)/dnrd
pkgincludedir = $(includedir)/dnrd

top_builddir = ..

ACLOCAL = aclocal-1.4
AUTOCONF = autoconf
AUTOMAKE = automake-1.4
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = 
build_triplet = i686-pc-linux-gnu
host_alias = 
host_triplet = i686-pc-linux-gnu
target_alias = 
target_triplet = i686-pc-linux-gnu
CC = rsdk-linux-gcc
STRIP = rsdk-linux-strip
MAKEINFO = makeinfo
PACKAGE = dnrd
THREAD_CFLAGS = 
#THREAD_LIBS = -lpthread
THREAD_LIBS = 
VERSION = 2.12.1

sbin_PROGRAMS = dnrd
dnrd_SOURCES = args.c args.h cache.c cache.h check.c check.h common.c common.h dns.c dns.h lib.c lib.h main.c master.c master.h query.c query.h relay.c relay.h sig.c sig.h tcp.c tcp.h udp.c udp.h
dnrd_SOURCES += regexp.c regexp.h regmagic.h
#dnrd_LDADD = -lpthread
dnrd_LDADD =
INCLUDES = 
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = config.h
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(sbin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I.
CPPFLAGS = 
LDFLAGS = 
LIBS = 
dnrd_OBJECTS =  args.o cache.o check.o common.o dns.o lib.o main.o \
master.o query.o relay.o sig.o tcp.o udp.o
dnrd_OBJECTS += regexp.o
dnrd_DEPENDENCIES = 
dnrd_LDFLAGS = 

CFLAGS := -g $(CFLAGS) -Os
CFLAGS += $(EXTRA_CFLAGS)

include ../../../linux-2.6.30/.config


ifeq ($(CONFIG_DOMAIN_NAME_QUERY_SUPPORT),y)
CFLAGS += -DCONFIG_DOMAIN_NAME_QUERY_SUPPORT
endif

ifeq ($(CONFIG_RTL_ULINKER),y)
CFLAGS += -DCONFIG_DOMAIN_NAME_QUERY_SUPPORT
endif
	
ifeq ($(RTL8186_TR),1)
CFLAGS+=-DCONFIG_RTL8186_TR 
endif

ifeq ($(CONFIG_RTL865X_SC),1)
CFLAGS+=-DCONFIG_RTL865X_SC 
endif

ifeq ($(GR),1)  
CFLAGS+=-DCONFIG_RTL8186_GR
endif

COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DIST_COMMON =  


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
SOURCES = $(dnrd_SOURCES)
OBJECTS = $(dnrd_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
.c.o:
	$(COMPILE) -c $<

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f dnrd *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

dnrd: $(dnrd_OBJECTS) $(dnrd_DEPENDENCIES)
	@rm -f dnrd
	$(LINK) $(dnrd_LDFLAGS) $(dnrd_OBJECTS) $(dnrd_LDADD) $(LIBS)
	$(STRIP) $@ 

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES) config.h.in $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)config.h.in$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags config.h.in $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = src

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
args.o: args.c args.h common.h lib.h cache.h
cache.o: cache.c common.h dns.h cache.h lib.h
check.o: check.c check.h
common.o: common.c config.h common.h lib.h
dns.o: dns.c dns.h lib.h common.h
lib.o: lib.c config.h lib.h common.h
main.o: main.c relay.h cache.h common.h args.h sig.h master.h
master.o: master.c common.h dns.h master.h lib.h
query.o: query.c query.h common.h
relay.o: relay.c query.h relay.h cache.h common.h tcp.h udp.h master.h \
	dns.h
sig.o: sig.c sig.h common.h
tcp.o: tcp.c common.h relay.h cache.h
udp.o: udp.c common.h relay.h cache.h query.h master.h

info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
all-recursive-am: config.h
	$(MAKE) $(AM_MAKEFLAGS) all-recursive

install-exec-am: install-sbinPROGRAMS
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-sbinPROGRAMS
uninstall: uninstall-am
all-am: Makefile $(PROGRAMS) config.h
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(sbindir)

romfs:
	if [ "$(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)" != "n" -a "$(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)" != "" ]; then \
		rm -f $(DIR_ROMFS)/etc/hosts; \
		$(ROMFSINST) -s /var/hosts /etc/hosts; \
	fi
	if [ "$(CONFIG_RTL_ULINKER)" != "n" -a "$(CONFIG_RTL_ULINKER)" != "" ]; then \
		rm -f $(DIR_ROMFS)/etc/hosts; \
		$(ROMFSINST) -s /var/hosts /etc/hosts; \
	fi
	$(ROMFSINST) dnrd /bin/dnrd
	$(ROMFSINST) -s /var/dnrd /etc/dnrd
	$(ROMFSINST) -s /var/resolv.conf /etc/resolv.conf

mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-hdr mostlyclean-sbinPROGRAMS \
		mostlyclean-compile mostlyclean-tags \
		mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-hdr clean-sbinPROGRAMS clean-compile clean-tags \
		clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-hdr distclean-sbinPROGRAMS distclean-compile \
		distclean-tags distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-hdr maintainer-clean-sbinPROGRAMS \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-hdr distclean-hdr clean-hdr maintainer-clean-hdr \
mostlyclean-sbinPROGRAMS distclean-sbinPROGRAMS clean-sbinPROGRAMS \
maintainer-clean-sbinPROGRAMS uninstall-sbinPROGRAMS \
install-sbinPROGRAMS mostlyclean-compile distclean-compile \
clean-compile maintainer-clean-compile tags mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck all-recursive-am \
install-exec-am install-exec install-data-am install-data install-am \
install uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
