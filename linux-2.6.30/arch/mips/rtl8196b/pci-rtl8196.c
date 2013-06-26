/*
 * RTL8196B PCIE Host Controller Glue Driver
 * Author: ghhuang@realtek.com.tw
 *
 * Notes:
 * - Two host controllers available.
 * - Each host direcly connects to one device
 * - Supports PCI devices through PCIE-to-PCI bridges
 * - If no PCI devices are connected to RC. Timeout monitor shall be 
 *   enabled to prevent bus hanging.
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <asm/mipsregs.h>

#include <platform.h>


#define PCI_8BIT_ACCESS    1
#define PCI_16BIT_ACCESS   2
#define PCI_32BIT_ACCESS   4
#define PCI_ACCESS_READ    8
#define PCI_ACCESS_WRITE   16

#define MAX_NUM_DEV  4

#define DEBUG_PRINTK 0


static int pci0_bus_number = 0xff;
static int pci1_bus_number = 0xff;


static struct resource rtl8196b_pci0_io_resource = {
   .name   = "RTL8196B PCI0 IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(PCIE0_D_IO),
   .end    = PADDR(PCIE0_D_IO + 0x1FFFFF)
};

static struct resource rtl8196b_pci0_mem_resource = {
   .name   = "RTL8196B PCI0 MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(PCIE0_D_MEM),
   .end    = PADDR(PCIE0_D_MEM + 0xFFFFFF)
};

static struct resource rtl8196b_pci1_io_resource = {
   .name   = "RTL8196B PCI1 IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(PCIE1_D_IO),
   .end    = PADDR(PCIE1_D_IO + 0x1FFFFFF)
};

static struct resource rtl8196b_pci1_mem_resource = {
   .name   = "RTL8196B PCI1 MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(PCIE1_D_MEM),
   .end    = PADDR(PCIE1_D_MEM + 0xFFFFFF)
};
//#define PIN_208
#define DEMO_8196
static int rtl8196b_pci_reset(void)
{
	cli();
   /* If PCI needs to be reset, put code here.
    * Note: 
    *    Software may need to do hot reset for a period of time, say ~100us.
    *    Here we put 2ms.
    */
#if 1
#ifdef PIN_208
WRITE_MEM32(0xb8000044, 0x358);//Disable  PCIE EX-PLL
#else
//#ifdef DEMO_8196
//WRITE_MEM32(0xb8000044, 0x358);//Disable PCIE EX-PLL
//#else
//Modified for PHY parameter for RD center 12292008
WRITE_MEM32(0xb8000044, 0x9);//Enable  PCIE IN-PLL
//#endif
#endif
mdelay(100);
WRITE_MEM32(0xb8000010, 0x00FFFFD6);//Active LX & PCIE Clock in 8196B system register
mdelay(100);
WRITE_MEM32(0xb800003C, 0x1);//PORT0 PCIE PHY MDIO Reset
mdelay(100);
WRITE_MEM32(0xb800003C, 0x3);//PORT0 PCIE PHY MDIO Reset
mdelay(100);
WRITE_MEM32(0xb8000040, 0x1);//PORT1 PCIE PHY MDIO Reset
mdelay(100);
WRITE_MEM32(0xb8000040, 0x3);//PORT1 PCIE PHY MDIO Reset
mdelay(100);
WRITE_MEM32(0xb8b01008, 0x1);// PCIE PHY Reset Close:Port 0
mdelay(100);
WRITE_MEM32(0xb8b01008, 0x81);// PCIE PHY Reset On:Port 0
mdelay(100);
#ifdef PIN_208
WRITE_MEM32(0xb8b21008, 0x1);// PCIE PHY Reset Close:Port 1
mdelay(100);
WRITE_MEM32(0xb8b21008, 0x81);// PCIE PHY Reset On:Port 1
mdelay(100);
#endif
#if 0//1//1// 1//def OUT_CYSTALL
WRITE_MEM32(0xb8b01000, 0xcc011901);// PCIE Close Port 0
mdelay(10); 
#ifdef PIN_208
WRITE_MEM32(0xb8b21000, 0xcc011901);// PCIE Close Port 1
mdelay(10); 
#endif
#endif
WRITE_MEM32(0xb8000010, 0x01FFFFD6);// PCIE PHY Reset On:Port 1
mdelay(100);   
#endif
   WRITE_MEM32(PCIE0_H_PWRCR, READ_MEM32(PCIE0_H_PWRCR) & 0xFFFFFF7F);
#ifdef PIN_208
   WRITE_MEM32(PCIE1_H_PWRCR, READ_MEM32(PCIE1_H_PWRCR) & 0xFFFFFF7F);
#endif
   mdelay(100);
   WRITE_MEM32(PCIE0_H_PWRCR, READ_MEM32(PCIE0_H_PWRCR) | 0x00000080);
#ifdef PIN_208
   WRITE_MEM32(PCIE1_H_PWRCR, READ_MEM32(PCIE1_H_PWRCR) | 0x00000080);
#endif
   sti();
   return 0;
}


