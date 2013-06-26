#
# Makefile for the kernel serial device drivers.
#
ifeq ($(CONFIG_SERIAL_SC16IS7X0),y)
	EXTRA_CFLAGS += -DCONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
endif

obj-$(CONFIG_SERIAL_CORE) += serial_core.o

# These Sparc drivers have to appear before others such as 8250
# which share ttySx minor node space.  Otherwise console device
# names change and other unplesantries.

obj-$(CONFIG_SERIAL_8250) += 8250.o
obj-$(CONFIG_SERIAL_8250_PNP) += 8250_pnp.o
obj-$(CONFIG_SERIAL_8250_GSC) += 8250_gsc.o
obj-$(CONFIG_SERIAL_8250_PCI) += 8250_pci.o
obj-$(CONFIG_SERIAL_8250_CONSOLE) += 8250_early.o
obj-$(CONFIG_SERIAL_OF_PLATFORM) += of_serial.o
obj-$(CONFIG_SERIAL_OF_PLATFORM_NWPSERIAL) += nwpserial.o
obj-$(CONFIG_KGDB_SERIAL_CONSOLE) += kgdboc.o
obj-$(CONFIG_SERIAL_SC16IS7X0) += 8250_sc16is7x0.o
obj-$(CONFIG_SERIAL_SC16IS7X0) += i2c.o
obj-$(CONFIG_SERIAL_SC16IS7X0) += gpio_8972b.o

EXTRA_CFLAGS += -Iinclude/asm-mips/rtl865x/
