all:	site_survey

#CC	= mips-uclibc-gcc
CC = rsdk-linux-gcc
#STRIP	= mips-uclibc-strip
STRIP	= rsdk-linux-strip

CFLAGS	=
IFLAGS	=
LDFLAGS = --static

DEBUG	= -Wall -Os -DDEBUG
#CFLAGS += $(DEBUG)

site_survey : Makefile site_survey.o
	$(CC) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $(LDFLAGS) site_survey.o
	$(STRIP) $@

clean:
	rm -f *.o site_survey

romfs:
	$(ROMFSINST) /bin/site_survey
