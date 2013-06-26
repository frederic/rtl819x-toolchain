/*
 * (C) Copyright David Brownell 2000-2002
 * Copyright (c) 2005 MontaVista Software
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Ported to 834x by Randy Vinson <rvinson@mvista.com> using code provided
 * by Hunter Wu.
 */

#include <linux/platform_device.h>
//#include <linux/rtl8652_devices.h>

#include "bspchip.h"
#include "usb-rtl8652.h"

/* FIXME: Power Managment is un-ported so temporarily disable it */
#undef CONFIG_PM

/* PCI-based HCs are common, but plenty of non-PCI HCs are used too */

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

/**
 * usb_hcd_rtl8652_probe - initialize FSL-based HCDs
 * @drvier: Driver to be used for this HCD
 * @pdev: USB Host Controller being probed
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller.
 *
 */
int usb_hcd_rtl8652_probe(const struct hc_driver *driver,
		      struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	int irq;
	int retval;

	pr_debug("initializing RTL-SOC USB Controller\n");

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no IRQ. Check %s setup!\n",
			dev_name(&pdev->dev));
		return -ENODEV;
	}
	irq = res->start;
	hcd = usb_create_hcd(driver, &pdev->dev,dev_name(&pdev->dev));
	if (!hcd) {
		retval = -ENOMEM;
		goto err1;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no register addr. Check %s setup!\n",
			dev_name(&pdev->dev));
		retval = -ENODEV;
		goto err2;
	}
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;
	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len,
				driver->description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto err2;
	}
	hcd->regs = (void *) KSEG1ADDR(hcd->rsrc_start);
        //hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);

	if (hcd->regs == NULL) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto err3;
	}

	
	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval != 0)
		goto err4;

	return retval;

      err4:
	iounmap(hcd->regs);
      err3:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
      err2:
	usb_put_hcd(hcd);
      err1:
	dev_err(&pdev->dev, "init %s fail, %d\n", dev_name(&pdev->dev), retval);
	return retval;
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_hcd_rtl8652_remove - shutdown processing for FSL-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_hcd_rtl8652_probe().
 *
 */
void usb_hcd_rtl8652_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

#if 0
static void mpc83xx_setup_phy(struct ehci_hcd *ehci,
			      enum rtl8652_usb2_phy_modes phy_mode,
			      unsigned int port_offset)
{
	u32 portsc = 0;
	switch (phy_mode) {
	case FSL_USB2_PHY_ULPI:
		portsc |= PORT_PTS_ULPI;
		break;
	case FSL_USB2_PHY_SERIAL:
		portsc |= PORT_PTS_SERIAL;
		break;
	case FSL_USB2_PHY_UTMI_WIDE:
		portsc |= PORT_PTS_PTW;
		/* fall through */
	case FSL_USB2_PHY_UTMI:
		portsc |= PORT_PTS_UTMI;
		break;
	case FSL_USB2_PHY_NONE:
		break;
	}
	writel(portsc, &ehci->regs->port_status[port_offset]);
}
static void mpc83xx_usb_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	struct rtl8652_usb2_platform_data *pdata;
	void __iomem *non_ehci = hcd->regs;

	pdata =
	    (struct rtl8652_usb2_platform_data *)hcd->self.controller->
	    platform_data;
	/* Enable PHY interface in the control reg. */
	out_be32(non_ehci + FSL_SOC_USB_CTRL, 0x00000004);
	out_be32(non_ehci + FSL_SOC_USB_SNOOP1, 0x0000001b);

	if (pdata->operating_mode == FSL_USB2_DR_HOST)
		mpc83xx_setup_phy(ehci, pdata->phy_mode, 0);

	if (pdata->operating_mode == FSL_USB2_MPH_HOST) {
		unsigned int chip, rev, svr;

		svr = mfspr(SPRN_SVR);
		chip = svr >> 16;
		rev = (svr >> 4) & 0xf;

		/* Deal with USB Erratum #14 on MPC834x Rev 1.0 & 1.1 chips */
		if ((rev == 1) && (chip >= 0x8050) && (chip <= 0x8055))
			ehci->has_rtl8652_port_bug = 1;

		if (pdata->port_enables & FSL_USB2_PORT0_ENABLED)
			mpc83xx_setup_phy(ehci, pdata->phy_mode, 0);
		if (pdata->port_enables & FSL_USB2_PORT1_ENABLED)
			mpc83xx_setup_phy(ehci, pdata->phy_mode, 1);
	}

	/* put controller in host mode. */
	writel(0x00000003, non_ehci + FSL_SOC_USB_USBMODE);
	out_be32(non_ehci + FSL_SOC_USB_PRICTRL, 0x0000000c);
	out_be32(non_ehci + FSL_SOC_USB_AGECNTTHRSH, 0x00000040);
	out_be32(non_ehci + FSL_SOC_USB_SICTRL, 0x00000001);
}
#endif
static int synopsys_usb_setup(struct ehci_hcd *ehci)
{
#if 1  //tony: patch for synopsys 
	printk("read synopsys=%x\n",readl (&ehci->regs->command+0x84));
	writel(0x01fd0020,&ehci->regs->command+0x84);
	printk("read synopsys2=%x\n",readl (&ehci->regs->command+0x84));
#endif
    ehci->broken_periodic = 1;
    ehci->need_io_watchdog = 1;     // does RT EHCI needs I/O watch dog ?
	return 0;
}

