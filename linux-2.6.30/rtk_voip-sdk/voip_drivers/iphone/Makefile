ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
obj-y    := ipphone_interface.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510) += WM8510.o
#obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510) += base_i2c_core.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510) += base_i2c_WM8510.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621) += ALC5621.o
#obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621) += base_i2c_core.o
obj-$(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621) += base_i2c_ALC5621.o

objs-8952 := base_gpio_8952.o keyscan_matrix.o led_directgpio.o lcm_epl65132.o lcm_mmap.o
objs-8972 := base_gpio_8972.o lcm_ht1650.o PT6961.o
objs-8186 := base_gpio_8186.o lcm_char16x2.o keyscan_tri.o led_74hc164.o
objs-8972b_v00 := base_gpio_8972b.o lcm_splc780d.o keyscan_matrix.o led_directgpio.o
objs-8972b := base_gpio_8972b.o

ifeq ($(CONFIG_RTK_VOIP_GPIO_IPP_8952_V00),y)
obj-y += $(objs-8952) 
endif
ifeq ($(CONFIG_RTK_VOIP_GPIO_IPP_8972_V00),y)
obj-y += $(objs-8972)
endif
ifeq ($(CONFIG_RTK_VOIP_GPIO_IPP_8972_V01),y) 
obj-y += $(objs-8972) 
endif
ifeq ($(CONFIG_RTK_VOIP_GPIO_IPP_101),y)
obj-y += $(objs-8186) 
endif
ifeq ($(CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00),y)
obj-y += $(objs-8972b_v00)
endif
ifeq ($(CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99),y)
obj-y += $(objs-8972b) 
endif

ifeq ($(CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99),y)
obj-$(CONFIG_RTK_VOIP_IP_PHONE)   += buildno.o  
else
obj-$(CONFIG_RTK_VOIP_IP_PHONE)   += keypad_interface.o lcm_interface.o buildno.o  
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include

include $(DIR_VOIP)/rules.mak

# increase build number 
buildno.o : buildno.c /proc/uptime
	@n=`grep BUILDNO $< | cut -d/ -f1 | tr -d ' \t'`;    \
	m=`expr $$n + 1`;   \
	x=`sed -e "/BUILDNO/s/$$n/$$m/g" $<`;    \
	n=`echo "$$x" | grep BUILDDATE | cut -d/ -f1 | tr -d ' \t'`;    \
	m=`date +%Y%m%d`;   \
	y=`echo "$$x" | sed -e "/BUILDDATE/s/$$n/$$m/g"` ;  \
	echo "$$y" > $<; echo "" >> $< 
	$(CC) $(CFLAGS) -c -o $@ $< 

endif

