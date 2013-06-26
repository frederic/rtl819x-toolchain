CC = rsdk-linux-gcc
CFLAGS := -Wall -Os $(HARD_FLOAT) $(CFLAGS)

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
endif

ifeq ($(GUEST_ZONE),1)
CFLAGS  += -DGUEST_ZONE
endif

all:			libbridge.a

clean:
			rcsclean *
			rm -f *~
			rm -f *.o
			rm -f libbridge.a

libbridge.a:		libbridge_compat.o libbridge_devif.o libbridge_if.o libbridge_init.o libbridge_misc.o
			ar rcs libbridge.a libbridge_compat.o libbridge_devif.o libbridge_if.o libbridge_init.o libbridge_misc.o
			ranlib libbridge.a

libbridge_compat.o:	libbridge_compat.c if_index.c
			$(CC) $(CFLAGS) -c libbridge_compat.c

libbridge_devif.o:	libbridge_devif.c libbridge.h libbridge_private.h
			$(CC) $(CFLAGS) -c libbridge_devif.c

libbridge_if.o:		libbridge_if.c libbridge.h libbridge_private.h
			$(CC) $(CFLAGS) -c libbridge_if.c

libbridge_init.o:	libbridge_init.c libbridge.h libbridge_private.h
			$(CC) $(CFLAGS) -c libbridge_init.c

libbridge_misc.o:	libbridge_misc.c libbridge.h libbridge_private.h
			$(CC) $(CFLAGS) -c libbridge_misc.c
