include ../.linux_config

EXES-$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST) += wait_dsp
OBJS-$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST) += wait_dsp.o 

CFLAGS = -g -Wall -I../includes 
LDFLAGS = -L../voip_manager -lvoip_manager 

EXES = $(EXES-y)
OBJS = $(OBJS-y)

DEPENDENCY	= $(OBJS:.o=.d)

all: $(EXES) 

$(EXES): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ 

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

romfs:
	[ ! -f wait_dsp ] || $(ROMFSINST) wait_dsp /bin/wait_dsp

clean:
	rm -f $(LIBS) $(OBJS) *.d

-include $(DEPENDENCY)
