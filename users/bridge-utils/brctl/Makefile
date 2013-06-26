CFLAGS := -Wall -c -g -Os $(HARD_FLOAT) $(CFLAGS)
LDFLAGS = -s -Wall -g 
CC = rsdk-linux-gcc

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += --static -Wl,--gc-sections
endif

ifeq ($(GUEST_ZONE),1)
CFLAGS  += -DGUEST_ZONE
endif

all:			brctl brctld

brctl:			brctl.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a
			$(CC) $(LDFLAGS) -o brctl brctl.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a

brctld:			brctld.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a
			$(CC) $(LDFLAGS) -o brctld brctld.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a

clean:
			rm -f *.o
			rm -f brctl
			rm -f brctld
			rm -f core

brctl.o:		brctl.c brctl.h
			$(CC) -s -I../libbridge $(CFLAGS) -o brctl.o brctl.c

brctld.o:		brctld.c brctl.h
			$(CC) -s -I../libbridge $(CFLAGS) -o brctld.o brctld.c

brctl_cmd.o:		brctl_cmd.c brctl.h
			$(CC) -s -I../libbridge $(CFLAGS) -o brctl_cmd.o brctl_cmd.c

brctl_disp.o:		brctl_disp.c brctl.h
			$(CC) -s -I../libbridge $(CFLAGS) -o brctl_disp.o brctl_disp.c
