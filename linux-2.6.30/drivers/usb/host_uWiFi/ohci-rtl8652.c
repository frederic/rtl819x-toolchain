/*
 * OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 * (C) Copyright 2002 Hewlett-Packard Company
 *
 * Bus Glue for rtl8652.
 *
 * Written by Christopher Hoover <ch@hpl.hp.com>
 * Based on fragments of previous driver by Russell King et al.
 *
 * Modified for LH7A404 from ohci-sa1111.c
 *  by Durgesh Pattamatta <pattamattad@sharpsec.com>
 *
 * Modified for pxa27x from ohci-lh7a404.c
 *  by Nick Bane <nick@cecomputing.co.uk> 26-8-2004
 *
 * Modified for rtl8652 from ohci-pxa27x.c
 *  by Lennert Buytenhek <buytenh@wantstofly.org> 28-2-2006
 *  Based on an earlier driver by Ray Lehtiniemi
 *
 * This file is licenced under the GPL.
 */
#include <linux/device.h>
#include <linux/signal.h>
#include <linux/platform_device.h>
#include "usb-rtl8652.h"

static void rtl8652_start_hc(struct device *dev)
{
	return 0;
}

static void rtl8652_stop_hc(struct device *dev)
{
	return 0;
}

static int usb_hcd_rtl8652_probe(const struct hc_driver *driver,
			 struct platform_device *pdev)
{
	int retval;
	struct usb_hcd *hcd;

	if (pdev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		return -ENOMEM;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, "rtl8652");
	if (hcd == NULL)
		return -ENOMEM;

	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = pdev->resource[0].end - pdev->resource[0].start + 1;
	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		usb_put_hcd(hcd);
		retval = -EBUSY;
		goto err1;
	}

//	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	hcd->regs =  (void *)KSEG1ADDR(pdev->resource[0].start);
	if (hcd->regs == NULL) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto err2;
	}
	rtl8652_start_hc(&pdev->dev);

	ohci_hcd_init(hcd_to_ohci(hcd));

	retval = usb_add_hcd(hcd, pdev->resource[1].start, IRQF_SHARED);
	
	if (retval == 0)
		return retval;

	rtl8652_stop_hc(&pdev->dev);
//	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);

	return retval;
}

static void usb_hcd_rtl8652_remove(struct usb_hcd *hcd,
			struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	rtl8652_stop_hc(&pdev->dev);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

static int __devinit ohci_rtl8652_start(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;
	unsigned int fminterval;

	if ((ret = ohci_init(ohci)) < 0)
		return ret;
	
  	fminterval = 0x2edf;
	ohci_writel (ohci,(fminterval * 9) / 10, &ohci->regs->periodicstart);
	fminterval |= ((((fminterval - 210) * 6) / 7) << 16); 
	ohci_writel (ohci,fminterval, &ohci->regs->fminterval);	
	ohci_writel (ohci,0x628, &ohci->regs->lsthresh);
	ohci_writel(ohci,0x3e67,&ohci->regs->periodicstart);
	if ((ret = ohci_run(ohci)) < 0) {
		err("can't start %s", hcd->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}

	return 0;
}

static struct hc_driver ohci_rtl8652_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "RTL8652 OHCI",
	.hcd_priv_size		= sizeof(struct ohci_hcd),
	.irq			= ohci_irq,
	.flags			= HCD_USB11 | HCD_MEMORY,
	.start			= ohci_rtl8652_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,
	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,
	.get_frame_number	= ohci_get_frame,
	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,
#if 0
	.hub_irq_enable		= ohci_rhsc_enable,
#endif
#ifdef CONFIG_PM
	.bus_suspend		= ohci_bus_suspend,
	.bus_resume		= ohci_bus_resume,
#endif
	.start_port_reset	= ohci_start_port_reset,
};

extern int usb_disabled(void);

static int ohci_hcd_rtl8652_drv_probe(struct platform_device *pdev)
{
	int ret;

	ret = -ENODEV;
	if (!usb_disabled())
	{
		ret = usb_hcd_rtl8652_probe(&ohci_rtl8652_hc_driver, pdev);
	}
	return ret;
}

static int ohci_hcd_rtl8652_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_rtl8652_remove(hcd, pdev);

	return 0;
}

#ifdef CONFIG_PM
static int ohci_hcd_rtl8652_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct ochi_hcd *ohci = hcd_to_ohci(hcd);

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	rtl8652_stop_hc(&pdev->dev);
	hcd->state = HC_STATE_SUSPENDED;
	pdev->dev.power.power_state = PMSG_SUSPEND;

	return 0;
}

static int ohci_hcd_rtl8652_drv_resume(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int status;

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	rtl8652_start_hc(&pdev->dev);
	pdev->dev.power.power_state = PMSG_ON;
	usb_hcd_resume_root_hub(hcd);

	return 0;
}
#endif

/* work with hotplug and coldplug */
MODULE_ALIAS("platform:rtl8652-ohci");

static struct platform_driver ohci_hcd_rtl8652_driver = {
	.probe		= ohci_hcd_rtl8652_drv_probe,
	.remove		= ohci_hcd_rtl8652_drv_remove,
	.shutdown 	= usb_hcd_platform_shutdown,
#ifdef CONFIG_PM
	.suspend	= ohci_hcd_rtl8652_drv_suspend,
	.resume		= ohci_hcd_rtl8652_drv_resume,
#endif
	.driver		= {
		.name	= "rtl8652-ohci",
		.owner	= THIS_MODULE,
	},
};

/*here register platform rtl8652 usb device.
  do it in kernel boot is a good choice*/
static int  ohci_rtl8652_init(void)
{
	/*register platform device*/
	int retval;
	static struct platform_device *usb_dev_host = NULL;
	struct resource r[2];
	memset(&r, 0, sizeof(r));

	r[0].start = PADDR(OHCI_RTL8652_USB_BASE);
	r[0].end =  PADDR(OHCI_RTL8652_USB_BASE)+sizeof(struct ohci_regs);
	r[0].flags = IORESOURCE_MEM; 

	r[1].start = r[1].end = RTL8652_USB_IRQ;
	r[1].flags = IORESOURCE_IRQ;

	usb_dev_host = platform_device_register_simple("rtl8652-ohci", 0, r, 2);

	usb_dev_host->dev.coherent_dma_mask = 0xffffffffUL;
       usb_dev_host->dev.dma_mask = &usb_dev_host->dev.coherent_dma_mask;
	
	if (IS_ERR(usb_dev_host)) 
	{
		retval = PTR_ERR(usb_dev_host);
		goto err;
	}
	
	return 0;
	err:
	return retval;
}

#if 0
static int __init ohci_hcd_rtl8652_init(void)
{
	ohci_rtl8652_init();
	return platform_driver_register(&ohci_hcd_rtl8652_driver);
}

static void __exit ohci_hcd_rtl8652_cleanup(void)
{
	platform_driver_unregister(&ohci_hcd_rtl8652_driver);
}


module_init(ohci_hcd_rtl8652_init);
module_exit(ohci_hcd_rtl8652_cleanup);
#endif

