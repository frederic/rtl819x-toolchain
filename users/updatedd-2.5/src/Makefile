# -- updatedd: Makefile.in --
#
# Copyright (C) 2002, 2003, 2004, 2005 Philipp Benner
#
# This file is part of UpdateDD - http://updatedd.philipp-benner.de.
#
# UpdateDD is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# any later version.
#
# UpdateDD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with UpdateDD; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

SHELL		= /bin/sh -e


bin		= updatedd

prefix		= /usr/local
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
libdir		= ${exec_prefix}/lib
mandir		= ${prefix}/man
srcdir		= .
top_srcdir	= ..
builddir	= .
top_builddir	= ../

INSTALL		= /usr/bin/install -c
CC		= rsdk-linux-gcc
STRIP		= rsdk-linux-strip
#CFLAGS		= -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs -O2 --static -s -DLIBPATH=\"$(libdir)/updatedd\"
CFLAGS		= -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs --static -s -DLIBPATH=\"$(libdir)/updatedd\" -Os
LIBS		= -ldl  -lnsl -lresolv 
LIBERROR	= $(top_builddir)/libexception_handle/libexception_handle.a

OBJECTS		= updatedd.o plugins/dyndns.o  plugins/tzo.o  #plugins/corede.o plugins/atnet.o  plugins/noip.o

ifeq ($(RTL865X_GRC),1)
OBJECTS += plugins/corede.o plugins/atnet.o
CFLAGS  += -DCONFIG_RTL865X_GRC
endif

UNAME:=$(shell uname -s)

ifeq (Darwin,$(UNAME))
  CFLAGS+=-D__DARWIN__
endif

all: $(bin)

$(bin): $(OBJECTS)
#	$(CC) -O2 --static -s -Wall  $(OBJECTS) $(LIBERROR) -o $(bin)  $(LIBS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -Wall  $(OBJECTS) $(LIBERROR) -o $(bin)  $(LIBS)
	$(STRIP) $@

install:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 $(bin) $(DESTDIR)$(bindir)

	$(INSTALL) -d $(DESTDIR)$(mandir)/man1
	$(INSTALL) -m 644 $(srcdir)/updatedd.1 $(DESTDIR)$(mandir)/man1

uninstall:
	$(RM) $(bindir)$(bin)
	$(RM) $(mandir)/man1/updatedd.1

clean:
	$(RM) $(OBJECTS)
	$(RM) $(bin)

distclean: clean
	$(RM) Makefile

romfs:
	$(ROMFSINST) $(bin) /bin/$(bin)

updatedd.o: updatedd.h

.PHONY: all install uninstall clean distclean