/* called after powerup, by probe or system-pm "wakeup" */
static int ehci_rtl8652_reinit(struct ehci_hcd *ehci)
{
	synopsys_usb_setup(ehci);
	ehci_port_power(ehci, 0);
	return 0;
}

/* called during probe() after chip reset completes */
static int ehci_rtl8652_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval;
	/* EHCI registers start */
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs +
	    HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));

	
	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	retval = ehci_halt(ehci);
	if (retval)
		return retval;

	/* data structure init */
	retval = ehci_init(hcd);
	if (retval)
		return retval;

	ehci_reset(ehci);

	retval = ehci_rtl8652_reinit(ehci);
	return retval;
}

static const struct hc_driver ehci_rtl8652_hc_driver = {
	.description = hcd_name,
	.product_desc = "RTL8652 On-Chip EHCI Host Controller",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq = ehci_irq,
	.flags = HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	.reset = ehci_rtl8652_setup,
	.start = ehci_run,
#ifdef	CONFIG_PM
	.suspend = ehci_bus_suspend,
	.resume = ehci_bus_resume,
#endif
	.stop = ehci_stop,
	.shutdown = ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number = ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data = ehci_hub_status_data,
	.hub_control = ehci_hub_control,
	.bus_suspend = ehci_bus_suspend,
	.bus_resume = ehci_bus_resume,
	.relinquish_port = ehci_relinquish_port,
	.port_handed_over = ehci_port_handed_over,
	.clear_tt_buffer_complete     = ehci_clear_tt_buffer_complete,
};

static int ehci_rtl8652_drv_probe(struct platform_device *pdev)
{
	if (usb_disabled())
		return -ENODEV;

	return usb_hcd_rtl8652_probe(&ehci_rtl8652_hc_driver, pdev);
}

static int ehci_rtl8652_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_rtl8652_remove(hcd, pdev);

	return 0;
}

MODULE_ALIAS("platform:rtl8652-ehci");

