# ===========================================================================
# Kernel configuration targets
# These targets are used from top-level makefile

.PHONY: oldconfig menuconfig config silentoldconfig

Kconfig := Kconfig

menuconfig: $(DIR_ROOT)/config/mconf
	$(DIR_ROOT)/config/mconf $(Kconfig)

config: $(DIR_ROOT)/config/conf
	$(DIR_ROOT)/config/conf $(Kconfig)

oldconfig: $(DIR_ROOT)/config/conf
	$(DIR_ROOT)/config/conf -o $(Kconfig)

silentoldconfig: $(DIR_ROOT)/config/conf
	$(DIR_ROOT)/config/conf -s $(Kconfig)
