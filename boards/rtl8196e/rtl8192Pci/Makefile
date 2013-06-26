#
# drivers/net/rtl8190/rtl8192Pci
#
# Makefile for build rtl8192 f/w binary
#

#O_TARGET	:= rtl8192Pci.o

obj-y		:= rtl8192sfw.o AGC_TAB.o phy_reg.o MACPHY_REG.o \
			radio_a.o radio_b.o PHY_REG_PG.o phy_reg_MP.o PHY_to1T2R.o PHY_to1T2R_b.o PHY_to1T1R.o PHY_to1T1R_b.o radio_a_hp.o
		
%.o: %.bin
	@touch dummy.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -DKBUILD_BASENAME=$(subst $(comma),_,$(subst -,_,$(*F))) $(CFLAGS_$@) -c -o $@ dummy.c
#	$(OBJCOPY) --add-section .$<=$< $@
	$(OBJCOPY) --add-section .$(notdir $<)=$< $@
	
%.o: %.txt
	@touch dummy.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -DKBUILD_BASENAME=$(subst $(comma),_,$(subst -,_,$(*F))) $(CFLAGS_$@) -c -o $@ dummy.c
#	$(OBJCOPY) --add-section .$<=$< $@
	$(OBJCOPY) --add-section .$(notdir $<)=$< $@
	

#include $(TOPDIR)/Rules.make
