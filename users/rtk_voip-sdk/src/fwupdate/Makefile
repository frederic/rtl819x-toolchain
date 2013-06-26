include ../../.linux_config

TARGET		=	fwupdate
OBJS		=	fwupdate.o
LIBS		=	../../flash/libvoip_flash_client.a
DEPENDENCY	=	$(OBJS:.o=.d)
LDFLAGS		=	-lpthread -ldl -L../../voip_manager/ -lvoip_manager 
CFLAGS		=	-I../../includes/

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS		+=	-I../../ui/include/
endif

# 865x only
CFLAGS		+=	-I../../../../include/

CFLAGS		+=	-DVOIP_SUPPORT -DHOME_GATEWAY

ifneq ($(CONFIG_RTK_VOIP_PACKAGE_867X), y)
all: $(OBJS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)
endif

clean:
	rm -f $(TARGET) $(OBJS) $(DEPENDENCY)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)
