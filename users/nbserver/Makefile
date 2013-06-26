###########################################################################
# Makefile for netbios client/server for unix
# Copyright Andrew Tridgell 1992,1993
###########################################################################

CFLAGS += -DLINUX

#FLAGS1 = -O2
#LIBS1 = 
##CFLAGS += -s -Wall -c -Os $(CFLAGS) -DLINUX
##LDFLAGS = -s -Wall
# If you have the gcc compiler then use it, otherwise any ansi-C compiler
#CC = gcc

# set these to where to find various files
#FLAGS2 = -DDEBUGFILE=\"/home/nimbus/tridge/server/log\"
#FLAGS3 = -DSERVICES=\"/home/nimbus/tridge/server/smb.conf\"

# set this to the name of the default account, which is the one
# to use when no username or password is specified 
#FLAGS4 = -DGUEST_ACCOUNT=\"tridge\"

# what mode mask to use when creating files and directories
#FLAGS5 = -DDEF_CREATE_MASK=0755


#####################################
# WHICH OPERATING SYSTEM?
# UNCOMMENT ONE OF THE SECTIONS BELOW
#
# The following are additional flags that may apply
#   -DNETGROUP if your machine supports yp netgroups
#   -DSHADOW_PWD if you are using shadow passwords
#   -DPWDAUTH if your library has a pwdauth() call
#   -DHAVE_LOCKF if your library has the lockf() call and you run lockd 
#   -DUSE_DIRECT if your library uses direct rather than dirent structures
#####################################

# Use this for Linux with shadow passwords
# FLAGSM = -DLINUX -DSHADOW_PWD
# LIBSM = -lshadow

# Use this for Linux without shadow passwords
# FLAGSM = -DLINUX
# LIBSM = 

# This is for SUNOS
#FLAGSM = -DSUN -DPWDAUTH -DHAVE_LOCKF
#LIBSM =   

# This is for SOLARIS
# FLAGSM = -DSOLARIS -DHAVE_LOCKF -DBSD_COMP
# LIBSM = -lsocket -lnsl

# This is for SVR4
# FLAGSM = -DSVR4 -DSHADOW_PWD -DHAVE_LOCKF
# LIBSM = -lsocket -lnsl -lc -L/usr/ucblib -lucb

# This is for ULTRIX
# FLAGSM = -DULTRIX -DUSE_DIRECT -DHAVE_LOCKF
# LIBSM =   

# This is for OSF1 (Alpha)
# FLAGSM = -DOSF1 -DHAVE_LOCKF
# LIBSM =

# This is for AIX
# FLAGSM = -DAIX -DHAVE_LOCKF
# LIBSM =   

# This is for BSDI
# FLAGSM = -DBSDI -DPWDAUTH -DUSE_DIRECT -DHAVE_LOCKF
# LIBSM =   

# This is for NetBSD
# FLAGSM = -DNETBSD -DUSE_DIRECT -DSHADOW_PWD
# LIBSM = -lcrypt 

# This is for SEQUENT. Can someone test this please?
# FLAGSM = -DSEQUENT -DPWDAUTH -DUSE_DIRECT -DHAVE_LOCKF
# LIBSM =   

# This is for HP-UX
# FLAGSM = -DHPUX -DHAVE_LOCKF -Aa -D_HPUX_SOURCE -D_POSIX_SOURCE
# LIBSM = 

# This is for SGI. Can someone test this please?
# FLAGSM = -DSGI -DPWDAUTH -DUSE_DIRECT -DHAVE_LOCKF
# LIBSM =   


#CFLAGS = $(FLAGS1) $(FLAGS2) $(FLAGS3) $(FLAGS4) $(FLAGS5) $(FLAGSM)
#LIBS = $(LIBS1) $(LIBSM)

#all: smbserver smbclient nmbserver testparm
all: nmbserver


romfs:
	$(ROMFSINST) ./nmbserver /bin/nmbserver
	
INCLUDES = local.h includes.h smb.h loadparm.h params.h

smbserver: server.o util.o loadparm.o params.o access.o
	$(CC) $(CFLAGS) -o smbserver server.o util.o loadparm.o params.o \
                        access.o $(LIBS)

nmbserver: nameserv.o util.o
	$(CC) $(LDFLAGS) -o nmbserver nameserv.o util.o

smbclient: client.o util.o
	$(CC) $(CFLAGS) -o smbclient client.o util.o $(LIBS)

testparm: testparm.o util.o loadparm.o params.o access.o
	$(CC) $(CFLAGS) -o testparm testparm.o util.o loadparm.o params.o \
                        access.o $(LIBS)

.c.o: $(INCLUDES)
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm -f *.o *~ smbserver smbclient nmbserver testparm
