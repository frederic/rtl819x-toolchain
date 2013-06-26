CFLAGS = -O2 -c -static -Wall -I./include 
#CFLAGS = -g -c -static -Wall -I./include
OUT_DIR	= ./tmp/

AP_INCLUDE_H_DIR = ../includes
ifdef CONFIG_DEFAULTS_KERNEL_2_6
#KERNEL_H_DIR = ../../../linux-2.6.x/rtk_voip/include
ifneq ("$(DIR_LINUX)","")
KERNEL_IPHONE_DRIVER_DIR =$(DIR_LINUX)/rtk_voip/voip_drivers/iphone
else
KERNEL_IPHONE_DRIVER_DIR = ../../../../linux-2.6.x/rtk_voip/voip_drivers/iphone
endif
else
#KERNEL_H_DIR = ../../../linux-2.4.18/rtk_voip/include
KERNEL_IPHONE_DRIVER_DIR = ../../../../linux-2.4.18/rtk_voip/voip_drivers/iphone
endif

CFLAGS += -I$(AP_INCLUDE_H_DIR)

SRC_UI_OBJ =	ui_buildno.o		\
				ui_call.o			\
				ui_datetime.o		\
				ui_event_handler.o	\
				ui_event_send.o		\
				ui_exit.o			\
				ui_host.o			\
				ui_main.o			\
				ui_menu.o			\
				ui_mode.o			\
				ui_phonebook.o		\
				ui_records.o		\
				ui_softkey.o		\
				ui_state.o			\
				ui_state_check.o	\
				ui_sup.o				

SRC_RES_OBJ =	res_font_chinese.o	\
				res_font_english.o	\
				res_ime.o			\
				res_image.o			\
				res_string.o

SRC_COM_OBJ = 	com_edit.o			\
				com_ime.o			\
				com_ime_english.o	\
				com_ime_phonetic.o	\
				com_ime_pinyin.o	\
				com_ime_wubihua.o	\
				com_scroll.o		\
				com_select.o			

SRC_GS_OBJ =	gs_basic.o			\
				gs_cursor.o			\
				gs_drawimage.o		\
				gs_drawtext.o		\
				gs_font.o			\
				gs_lib.o			\
				gs_shape.o			\
				gs_vram.o				
				
SRC_FLASH_OBJ =	flash_rw_api.o			
				
SRC_IOCTL_OBJ =	ioctl_codec.o		\
				ioctl_kernel.o		\
				ioctl_keypad.o		\
				ioctl_lcd.o			\
				ioctl_led.o			\
				ioctl_net.o			\
				ioctl_misc.o		\
				ioctl_softrtc.o		\
				ioctl_timer.o			

SRC_MM_OBJ =	mm_animate.o		\
				mm_led.o			\
				mm_ring.o			\
				mm_sup.o

UI_SRC_OBJ =	$(SRC_UI_OBJ) $(SRC_RES_OBJ) $(SRC_COM_OBJ) 			\
				$(SRC_GS_OBJ) $(SRC_FLASH_OBJ) $(SRC_IOCTL_OBJ)			\
				$(SRC_MM_OBJ) 

DEP_SRC_OBJ =	$(UI_SRC_OBJ) 
ALL_SRC_OBJ =	$(UI_SRC_OBJ) 

LINK_LIBS	=	../../flash/libvoip_flash_client.a -L../../voip_manager -lvoip_manager

all: $(OUT_DIR)ui

########################################################
# generate execute file
#
$(OUT_DIR)ui: $(ALL_SRC_OBJ)
	@echo "Generate execute file... ui"
	@echo "all:" > $(OUT_DIR)Makefile
	@echo "	@$(CC) $(ALL_SRC_OBJ) $(LINK_LIBS) -o ui -lpthread" >> $(OUT_DIR)Makefile
	make -C $(OUT_DIR)

########################################################
# rules
#

#### depend rules
$(OUT_DIR)%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

#### .o -> ./tmp/.o
%.o: $(OUT_DIR)%.o
	
#### no depend rules. 
%.o : %.c
	$(CC) $(CFLAGS) -c -o $(OUT_DIR)$@ $<

#### increase build number 
$(OUT_DIR)ui_buildno.o : ui_buildno.c /proc/uptime
	@n=`grep BUILDNO $< | cut -d/ -f1 | tr -d ' \t'`;    \
	m=`expr $$n + 1`;   \
	x=`sed -e "/BUILDNO/s/$$n/$$m/g" $<`;    \
	n=`echo "$$x" | grep BUILDDATE | cut -d/ -f1 | tr -d ' \t'`;    \
	m=`date +%Y%m%d`;   \
	y=`echo "$$x" | sed -e "/BUILDDATE/s/$$n/$$m/g"` ;  \
	echo "$$y" > $<; echo "" >> $< 
	$(CC) $(CFLAGS) -c -o $@ $< 

########################################################
# dep
#
dep:
	@if [ ! -d $(OUT_DIR) ]; then 						\
		echo "Build output directory..." $(OUT_DIR);	\
		mkdir $(OUT_DIR);								\
    fi; 
	ln -fs $(KERNEL_IPHONE_DRIVER_DIR)/keypad_map.h include/keypad_map.h
	ln -fs ../flash_layout.h include/ui_flash_layout.h
	@rm -f .depend
	@#for i in $(ALL_SRC_NAME); do gcc -MT $(OUT_DIR)/$${i} -MM "$${i}.c" ; done > .depend
	@#for i in $(ALL_SRC_SNAME); do gcc -MT $(OUT_DIR)/$${i}.o -MM "$${i}.c" ; done > .depend
	@#for i in $(ALL_SRC_OBJ); do gcc $(CFLAGS) -MT $(OUT_DIR)$${i} -MM "$${i/.o/.c}" ; done > .depend
	@for i in $(DEP_SRC_OBJ); do echo "make dep..." $${i} ; gcc $(CFLAGS) -MT $(OUT_DIR)$${i} -MM "$${i/.o/.c}" >> .depend ; done
	@#for fn in $(SRC_IOCTL_NAME) ; $(CC) $(CFLAGS) -MM $(OUT_DIR)/$(fn).o $(fn).c >> .depend ; done

ifeq (.depend,$(wildcard .depend))
include .depend
endif

########################################################
# copy to nfs directory
#
nfs:
	cp -f $(OUT_DIR)ui /work/nfs/ipphone/ui

########################################################
# clean
#
clean: 
	rm -f $(OUT_DIR)*.o $(OUT_DIR)ui
	@#rm -f $(ALL_SRC_OBJ) ui