static struct platform_driver ehci_rtl8652_driver = {
	.probe = ehci_rtl8652_drv_probe,
	.remove = ehci_rtl8652_drv_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		   .name = "rtl8652-ehci",
		   },
};
#if defined(CONFIG_RTL_8196C)
void SetUSBPhy(unsigned char reg, unsigned char val)
{
	
	#define	USB2_PHY_DELAY	{mdelay(5);}
	//8196C demo board: 0xE0:99, 0xE1:A8, 0xE2:98, 0xE3:C1,  0xE5:91, 	
	REG32(0xb8000034) = (0x1f00 | val); USB2_PHY_DELAY;
	//printk("0xb8000034=%08x\n", REG32(0xb8000034));		
	
	unsigned char reg_h=(reg &0xf0)>>4;
	unsigned char reg_l=(reg &0x0f);
		
	mdelay(100);	
	REG32(0xb80210A4) = (0x00300000 | (reg_l<<16)); USB2_PHY_DELAY;
	REG32(0xb80210A4) = (0x00200000 | (reg_l<<16)); USB2_PHY_DELAY;	
	REG32(0xb80210A4) = (0x00300000 | (reg_l<<16)); USB2_PHY_DELAY;	
	REG32(0xb80210A4) = (0x00300000 | (reg_h<<16)); USB2_PHY_DELAY;	
	REG32(0xb80210A4) = (0x00200000 | (reg_h<<16)); USB2_PHY_DELAY;	
	REG32(0xb80210A4) = (0x00300000 | (reg_h<<16)); USB2_PHY_DELAY;
	
}

unsigned char GetUSBPhy(unsigned char reg)
{
	#define	USB2_PHY_DELAY	{mdelay(5);}

	unsigned char reg_h=((reg &0xf0)>>4)-2;
	unsigned char reg_l=(reg &0x0f);
		
	REG32(0xb80210A4) = (0x00300000 | (reg_l<<16)); USB2_PHY_DELAY;
	REG32(0xb80210A4) = (0x00200000 | (reg_l<<16)); USB2_PHY_DELAY;
	REG32(0xb80210A4) = (0x00300000 | (reg_l<<16)); USB2_PHY_DELAY;	
	REG32(0xb80210A4) = (0x00300000 | (reg_h<<16)); USB2_PHY_DELAY;	
	REG32(0xb80210A4) = (0x00200000 | (reg_h<<16)); USB2_PHY_DELAY;	
	REG32(0xb80210A4) = (0x00300000 | (reg_h<<16)); USB2_PHY_DELAY;	
	
	unsigned char val;
	val=REG32(0xb80210A4)>>24;
	//printk("reg=%x val=%x\n",reg, val);
	return val;
}
#endif

static void synopsys_usb_patch(void)
{

#ifndef REG32(reg)
#define REG32(reg)	(*(volatile unsigned int *)(reg))
#endif

#define	USB2_PHY_DELAY	{int i=100; while(i>0) {i--;}}
	/* Patch: for USB 2.0 PHY */
#if !defined(CONFIG_RTL_8196C)
#if 0
	/* For Port-0 */
	writel(0x0000000E,0xb8003314) ;	USB2_PHY_DELAY;
	writel(0x00340000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00240000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00340000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x003E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x002E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x003E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x000000D8,0xb8003314) ;	USB2_PHY_DELAY;
	writel(0x00360000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00260000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00360000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x003E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x002E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x003E0000,0xb80210A4) ;	USB2_PHY_DELAY;

	/* For Port-1 */
	writel(0x000E0000,0xb8003314) ;	USB2_PHY_DELAY;
	writel(0x00540000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00440000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00540000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x005E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x004E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x005E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00D80000,0xb8003314) ;	USB2_PHY_DELAY;
	writel(0x00560000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00460000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x00560000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x005E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x004E0000,0xb80210A4) ;	USB2_PHY_DELAY;
	writel(0x005E0000,0xb80210A4) ;	USB2_PHY_DELAY;
#else
	/* For Port-0 */
	REG32(0xb8003314) = 0x0000000E;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00340000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00240000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00340000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x003E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x002E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x003E0000;	USB2_PHY_DELAY;
	REG32(0xb8003314) = 0x000000D8;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00360000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00260000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00360000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x003E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x002E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x003E0000;	USB2_PHY_DELAY;

	/* For Port-1 */
	REG32(0xb8003314) = 0x000E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00540000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00440000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00540000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x005E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x004E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x005E0000;	USB2_PHY_DELAY;
	REG32(0xb8003314) = 0x00D80000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00560000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00460000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x00560000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x005E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x004E0000;	USB2_PHY_DELAY;
	REG32(0xb80210A4) = 0x005E0000;	USB2_PHY_DELAY;
	
	printk("USB 2.0 Phy Patch(D): &0xb80210A4 = %08x\n", REG32(0xb80210A4));	/* A85E0000 */	
#endif
#elif defined(CONFIG_RTL_8196C)

	//disable Host chirp J-K
	SetUSBPhy(0xf4,0xe3);	GetUSBPhy(0xf4);
	//8196C demo board: 0xE0:99, 0xE1:A8, 0xE2:98, 0xE3:C1,  0xE5:91, 
	SetUSBPhy(0xe0,0x99);   if(GetUSBPhy(0xe0)!=0x99) printk("reg 0xe0 not correct\n");
	SetUSBPhy(0xe1,0xa8);	if(GetUSBPhy(0xe1)!=0xa8) printk("reg 0xe1 not correct\n");
	SetUSBPhy(0xe2,0x98);	if(GetUSBPhy(0xe2)!=0x98) printk("reg 0xe2 not correct\n");
	SetUSBPhy(0xe3,0xc1);	if(GetUSBPhy(0xe3)!=0xc1) printk("reg 0xe3 not correct\n");
	SetUSBPhy(0xe5,0x91);	if(GetUSBPhy(0xe5)!=0x91) printk("reg 0xe5 not correct\n");			
	
	//test packet.	
	/*
	REG32(0xb8021054)=0x85100000;
	REG32(0xb8021010)=0x08000100;
	REG32(0xb8021054)=0x85180400;
	*/
	printk("USB 2.0 PHY Patch Done.\n");

#else
	printk("========================== NO PATCH USB 2.0 PHY =====================\n");
#endif
	return;
}

