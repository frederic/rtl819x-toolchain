#
# drivers/net/rtl8192cd
#
# Makefile for the Linux Wireless network device drivers.
#

ifeq ($(CONFIG_RTL_92D_SUPPORT),y)
  ifeq ($(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D),y)
  RTL_WLAN_DATA_DIR := data
  endif
  RTL_WLAN_DATA_DIR_D := data_92d
else
  ifeq ($(CONFIG_RTL_92C_SUPPORT),y)
  RTL_WLAN_DATA_DIR := data
  endif
#  RTL_WLAN_DATA_DIR_D := data_92d
endif
ifeq ($(CONFIG_RTL_88E_SUPPORT),y)
  RTL_WLAN_DATA_DIR_E := data_88e
endif

ifeq ($(CONFIG_RTL_MESH_SUPPORT),y)
        EXTRA_CFLAGS += -DCONFIG_RTK_MESH #-DMESH_USE_METRICOP
#        ifeq ($(CONFIG_11S_TEST_MODE),y)
#        EXTRA_CFLAGS += -D_11s_TEST_MODE_
#        endif
        obj-mesh = ../mesh_ext/mesh_proc.o\
        ../mesh_ext/mesh_route.o\
        ../mesh_ext/mesh_rx.o\
        ../mesh_ext/mesh_sme.o\
        ../mesh_ext/mesh_security.o\
        ../mesh_ext/mesh_tx.o\
        ../mesh_ext/mesh_util.o\
        ../mesh_ext/mesh_11kv.o\
        ../mesh_ext/hash_table.o
endif

