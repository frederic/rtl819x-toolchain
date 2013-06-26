/*
 * Realtek Semiconductor Corp.
 *
 * ehci-dwc.c: DWC USB host controller
 *
 * Tony Wu (tonywu@realtek.com)
 * Jan. 10, 2009
 */

#include <linux/platform_device.h>

#define EHCI_DWC_NAME  "dwc_ehci"

/* called during probe() after chip reset completes */
static int
ehci_dwc_setup (struct usb_hcd *hcd)
{
  struct ehci_hcd *ehci = hcd_to_ehci (hcd);
  int retval = 0;

  retval = ehci_halt (ehci);
  if (retval)
    return retval;

  /* data structure init */
  retval = ehci_init (hcd);
  if (retval)
    return retval;

  if (ehci_is_TDI (ehci))
    ehci_reset (ehci);

#ifdef	CONFIG_USB_SUSPEND
  /* REVISIT: the controller works fine for wakeup iff the root hub
   * itself is "globally" suspended, but usbcore currently doesn't
   * understand such things.
   *
   * System suspend currently expects to be able to suspend the entire
   * device tree, device-at-a-time.  If we failed selective suspend
   * reports, system suspend would fail; so the root hub code must claim
   * success.  That's lying to usbcore, and it matters for for runtime
   * PM scenarios with selective suspend and remote wakeup...
   */
  if (ehci->no_selective_suspend && device_can_wakeup (&pdev->dev))
    ehci_warn (ehci, "selective suspend/wakeup unavailable\n");
#endif

  ehci_port_power(ehci, 0);
  return retval;
}

struct hc_driver ehci_dwc_hc_driver = {
  .description = hcd_name,
  .product_desc = "EHCI Host Controller",
  .hcd_priv_size = sizeof (struct ehci_hcd),
  /*
   * generic hardware linkage
   */
  .irq = ehci_irq,
  .flags = HCD_MEMORY | HCD_USB2,

  /*
   * basic lifecycle operations
   */
  .reset = ehci_dwc_setup,
  .start = ehci_run,
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

#ifdef CONFIG_PM
  .bus_suspend = ehci_bus_suspend,
  .bus_resume = ehci_bus_resume,
#endif

  .relinquish_port    = ehci_relinquish_port,
  .port_handed_over   = ehci_port_handed_over,
};

int
ehci_dwc_drv_probe (struct platform_device *pdev)
{
  struct hc_driver *phcd;
  struct usb_hcd *hcd;
  struct ehci_hcd *ehci;
  struct resource *res;
  int retval;
  u32 dwc_usb_irq; 

  phcd = &ehci_dwc_hc_driver;

  res = platform_get_resource (pdev, IORESOURCE_IRQ, 0);
  if (res == NULL)
    {
      printk (KERN_ERR __FILE__ ": get irq resource failed!\n");
      retval = -ENOMEM;
      return retval;
    }

  dwc_usb_irq = res->start;

  hcd = usb_create_hcd (phcd, &pdev->dev, dev_name(&pdev->dev));
  if (hcd == NULL)
    {
      retval = -ENOMEM;
      return retval;
    }

  res = platform_get_resource (pdev, IORESOURCE_MEM, 0);
  if (res == NULL)
    {
      printk (KERN_ERR __FILE__ ": get memory resource failed!\n");
      retval = -ENOMEM;
      return retval;
    }

  hcd->rsrc_start = res->start;
  hcd->rsrc_len = res->end - res->start + 1;
  if (request_mem_region (hcd->rsrc_start, hcd->rsrc_len, EHCI_DWC_NAME) == NULL)
    {
      printk (KERN_ERR __FILE__ ": request_mem_region failed\n");
      retval = -ENOMEM; 
      return retval;
    }

  hcd->regs = (void *) ioremap(hcd->rsrc_start, hcd->rsrc_len);
  if (!hcd->regs)
    { 
      printk(KERN_ERR __FILE__ ": ioremap failed\n");
      release_mem_region (hcd->rsrc_start, hcd->rsrc_len);
      retval = -ENOMEM; 
      return retval;
    }

  ehci = hcd_to_ehci (hcd);
  ehci->caps = hcd->regs;
  ehci->regs = hcd->regs + HC_LENGTH (readl (&ehci->caps->hc_capbase));
  ehci->hcs_params = readl (&ehci->caps->hcs_params);

  retval = usb_add_hcd (hcd, dwc_usb_irq, IRQF_SHARED);
  if (retval != 0)
    {
      release_mem_region (hcd->rsrc_start, hcd->rsrc_len);
      iounmap(hcd->regs);
    }

  return retval;
}

static int
ehci_dwc_drv_remove (struct platform_device *pdev)
{
  struct usb_hcd *hcd;

  hcd = platform_get_drvdata (pdev);
  usb_remove_hcd (hcd);
  if (hcd->driver->flags & HCD_MEMORY)
    {
      iounmap (hcd->regs);
      release_mem_region (hcd->rsrc_start, hcd->rsrc_len);
    }
  else
    release_region (hcd->rsrc_start, hcd->rsrc_len);

  usb_put_hcd (hcd);
  return 0;
}

struct platform_driver ehci_dwc_driver = {
  .probe = ehci_dwc_drv_probe,
  .remove = ehci_dwc_drv_remove,
  .shutdown = usb_hcd_platform_shutdown,
  .driver = {
    .name = "dwc_ehci",
  },
};
