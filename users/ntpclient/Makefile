# Under Solaris, you need to 
#    CFLAGS += -xO2 -Xc
#    LDLIBS += -lnsl -lsocket
# Some versions of Linux may need
    CFLAGS +=  -D_GNU_SOURCE
# To cross-compile
#CC = mips-uclibc-gcc
# To check for lint
# -Wundef not recognized by gcc-2.7.2.3
#CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes

CFLAGS += -Os 
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += -s

all: ntpclient

test: ntpclient
	./ntpclient -d -r <test.dat

ntpclient: ntpclient.o phaselock.o

adjtimex: adjtimex.o

romfs:
	$(ROMFSINST) /bin/ntpclient
	$(ROMFSINST) -s /var/TZ /etc/TZ

clean:
	rm -f ntpclient adjtimex *.o
