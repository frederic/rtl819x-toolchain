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


prefix		= /usr/local
exec_prefix	= ${prefix}
sbindir		= ${exec_prefix}/sbin
libdir		= ${exec_prefix}/lib
srcdir		= .
top_srcdir	= ../..
builddir	= .
top_builddir	= ../../

INSTALL		= /usr/bin/install -c
CC		= mips-uclibc-gcc
CFLAGS		= -I$(top_builddir) -I$(top_srcdir)/include -Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs -g -O2 -fPIC
LIBERROR	= $(top_builddir)/libexception_handle/libexception_handle.a

OBJECTS		:= dyndns.o tzo.o noip.o
PLUGINS_ROOT	:= $(OBJECTS:.o= )

UNAME:=$(shell uname -s)

all: plugins

plugins: $(OBJECTS) $(PLUGINS_ROOT:%=%)
%: %.o
	$(CC) $(LDFLAGS) $< \
		$(LIBERROR)

install:
	$(INSTALL) -d $(DESTDIR)$(libdir)/updatedd
	$(INSTALL) -m 755 $(PLUGINS) $(DESTDIR)$(libdir)/updatedd

uninstall:
	$(RM) -r $(libdir)/updatedd

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PLUGINS)

distclean: clean
	$(RM) Makefile

dyndns.o:	dyndns.h
tzo.o:		tzo.h
noip.o:		noip.h

.PHONY: all plugins install uninstall clean distclean