static int rtl8196b_pcibios_config_access(unsigned char access_type,
       unsigned int addr, unsigned int *data)
{
   /* Do 8bit/16bit/32bit access */
   if (access_type & PCI_ACCESS_WRITE)
   {
      if (access_type & PCI_8BIT_ACCESS)
         WRITE_MEM8(addr, *data);
      else if (access_type & PCI_16BIT_ACCESS)
         WRITE_MEM16(addr, *data);
      else
         WRITE_MEM32(addr, *data);
   }
   else if (access_type & PCI_ACCESS_READ)
   {
      if (access_type & PCI_8BIT_ACCESS)
         *data = READ_MEM8(addr);
      else if (access_type & PCI_16BIT_ACCESS)
         *data = READ_MEM16(addr);
      else
         *data = READ_MEM32(addr);
   }

   /* If need to check for PCIE access timeout, put code here */
   /* ... */

   return 0;
}



/*
 * RTL8196b supports config word read access for 8/16/32 bit
 *
 * FIXME: currently only utilize 32bit access
 */
static int rtl8196b_pcibios0_read(struct pci_bus *bus, unsigned int devfn,
                                  int where, int size, unsigned int *val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   if (pci0_bus_number == 0xff)
      pci0_bus_number = bus->number;

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci0_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE0_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            *val = (data >> ((where & 3) << 3)) & 0xff;
         else if (size == 2)
            *val = (data >> ((where & 3) << 3)) & 0xffff;
         else
            *val = data;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci0_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE0_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(PCIE0_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = PCIE0_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Read Value: 0x%08X\n", *val);
   #endif

   return PCIBIOS_SUCCESSFUL;
}


static int rtl8196b_pcibios0_write(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, unsigned int val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   static int pci0_bus_number = 0xff;

   if (pci0_bus_number == 0xff)
      pci0_bus_number = bus->number;

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci0_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE0_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            data = (data & ~(0xff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else if (size == 2)
            data = (data & ~(0xffff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else
            data = val;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE, addr, &data))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci0_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE0_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(PCIE0_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = PCIE0_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   return PCIBIOS_SUCCESSFUL;
}


/*
 * RTL8196b supports config word read access for 8/16/32 bit
 *
 * FIXME: currently only utilize 32bit access
 */
static int rtl8196b_pcibios1_read(struct pci_bus *bus, unsigned int devfn,
                                  int where, int size, unsigned int *val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   if (pci1_bus_number == 0xff)
      pci1_bus_number = bus->number;

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci1_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE1_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            *val = (data >> ((where & 3) << 3)) & 0xff;
         else if (size == 2)
            *val = (data >> ((where & 3) << 3)) & 0xffff;
         else
            *val = data;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci1_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE1_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(PCIE1_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = PCIE1_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Read Value: 0x%08X\n", *val);
   #endif

   return PCIBIOS_SUCCESSFUL;
}


static int rtl8196b_pcibios1_write(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, unsigned int val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   static int pci1_bus_number = 0xff;

   if (pci1_bus_number == 0xff)
      pci1_bus_number = bus->number;

   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif


   if (bus->number == pci1_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE1_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            data = (data & ~(0xff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else if (size == 2)
            data = (data & ~(0xffff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else
            data = val;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE, addr, &data))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else if (bus->number == (pci1_bus_number + 1))
   {
      /* PCIE devices directly connected */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = PCIE1_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }
   else
   {
      /* Devices connected through bridge */
      if (PCI_SLOT(devfn) < MAX_NUM_DEV)
      {
         WRITE_MEM32(PCIE1_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = PCIE1_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   return PCIBIOS_SUCCESSFUL;
}

struct pci_ops rtl8196b_pci0_ops = {
   .read = rtl8196b_pcibios0_read,
   .write = rtl8196b_pcibios0_write
};

struct pci_ops rtl8196b_pci1_ops = {
   .read = rtl8196b_pcibios1_read,
   .write = rtl8196b_pcibios1_write
};


static struct pci_controller rtl8196b_pci0_controller = {
   .pci_ops        = &rtl8196b_pci0_ops,
   .mem_resource   = &rtl8196b_pci0_mem_resource,
   .io_resource    = &rtl8196b_pci0_io_resource,
};

static struct pci_controller rtl8196b_pci1_controller = {
   .pci_ops        = &rtl8196b_pci1_ops,
   .mem_resource   = &rtl8196b_pci1_mem_resource,
   .io_resource    = &rtl8196b_pci1_io_resource,
};


int pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("**Slot: %d\n", slot);
   printk("**Pin: %d\n", pin);
   printk("**Dev->BUS->Number: %d\n", dev->bus->number);
   #endif

   if (dev->bus->number < pci1_bus_number)
      return PCIE_IRQ;
   else
      return PCIE2_IRQ;
}

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   #endif

   return 0;
}


static __init int rtl8196b_pci_init(void)
{
   rtl8196b_pci_reset();

   set_io_port_base(KSEG1);
   ioport_resource.end = 0xFFFFFFFF;

   #if DEBUG_PRINTK
   printk("<<<<<Register 1st PCI Controller>>>>>\n");
   printk("<<<<<Register 2nd PCI Controller>>>>>\n");
   #endif

   register_pci_controller(&rtl8196b_pci0_controller);
#ifdef PIN_208   
	register_pci_controller(&rtl8196b_pci1_controller);
#endif
   return 0;
}

arch_initcall(rtl8196b_pci_init);
