#############################################################
#
# This example buildroot Makefile provides a minimal set of
# rules which allow buildroot to install your package to
# the target filesystem. You are free to insert as many rules
# in between as you need, just update the dependencies
# accordingly.
#
# Keep in mind that this makefile is directly included by the
# Makefile at the top of the buildroot tree. So remember to
# change all dallas controller references to something unique to 
# avoidinsta namespace collisions.
#
# Also note that this example makefile is only meant to provide
# hints on how to call your package's build system from within
# buildroot. While the examples provided below may work with
# most projects, they may not be applicable to yours. It is your
# responsibility to ensure that the rules below are correct for
# your project, and make the appropriate adjustments.
#
# Buildroot defines some useful variables to help you integrate
# your package with the buildroot system:
#
# DL_DIR: Directory where source archives are placed.
# BUILD_DIR: Directory where all source archives are unpacked,
#	     and where the build takes place.
# TARGET_DIR: Location of the target root filesystem.
# BASE_DIR: Top of the buildroot tree.
# TARGET_PATH: Location of the toolchain binaries.
# TARGET_CROSS: Prefix appended to the names of toolchain binaries.
# TARGET_CONFIGURE_OPTS: This variable defines a number of common
#	   Makefile variables (mostly toolchain-related, like
#	   CC, AR, ...) for buildroot. Should be used with
#	   "make -C" commands.
# TARGET_CC: Shortcut for $(TARGET_CROSS)gcc
# TARGET_CFLAGS: The common compiler flags used by gcc
#		  for the target.
# SWS_DIR: Path to where all the reuse components are populated.
# DISABLE_NLS: Used mainly for configure scripts, enables a common
#		option for configure when native language support
#		is disabled in uclibc.
# GNU_HOST_NAME: Specifies the machine type of the build host.
# GNU_TARGET_NAME: Specifies the machine type of the target.
#
#############################################################


#############################################################
#
# Convenience variables
#
#############################################################
DALLAS_CONTROLLER_VER:=R01_00-V001
DALLAS_CONTROLLER_SOURCE_URL:=http://www.pmc-sierra.com
DALLAS_CONTROLLER_SOURCE:=dls_controller-$(DALLAS_CONTROLLER_VER).tar.bz2
DALLAS_CONTROLLER_BUILD_DIR:=$(BUILD_DIR)/dls_controller-$(DALLAS_CONTROLLER_VER)

