# -- updatedd: Makefile.in --
#
# Copyright (C) 2004, 2005 Philipp Benner
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

SHELL	= /bin/sh -e

prefix	= /usr/local
datadir	= ${prefix}/share
srcdir	= .

INSTALL	= /usr/bin/install -c
SCRIPTS	= ipserv.pl interface.pl

all:

clean:
	$(RM) $(SCRIPTS)

distclean:
	$(RM) Makefile

install:
	$(INSTALL) -d $(DESTDIR)$(datadir)/updatedd
	$(INSTALL) $(SCRIPTS) $(DESTDIR)$(datadir)/updatedd

uninstall:
	$(RM) -r $(datadir)/updatedd
