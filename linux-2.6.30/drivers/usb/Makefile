#
# Makefile for the kernel USB device drivers.
#

# Object files in subdirectories

ifeq ($(CONFIG_USB_UWIFI_HOST),y)
obj-$(CONFIG_USB)		+= core_uWiFi/
else
obj-$(CONFIG_USB)		+= core/
endif

obj-$(CONFIG_USB_MON)		+= mon/

ifeq ($(CONFIG_USB_UWIFI_HOST),y)
obj-$(CONFIG_PCI)		+= host_uWiFi/
obj-$(CONFIG_USB_EHCI_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_ISP116X_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_OHCI_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_UHCI_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_FHCI_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_SL811_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_U132_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_R8A66597_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_HWA_HCD)	+= host_uWiFi/
obj-$(CONFIG_USB_ISP1760_HCD)	+= host_uWiFi/
else
obj-$(CONFIG_PCI)		+= host/
obj-$(CONFIG_USB_EHCI_HCD)	+= host/
obj-$(CONFIG_USB_ISP116X_HCD)	+= host/
obj-$(CONFIG_USB_OHCI_HCD)	+= host/
obj-$(CONFIG_USB_UHCI_HCD)	+= host/
obj-$(CONFIG_USB_FHCI_HCD)	+= host/
obj-$(CONFIG_USB_SL811_HCD)	+= host/
obj-$(CONFIG_USB_U132_HCD)	+= host/
obj-$(CONFIG_USB_R8A66597_HCD)	+= host/
obj-$(CONFIG_USB_HWA_HCD)	+= host/
obj-$(CONFIG_USB_ISP1760_HCD)	+= host/
endif

obj-$(CONFIG_USB_C67X00_HCD)	+= c67x00/

obj-$(CONFIG_USB_WUSB)		+= wusbcore/

obj-$(CONFIG_USB_ACM)		+= class/
obj-$(CONFIG_USB_PRINTER)	+= class/
obj-$(CONFIG_USB_WDM)		+= class/
obj-$(CONFIG_USB_TMC)		+= class/

ifeq ($(CONFIG_USB_UWIFI_HOST),y)
obj-$(CONFIG_USB_STORAGE)	+= storage_uWiFi/
obj-$(CONFIG_USB)		+= storage_uWiFi/
else
obj-$(CONFIG_USB_STORAGE)	+= storage/
obj-$(CONFIG_USB)		+= storage/
endif
obj-$(CONFIG_USB_MDC800)	+= image/
obj-$(CONFIG_USB_MICROTEK)	+= image/

obj-$(CONFIG_USB_SERIAL)	+= serial/

obj-$(CONFIG_USB)		+= misc/

obj-$(CONFIG_USB_ATM)		+= atm/
obj-$(CONFIG_USB_SPEEDTOUCH)	+= atm/

obj-$(CONFIG_DWC_OTG)       += dwc_otg/

#wei add for otg device mode
ifeq ($(CONFIG_RTL_USB_OTG),y)
obj-$(CONFIG_USB_GADGET)	+= gadget_cathy/
else
obj-$(CONFIG_USB_GADGET)	+= gadget/
endif
