include ../../.linux_config

CFLAGS += -I../../includes -Wall
ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS 		+= -I../../ui/include
endif
CFLAGS += $(ECOS_SHELL_CFLAGS)

objs = cli.o clishell.o cliprint.o cmdtree.o cmdcore.o cmdargs.o 

lib = voipcli.a

all: $(objs) 
	$(AR) rcs $(lib) $(objs) 

ecos-all: $(objs)
	$(AR) rcs $(APP_LIB) $(objs)

clean:
	rm -f $(objs) $(lib)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
