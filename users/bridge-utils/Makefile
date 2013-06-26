ifeq ($(SLINK),1)
S_LINK="SLINK=1"
else
S_LINK=
endif

ifeq ($(MD),615-96b)
GUEST_ZONE="GUEST_ZONE=1"
else
GUEST_ZONE=
endif


#export HARD_FLOAT=-mhard-float


all:				brctl/brctl brctl/brctld misc/bidi

clean:
				make -C brctl clean
				make -C libbridge clean
				make -C misc clean

brctl/brctl:			brctl/brctl.c brctl/brctl.h brctl/brctl_cmd.c brctl/brctl_disp.c libbridge/libbridge.a libbridge/libbridge.h
				make -C brctl $(S_LINK) $(GUEST_ZONE)

brctl/brctld:			brctl/brctld.c brctl/brctl.h brctl/brctl_cmd.c brctl/brctl_disp.c libbridge/libbridge.a libbridge/libbridge.h
				make -C brctl

libbridge/libbridge.a:		libbridge/if_index.c libbridge/libbridge.h libbridge/libbridge_compat.c libbridge/libbridge_devif.c libbridge/libbridge_if.c libbridge/libbridge_init.c libbridge/libbridge_misc.c libbridge/libbridge_private.h
				make -C libbridge $(S_LINK) $(GUEST_ZONE)

misc/bidi:			misc/bidi.c
				make -C misc

romfs:
	$(ROMFSINST) brctl/brctl /bin/brctl
