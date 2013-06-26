srcdir = .
top_srcdir = .
top_builddir = .
OBJEXT = o
#CC = gcc
CC = mips-linux-gcc
CFLAGS += -g -Os
AR = mips-uclibc-ar
#LDFLAGS = 
lib_LIBRARIES = libfl.a

libfl_a_SOURCES = \
	libmain.c \
	libyywrap.c


BUILT_SOURCES = \
	skel.c


LIBRARIES = $(lib_LIBRARIES)

libfl_a_AR = $(AR) cru
RANLIB = mips-linux-ranlib
libfl_a_LIBADD =
am_libfl_a_OBJECTS = libmain.$(OBJEXT) libyywrap.$(OBJEXT)
libfl_a_OBJECTS = $(am_libfl_a_OBJECTS)
COMPILE = $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

all: $(LIBRARIES)

libfl.a: $(libfl_a_OBJECTS) $(libfl_a_DEPENDENCIES)
	-rm -f libfl.a
	$(libfl_a_AR) libfl.a $(libfl_a_OBJECTS) $(libfl_a_LIBADD)
	$(RANLIB) libfl.a
	

skel.c: flex.skl mkskel.sh flexint.h tables_shared.h
	sed 's/m4_/m4postproc_/g; s/m4preproc_/m4_/g' $(srcdir)/flex.skl | $(m4) -P -DFLEX_MAJOR_VERSION=`echo $(VERSION)|cut -f 1 -d .` -DFLEX_MINOR_VERSION=`echo $(VERSION)|cut -f 2 -d .` -DFLEX_SUBMINOR_VERSION=`echo $(VERSION)|cut -f 3 -d .` | sed 's/m4postproc_/m4_/g' | $(SHELL) $(srcdir)/mkskel.sh  >skel.c

# Explicitly describe dependencies.
# You can recreate this with `gcc -I. -MM *.c'
buf.o: buf.c flexdef.h flexint.h
ccl.o: ccl.c flexdef.h flexint.h
dfa.o: dfa.c flexdef.h flexint.h tables.h tables_shared.h
ecs.o: ecs.c flexdef.h flexint.h
gen.o: gen.c flexdef.h flexint.h tables.h tables_shared.h
libmain.o: libmain.c
libyywrap.o: libyywrap.c
main.o: main.c flexdef.h flexint.h version.h options.h scanopt.h \
 tables.h tables_shared.h
misc.o: misc.c flexdef.h flexint.h tables.h tables_shared.h
nfa.o: nfa.c flexdef.h flexint.h
options.o: options.c options.h scanopt.h flexdef.h flexint.h
parse.o: parse.c flexdef.h flexint.h tables.h tables_shared.h
scan.o: scan.c flexdef.h flexint.h parse.h
scanopt.o: scanopt.c flexdef.h flexint.h scanopt.h
skel.o: skel.c flexdef.h flexint.h
sym.o: sym.c flexdef.h flexint.h
tables.o: tables.c flexdef.h flexint.h tables.h tables_shared.h
tables_shared.o: tables_shared.c flexdef.h flexint.h tables.h \
 tables_shared.h
tblcmp.o: tblcmp.c flexdef.h flexint.h
yylex.o: yylex.c flexdef.h flexint.h parse.h
filter.o: filter.c flexdef.h flexint.h

clean:
	rm -f *.o 