ifeq ($(strip $(BR2_ALT_TOOLCHAIN)),y)
DALLAS_CONTROLLER_TOOLCHAIN_PATH:=$(subst ",,$(strip $(BR2_ALT_TOOLCHAIN_PATH)))/bin
#")
DALLAS_CONTROLLER_TOOLCHAIN_PREFIX:=$(DALLAS_CONTROLLER_TOOLCHAIN_PATH)/$(subst ",,$(strip $(BR2_ALT_TOOLCHAIN_PREFIX)))
#")
else
DALLAS_CONTROLLER_TOOLCHAIN_PATH:=$(TARGET_PATH)
DALLAS_CONTROLLER_TOOLCHAIN_PREFIX:=$(TARGET_CROSS)
endif

# Suffix to add to module names
ifeq ($(strip $(BR2_TARGET_PMC_MSP7120_LX_SDK)),y)
  PLATFORM_SUFFIX:="msp7120"
else
  ifeq ($(strip $(BR2_TARGET_PMC_MSP7140_LX_SDK)),y)
    PLATFORM_SUFFIX:="msp7140"
  else
    PLATFORM_SUFFIX:="unknown"
  endif
endif

# Kludge to get dallas controller to see the proper kernel release.
KVERSION=$(strip $(shell grep "^VERSION" $(LINUX_DIR)/Makefile | head -n 1 | cut -f 2 -d'='))
KPATCHLEVEL=$(strip $(shell grep "^PATCHLEVEL" $(LINUX_DIR)/Makefile | head -n1 | cut -f 2 -d'='))
KSUBLEVEL=$(strip $(shell grep "^SUBLEVEL" $(LINUX_DIR)/Makefile | head -n 1 |cut -f 2 -d'='))
KEXTRAVERSION=$(strip $(shell grep "^EXTRAVERSION" $(LINUX_DIR)/Makefile | head -n 1 | cut -f 2 -d'='))
KLOCALVERSION=$(subst ",,$(strip $(shell grep "^CONFIG_LOCALVERSION" $(LINUX_DIR)/.config | head -n 1 | cut -f 2 -d'=')))
#")
KERNELRELEASE=$(KVERSION).$(KPATCHLEVEL).$(KSUBLEVEL)$(KEXTRAVERSION)$(KLOCALVERSION)
DALLAS_CONTROLLER_MODULEPATH=$(TARGET_DIR)/lib/modules/${KERNELRELEASE}/dallas

# Environment variables to enable desired optimizations.
DALLAS_CONTROLLER_OPT_ENV:=

ifeq ($(strip $(BR2_DALLAS_NON-SLAVE_MODE)),y)
        DALLAS_WORK_MODE:=non-slave_mode
	DALLAS_IMAGE:=dallas.bin
else
        DALLAS_WORK_MODE:=slave_mode
	DALLAS_IMAGE:=isr.bin
endif

#############################################################
#
# This is the "fetch" rule, which instructs buildroot how to
# download your package source code if it isn't included yet.
# We may support this sometime in the future, but for now
# the necessary tarball should be included with our releases.
# Just leave this rule empty.
#
#############################################################
$(DL_DIR)/$(DALLAS_CONTROLLER_SOURCE):
	#$(WGET) -P $(DL_DIR) $(DALLAS_CONTROLLER_SOURCE_URL)/$(DALLAS_CONTROLLER_SOURCE)

#############################################################
#
# This is the "unpack" rule, which instructs buildroot how to
# extract the source code from your package archive and where
# to place it.
#
#############################################################
$(DALLAS_CONTROLLER_BUILD_DIR)/.unpacked:
	@if [ -e $(SWS_DIR)/dls_controller ]; then \
		 ln -sfn $(SWS_DIR)/dls_controller $(DALLAS_CONTROLLER_BUILD_DIR); \
	else \
		 bzcat $(DL_DIR)/$(DALLAS_CONTROLLER_SOURCE) | tar -C $(BUILD_DIR) -xf -; \
	fi
	touch $(DALLAS_CONTROLLER_BUILD_DIR)/.unpacked

#############################################################
#
# This is the "configure" rule. This is where you would apply
# your patches, run any build configuration scripts, and make
# changes to Makefile rules, variables, etc -- whatever is
# necessary to make your package build and install under
# buildroot.
#
# toolchain/patch-kernel.sh is a general source tree patching
# program included with buildroot. The arguments are:
# - target directory (normally $(DALLAS_CONTROLLER_BUILD_DIR) )
# - patch directory (path to where your patches reside,
#		      relative to the top of the buildroot dir)
# - remaining arguments are the actual patch files to apply.
#   Wildcards are permitted, but remember to escape them with
#   a backslash ('\') to avoid expansion before the shell
#   passes the wildcard characters to patch-kernel.sh.
#
#############################################################
$(DALLAS_CONTROLLER_BUILD_DIR)/.patched: $(DALLAS_CONTROLLER_BUILD_DIR)/.unpacked
	# Apply patches.
	toolchain/patch-kernel.sh $(DALLAS_CONTROLLER_BUILD_DIR) package/dls_controller/ dls_controller\*.patch
	touch $(DALLAS_CONTROLLER_BUILD_DIR)/.patched

$(DALLAS_CONTROLLER_BUILD_DIR)/.configured: $(DALLAS_CONTROLLER_BUILD_DIR)/.patched
	touch  $(DALLAS_CONTROLLER_BUILD_DIR)/.configured

#############################################################
#
# The "build" rule. Set the rule target to a file whose
# existance should signal the completion of a successful
# build. This rule typically invokes "make -C $(DALLAS_CONTROLLER_BUILD_DIR)"
#
#############################################################
$(DALLAS_CONTROLLER_BUILD_DIR)/dallas_app/dallas_ctrl: $(DALLAS_CONTROLLER_BUILD_DIR)/.configured
	$(DALLAS_CONTROLLER_OPT_ENV) \
	DALLAS_WORK_MODE="$(DALLAS_WORK_MODE)" \
	$(MAKE) -C $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_app \
		 TOOLPREFIX=$(TARGET_CROSS) \
		 CFLAGS='$$(INCS) $(TARGET_CFLAGS)' \
		 KERNELPATH=$(LINUX_DIR) \
		 KERNELRELEASE=$(KERNELRELEASE) \
		 TARGET=mipsisa32-be-elf

$(DALLAS_CONTROLLER_BUILD_DIR)/dallas_drv/dallas_ctrl.ko: $(DALLAS_CONTROLLER_BUILD_DIR)/.configured 
	$(DALLAS_CONTROLLER_OPT_ENV) \
	DALLAS_WORK_MODE="$(DALLAS_WORK_MODE)" \
	$(MAKE) -C $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_drv \
		 TARGET=mipsisa32-be-elf \
		 TOOLPREFIX=$(DALLAS_CONTROLLER_TOOLCHAIN_PREFIX) \
		 KERNELPATH=$(LINUX_DIR) \
		 KERNELRELEASE=$(KERNELRELEASE)

#############################################################
#
# The "install" rule. Set the rule target to a file in the
# target filesystem whose existance should signal a successful
# installation. This rule typically invokes
# "make -C $(DALLAS_CONTROLLER_BUILD_DIR) DESTDIR=$(TARGET_DIR) install".
# Sometimes it's a good idea to strip unneeded symbols from
# your installed program to save space.
#
#############################################################
dls_controller-install: $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_app/dallas_ctrl $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_drv/dallas_ctrl.ko
	@cp $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_app/dallas_ctrl $(TARGET_DIR)/sbin/dallas_ctrl
	@mkdir -p $(DALLAS_CONTROLLER_MODULEPATH)
	@cp $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_drv/dallas_ctrl.ko $(DALLAS_CONTROLLER_MODULEPATH)
	@cp $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_lib/$(DALLAS_IMAGE) $(DALLAS_CONTROLLER_MODULEPATH)
	@cp $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_lib/nvdb_config $(DALLAS_CONTROLLER_MODULEPATH)

	$(STRIP) $(TARGET_DIR)/sbin/dallas_ctrl

	# Default startup scripts for the dallas driver.
	@mkdir -p $(TARGET_DIR)/etc/init.d
	@cp -f package/dls_controller/S60dallas $(TARGET_DIR)/etc/init.d
	@chmod a+x $(TARGET_DIR)/etc/init.d/S60dallas
	
	@touch $(DALLAS_CONTROLLER_BUILD_DIR)/.mods_installed

.PHONY: install

#############################################################
#
# These are the rules that buildroot needs to see. You choose
# the name of the master build rule (usually named after your package)
# and buildroot assembles the names of the clean, source, and
# dirclean rules from this base. Use the install rule above
# as the dependency for the master build rule.
#
#############################################################

dls_controller: linux-kernel-prepare zlib dls_controller-install

dls_controller-tools-distrib: linux-kernel-prepare \
			  $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_drv/dallas_controller.ko \
			  $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_app/dallas_controller

dls_controller-tools-clean:
	@echo "Removing all dls_controller files from target rootfs"
	@rm -f  $(TARGET_DIR)/sbin/dallas_ctrl
	@rm -f  DALLAS_CONTROLLER_MODULEPATH/$(DALLAS_IMAGE)
	@rm -f  DALLAS_CONTROLLER_MODULEPATH/dallas_ctrl.ko
	@rm -f  $(TARGET_DIR)/etc/init.d/S60dallas

ifneq ($(strip $(wildcard $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_drv/Makefile)),)
	-$(MAKE) -C $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_drv TARGET=mipsisa32-be-elf clean
	-$(MAKE) -C $(DALLAS_CONTROLLER_BUILD_DIR)/dallas_app TARGET=mipsisa32-be-elf clean
endif

dls_controller-tools-dirclean:
	rm -rf $(DALLAS_CONTROLLER_BUILD_DIR)

#############################################################
#
# Toplevel Makefile options
# This ensures that the dallas controller master build rule is included
# when dallas controller is selected in "make config".
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_DLS_CONTROLLER)),y)
TARGETS+=dls_controller
endif