SRCS_TXT = $(addprefix $(src)/data_,$(notdir $(patsubst %.txt,%.c,$(wildcard $(src)/$(RTL_WLAN_DATA_DIR)/*.txt))))
SRCS_BIN = $(addprefix $(src)/data_,$(notdir $(patsubst %.bin,%.c,$(wildcard $(src)/$(RTL_WLAN_DATA_DIR)/*.bin))))
SRCS_TXT += $(addprefix $(src)/data_,$(notdir $(patsubst %.txt,%.c,$(wildcard $(src)/$(RTL_WLAN_DATA_DIR_D)/*.txt))))
SRCS_BIN += $(addprefix $(src)/data_,$(notdir $(patsubst %.bin,%.c,$(wildcard $(src)/$(RTL_WLAN_DATA_DIR_D)/*.bin))))
SRCS_TXT += $(addprefix $(src)/data_,$(notdir $(patsubst %.txt,%.c,$(wildcard $(src)/$(RTL_WLAN_DATA_DIR_E)/*.txt))))
SRCS_BIN += $(addprefix $(src)/data_,$(notdir $(patsubst %.bin,%.c,$(wildcard $(src)/$(RTL_WLAN_DATA_DIR_E)/*.bin))))


obj-$(CONFIG_RTL_WAPI_SUPPORT) += wapi_wai.o wapiCrypto.o
obj-$(CONFIG_RTL8192CD) += rtl8192cd.o

ifeq ($(CONFIG_RTL_88E_SUPPORT),y)
rtl8192cd-objs += HalPwrSeqCmd.o\
		Hal8188EPwrSeq.o\
		8188e_hw.o 

ifeq ($(CONFIG_RTL_ODM_WLAN_DRIVER),y)
	rtl8192cd-objs += \
		./OUTSRC/odm.o \
                ./OUTSRC/odm_debug.o\
                ./OUTSRC/odm_interface.o\
                ./OUTSRC/odm_HWConfig.o\
                ./OUTSRC/HalPhyRf.o\
                ./OUTSRC/rtl8188e/HalHWImg8188E_BB.o\
                ./OUTSRC/rtl8188e/HalHWImg8188E_MAC.o\
                ./OUTSRC/rtl8188e/HalHWImg8188E_RF.o\
                ./OUTSRC/rtl8188e/odm_RegConfig8188E.o\
                ./OUTSRC/rtl8188e/Hal8188ERateAdaptive.o\
                ./OUTSRC/rtl8188e/odm_RTL8188E.o\
                ./OUTSRC/rtl8188e/HalPhyRf_8188e.o		
else
	rtl8192cd-objs += Hal8192CDMOutSrc.o \
			  RateAdaptive.o 
endif
else
	rtl8192cd-objs += Hal8192CDMOutSrc.o
endif

rtl8192cd-objs +=\
	8192cd_tx.o\
	8192cd_rx.o\
	8192cd_osdep.o\
	8192cd_sme.o\
	8192cd_util.o\
	8192d_hw.o\
	8192cd_hw.o\
	8192cd_security.o\
	8192cd_tkip.o\
	8192cd_aes.o\
	8192cd_proc.o\
	8192cd_br_ext.o\
	8192cd_eeprom.o\
	8192cd_mp.o\
	8192cd_psk.o\
	8192cd_ioctl.o\
	1x_kmsm_aes.o\
	1x_kmsm_hmac.o\
	1x_md5c.o\
	1x_rc4.o\
	8192cd_mib.o\
	8192cd_dmem.o\
	8192cd_host.o\
	8192cd_led.o\
	8192cd_dfs.o\
	romeperf.o\
	HalDMOutSrc.o\
	$(obj-mesh)
	
ifeq ($(CONFIG_RTL_COMAPI_CFGFILE),y)
obj-y += 8192cd_comapi.o
endif
ifeq ($(CONFIG_RTL_P2P_SUPPORT),y)	
obj-y += 8192cd_p2p.o
endif
ifeq ($(CONFIG_RTL_COMAPI_WLTOOLS),y)
obj-y += 8192cd_comapi.o
endif
#For Hostapd
ifeq ($(CONFIG_RTL_HOSTAPD_SUPPORT),y)
obj-y += 8192cd_comapi.o
obj-y += 8192cd_net80211.o
obj-y += 8192cd_psk_hapd.o
endif
ifeq ($(CONFIG_RTL8672),y)
obj-y += rtl8672_port.o
endif

ifeq ($(CONFIG_RTL8672),y)
EXTRA_CFLAGS += -DCONFIG_RTL8196B -DCONFIG_RTL8196C -DCONFIG_RTL8196B_GW -DCONFIG_RTL8196C_TESTCHIP_PATCH -D_MP_TELNET_SUPPORT_
EXTRA_CFLAGS += -DCONFIG_COMPAT_NET_DEV_OPS
endif

#CONFIG_SINUX_SUPPORT=1
ifeq ($(CONFIG_SINUX_SUPPORT),1)
EXTRA_CFLAGS  += -DOPENSSL_FIPS -D__linux__ -DRSDK_BUILT -DOPENSSL_NO_SPEED -DOPENSSL_THREADS -D_REENTRANT \
                        -DDSO_DLFCN -DHAVE_DLFCN_H -DOPENSSL_NO_KRB5 -DB_ENDIAN -DTERMIO \
			-fomit-frame-pointer 
# -save-temps                      
                        
obj-y  += wps/ssl/mem.o wps/ssl/mem_clr.o wps/ssl/bn_add.o wps/ssl/bn_lib.o \
			wps/ssl/bn_asm.o wps/ssl/bn_const.o wps/ssl/bn_ctx.o \
                wps/ssl/bn_div.o wps/ssl/bn_exp.o wps/ssl/bn_gcd.o \
				wps/ssl/bn_mod.o wps/ssl/bn_mont.o \
                wps/ssl/bn_mul.o wps/ssl/bn_prime.o wps/ssl/bn_rand.o \
				wps/ssl/bn_recp.o wps/ssl/bn_shift.o wps/ssl/bn_sqr.o \
                wps/ssl/bn_word.o wps/ssl/dh_check.o wps/ssl/dh_gen.o \
				wps/ssl/dh_key.o wps/ssl/dh_lib.o wps/ssl/digest.o \
                wps/ssl/m_sha1.o wps/ssl/hmac.o wps/ssl/md_rand.o \
				wps/ssl/rand_lib.o wps/ssl/sha1dgst.o wps/ssl/sha256.o \
                wps/ssl/aes_cbc.o wps/ssl/fips_aes_core.o
				
#EXTRA_CFLAGS += -I../../../lib
obj-y += wps/8192cd_wscd.o
obj-y += wps/sercomm_intf.o
endif

EXTRA_CFLAGS += -I$(DIR_BOARD) 
#EXTRA_CFLAGS += -I$(DIR_BOARD) -Werror
#EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/

#
#Build C code for TXT or BIN file
#
$(obj)/8192cd_hw.o :  $(SRCS_TXT) $(SRCS_BIN)

$(obj)/data_%.c: $(src)/$(RTL_WLAN_DATA_DIR)/%.txt FORCE
	$(obj)/bin2c.pl $(notdir $(basename $@)) < $< > $@
$(obj)/data_%.c: $(src)/$(RTL_WLAN_DATA_DIR)/%.bin FORCE
	$(obj)/bin2c.pl $(notdir $(basename $@)) < $< > $@
$(obj)/data_%.c: $(src)/$(RTL_WLAN_DATA_DIR_D)/%.txt FORCE
	$(obj)/bin2c.pl $(notdir $(basename $@)) < $< > $@
$(obj)/data_%.c: $(src)/$(RTL_WLAN_DATA_DIR_D)/%.bin FORCE
	$(obj)/bin2c.pl $(notdir $(basename $@)) < $< > $@
$(obj)/data_%.c: $(src)/$(RTL_WLAN_DATA_DIR_E)/%.txt FORCE
	$(obj)/bin2c.pl $(notdir $(basename $@)) < $< > $@
$(obj)/data_%.c: $(src)/$(RTL_WLAN_DATA_DIR_E)/%.bin FORCE
	$(obj)/bin2c.pl $(notdir $(basename $@)) < $< > $@

