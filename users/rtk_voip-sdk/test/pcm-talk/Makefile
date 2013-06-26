NM = $(CROSS_COMPILE)nm
CFLAGS = -lpthread -static
CFLAGS_SERVER = $(CFLAGS) -DTALK_SERVER

all:talksvr talkcln pcmloop pcmioctl

talksvr: talk.c talk.h
	$(CC) -o $@ talk.c $(CFLAGS_SERVER)
#	$(NM) $@ | sort > talksvr.map
	$(STRIP) $@

talkcln: talk.c talk.h
	$(CC) -o $@ talk.c $(CFLAGS)
#	$(NM) $@ | sort > talkcln.map
	$(STRIP) $@

pcmloop: pcmloop.c
	$(CC) -o $@ pcmloop.c

pcmioctl: pcmioctl.c
	$(CC) -o $@ pcmioctl.c

clean:
	rm -f talksvr
	rm -f talkcln
	rm -f pcmloop
	rm -f pcmioctl
	rm -f *.map