/*here register platform rtl8652 usb device.
  do it in kernel boot is a good choice*/
 static struct platform_device *usb_dev_host = NULL;  //wei add
 //----------------------------------------------------------------------
static int  ehci_rtl8652_init(void)
{
	 REG32(0xb8000010)=REG32(0xb8000010)|0x20000;
	/*register platform device*/
	int retval;
	//static struct platform_device *usb_dev_host = NULL;

	if(usb_dev_host!=NULL)
	{	printk("Ehci-rtl8652.c: EHCI device already init\n");
		return -1;
	}


	struct resource r[2];
	memset(&r, 0, sizeof(r));

	r[0].start = PADDR(EHCI_RTL8652_USB_BASE);
	r[0].end = PADDR(EHCI_RTL8652_USB_BASE)+sizeof(struct ehci_regs);
	r[0].flags = IORESOURCE_MEM; 
	
	r[1].start = r[1].end = RTL8652_USB_IRQ;
	r[1].flags = IORESOURCE_IRQ;

	usb_dev_host = platform_device_register_simple("rtl8652-ehci",0, r, 2);

	usb_dev_host->dev.coherent_dma_mask = 0xffffffffUL;
      	usb_dev_host->dev.dma_mask = &usb_dev_host->dev.coherent_dma_mask;
	
	if (IS_ERR(usb_dev_host)) 
	{
		retval = PTR_ERR(usb_dev_host);
		usb_dev_host=NULL;  //wei add
		goto err;
	}
#if !defined(CONFIG_RTL_819XD)
#if defined(CONFIG_RTL_8198)
#ifdef CONFIG_RTL8198_REVISION_B
	// rock: pin_mux for USB over-current detection in rtl8198_rev_a
	if (REG32(BSP_REVR) == BSP_RTL8198_REVISION_A)
		REG32(0xb8000040) = 0x03;
#else
	// rock: pin_mux for USB over-current detection
	REG32(0xb8000040) = 0x03;
#endif
#else
	synopsys_usb_patch();
#endif
#endif

	return 0;
	err:
	return retval;
}
//----------------------------------------------------------------------
void  ehci_rtl8652_cleanup(void)
{
	if(usb_dev_host==NULL)
	{
		printk("EHCI already cleanup\n");
		return;
	}
	
	platform_device_unregister(	usb_dev_host);
	usb_dev_host=NULL;
	
}

