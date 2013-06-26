include ../.linux_config

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_865X), y)
CFLAGS += -I../includes
DOXYFILE = ./document/Doxyfile.865x
else
CFLAGS += -I../includes
DOXYFILE = ./document/Doxyfile
endif

ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS += -I../ui/include
endif

ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
DECT_PATH = ../dect_sitel/sc1445x_src
CFLAGS += -I$(DECT_PATH)/SiDECT
DECT_LIBA = $(DECT_PATH)/sacvm.a
endif
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
DECT_DSPG_PATH = ../src/dspg_cmbs_app
DECT_DSPG_LIBA = $(DECT_DSPG_PATH)/dspg_cmbs_app_release.a
CMBS_PATH = cmbs_tcx
CFLAGS += -I$(DECT_DSPG_PATH)/$(CMBS_PATH)/test
CFLAGS += -I$(DECT_DSPG_PATH)/$(CMBS_PATH)/cmbs
CFLAGS += -I$(DECT_DSPG_PATH)/$(CMBS_PATH)/test/appcmbs
endif

CFLAGS += -fPIC -Werror
CFLAGS += $(ECOS_SHELL_CFLAGS) 

LIBA = libvoip_manager.a
LIBSO = libvoip_manager.so
OBJS = voip_manager.o 

TMPSO = ./tmpso

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
#OBJS += voip_manager_dect.o 
#endif

DEPENDENCY	= $(OBJS:.o=.d)

ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
all: $(DECT_LIBA) $(OBJS) doc
	cp $(DECT_LIBA) $(LIBA) -f 
	$(AR) q $(LIBA) $(OBJS)
	make libso
	make example
endif
ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
all: $(DECT_DSPG_LIBA) $(OBJS) doc
	cp $(DECT_DSPG_LIBA) $(LIBA) -f 
	$(AR) rcs $(LIBA) $(OBJS)
	make libso
	make example
endif
else
all: $(OBJS) doc
	$(AR) rcs $(LIBA) $(OBJS)
	make libso
	make example
endif

ecos-all: $(OBJS)
	$(AR) rcs $(APP_LIB) $(OBJS)

libso:
	rm -rf $(TMPSO); mkdir $(TMPSO); cd $(TMPSO);	\
	$(AR) -x ../$(LIBA);	\
	mv -f keyb_data.o keyb_data.o-unused;			\
	mv -f keyb_facility.o keyb_facility.o-unused;	\
	$(LD) -shared *.o -o ../$(LIBSO) || exit $$?;	\
	cd .. 
	rm -f $(LIBA)

romfs:
	$(ROMFSINST)    $(LIBSO)   /lib/$(LIBSO)
	make -C document/examples romfs

clean:
	rm -f $(LIBA) $(LIBSO) $(OBJS) *.d
	rm -rf document/html
	rm -rf document/latex
	make -C document/examples clean

doc:
	doxygen $(DOXYFILE)
	-cp ./document/realtek.jpg ./document/html

example:
	make -C document/examples

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)
