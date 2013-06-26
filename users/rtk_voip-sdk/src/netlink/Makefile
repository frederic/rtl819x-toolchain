include ../../.linux_config

TARGET		=	netlink
OBJS		=	netlink.o
LIBS		=	
DEPENDENCY	=	$(OBJS:.o=.d)
LDFLAGS		=	-L../../voip_manager -lvoip_manager

ifeq ($(CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT), y)
LDFLAGS		+=	-ldl -lpthread
endif

CFLAGS		=	-I../../includes/

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS		+=	-I../../ui/include/
endif

ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
LDFLAGS	+= -lpthread -ldl
endif

# 865x only
CFLAGS		+=	-I../../../../include/

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
