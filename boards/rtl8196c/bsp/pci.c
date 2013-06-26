/*
 * RTL8196C PCIE Host Controller Glue Driver
 * Author: ghhuang@realtek.com.tw
 *
 * Notes:
 * - Two host controllers available.
 * - Each host direcly connects to one device
 * - Supports PCI devices through PCIE-to-PCI bridges
 * - If no PCI devices are connected to RC. Timeout monitor shall be 
 *   enabled to prevent bus hanging.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
//#include <asm/rlxregs.h>
#include "bspchip.h"

#define PCI_8BIT_ACCESS    1
#define PCI_16BIT_ACCESS   2
#define PCI_32BIT_ACCESS   4
#define PCI_ACCESS_READ    8
#define PCI_ACCESS_WRITE   16

#define MAX_NUM_DEV  4

#define DEBUG_PRINTK 0

#define CLK_MANAGE 	0xb8000010
#define SYS_PCIE_PHY0   (0xb8000000 +0x50)
#define SYS_PCIE_PHY1   (0xb8000000 +0x54)
#define PCIE_PHY0 	0xb8b01008
#define PCIE_PHY1 	0xb8b21008

#define MAX_PAYLOAD_SIZE_128B 0

static int pci0_bus_number = 0xff;

static struct resource rtl8196b_pci0_io_resource = {
   .name   = "RTL8196C PCI0 IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(BSP_PCIE0_D_IO),
   .end    = PADDR(BSP_PCIE0_D_IO + 0x1FFFFF)
};

static struct resource rtl8196b_pci0_mem_resource = {
   .name   = "RTL8196C PCI0 MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(BSP_PCIE0_D_MEM),
   .end    = PADDR(BSP_PCIE0_D_MEM + 0xFFFFFF)
};

//HOST PCIE
#define PCIE0_RC_EXT_BASE (0xb8b01000)
//RC Extended register
#define PCIE0_MDIO	(PCIE0_RC_EXT_BASE+0x00)
//MDIO
#define PCIE_MDIO_DATA_OFFSET (16)
#define PCIE_MDIO_DATA_MASK (0xffff <<PCIE_MDIO_DATA_OFFSET)
#define PCIE_MDIO_REG_OFFSET (8)
#define PCIE_MDIO_RDWR_OFFSET (0)
 

//------------------------------------------------------------------------
unsigned int HostPCIe_SetPhyMdioRead(unsigned int portnum, unsigned int regaddr)
{
	unsigned int mdioaddr=PCIE0_MDIO;
/*
	if(portnum==0)		mdioaddr=PCIE0_MDIO;	
	else if(portnum==1)	mdioaddr=PCIE1_MDIO;
	else return 0;
*/	
	REG32(mdioaddr)= ((regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET)  | (0<<PCIE_MDIO_RDWR_OFFSET); 
	//delay 
	volatile int i;
	for(i=0;i<5555;i++)  ;

	int val;
	val=REG32(mdioaddr)&  (0xffff <<PCIE_MDIO_DATA_OFFSET) ;
	return ((val>>PCIE_MDIO_DATA_OFFSET)&0xffff);
	
}


void HostPCIe_SetPhyMdioWrite(unsigned int portnum, unsigned int regaddr, unsigned short val)
{
	unsigned int mdioaddr;

	mdioaddr=PCIE0_MDIO;	
	
	REG32(mdioaddr)= ( (regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET) | ((val&0xffff)<<PCIE_MDIO_DATA_OFFSET)  | (1<<PCIE_MDIO_RDWR_OFFSET) ; 
	//delay 
	volatile int i;
	for(i=0;i<5555;i++)  ;
}

//----------------------------------------------------------------------------

void PCIE_MDIO_Reset(unsigned int portnum)
{
	unsigned int sys_pcie_phy;

	if (portnum==0)
		sys_pcie_phy=SYS_PCIE_PHY0;
	else if (portnum==1)
		sys_pcie_phy=SYS_PCIE_PHY1;
	else
		return;
		
	// 3.MDIO Reset
	REG32(sys_pcie_phy) = (1<<3) |(0<<1) | (0<<0);     //mdio reset=0,     	    
	REG32(sys_pcie_phy) = (1<<3) |(0<<1) | (1<<0);     //mdio reset=1, 
	REG32(sys_pcie_phy) = (1<<3) |(1<<1) | (1<<0);     //bit1 load_done=1
}
//------------------------------------------------------------------------
void PCIE_PHY_Reset(unsigned int portnum)
{
	unsigned int pcie_phy;

	if(portnum==0)	pcie_phy=PCIE_PHY0;
	else if(portnum==1)	pcie_phy=PCIE_PHY1;
	else return;

        //4. PCIE PHY Reset       
	REG32(pcie_phy) = 0x01;	//bit7:PHY reset=0   bit0: Enable LTSSM=1
	REG32(pcie_phy) = 0x81;   //bit7: PHY reset=1   bit0: Enable LTSSM=1
	
}
//------------------------------------------------------------------------
int PCIE_Check_Link(unsigned int portnum)
{
	unsigned int dbgaddr;
	unsigned int cfgaddr;
	
	if(portnum==0)	dbgaddr=0xb8b00728;
	else if(portnum==1)	dbgaddr=0xb8b20728;
	else return;	

  //wait for LinkUP
#ifdef CONFIG_RTK_VOIP
	// accelerate when no pcie card 
	int i=3;
#else
	int i=20;
#endif
	while(--i)
	{
	      if( (REG32(dbgaddr)&0x1f)==0x11)
		  	break;
      		mdelay(300);		  

	}
	if(i==0)
	{
		printk("i=%x  Cannot LinkUP \n",i);
		return 0;
	}
	else
	{
		printk("\nLink-UP OK\n");
		cfgaddr=0xb8b10000;

		REG32(cfgaddr+0x04)=0x00100007;

		printk("Find Port=%x Device:Vender ID=%x\n", portnum, REG32(cfgaddr) );
		REG32(cfgaddr);
		mdelay(1);
	}
	return 1;
}
//------------------------------------------------------------------------
void PCIE_Device_PERST(void)
{
	REG32(CLK_MANAGE) &= ~(1<<12);    //perst=0 off.  
	mdelay(500);   //PCIE standadrd: poweron: 100us, after poweron: 100ms
	mdelay(500);  		
	REG32(CLK_MANAGE) |=  (1<<12);   //PERST=1
	mdelay(500);
}

//=====================================================================
int  PCIE_reset_procedure(int portnum, int Use_External_PCIE_CLK, int mdio_reset)
{
	int result;
	 
	REG32(CLK_MANAGE) |=  (1<<11);        //enable active_pcie0

	mdelay(10);
	REG32(0xb8b0100c)=(1<<3);  //set target Device Num=1;
	mdelay(10);

	if (mdio_reset) {
		printk("Do MDIO_RESET\n");
		// 3.MDIO Reset
		PCIE_MDIO_Reset(portnum);
		mdelay(10);
	}
	//4. PCIE PHY Reset       
	PCIE_PHY_Reset(portnum);
	mdelay(10);
	mdelay(10);
 
	//----------------------------------------
	if (mdio_reset) {
		HostPCIe_SetPhyMdioWrite(portnum, 0, 0xD087);  //bokai tell, and fix

		HostPCIe_SetPhyMdioWrite(portnum, 1, 0x0003);
		HostPCIe_SetPhyMdioWrite(portnum, 2, 0x4d18);
#ifdef CONFIG_PHY_EAT_40MHZ
#ifdef CONFIG_HIGH_POWER_EXT_PA
		HostPCIe_SetPhyMdioWrite(portnum, 5, 0x0BF3);   //40M
#else
		HostPCIe_SetPhyMdioWrite(portnum, 5, 0x0BCB);   //40M
#endif
#endif

#ifdef  CONFIG_PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(portnum, 6, 0xF148);  //40M
#else
		HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf848);  //25M
#endif

		HostPCIe_SetPhyMdioWrite(portnum, 7, 0x31ff);
		HostPCIe_SetPhyMdioWrite(portnum, 8, 0x18d7);  //peisi tune

#if 0       //old,		
		HostPCIe_SetPhyMdioWrite(portnum, 9, 0x531c); 		
		HostPCIe_SetPhyMdioWrite(portnum, 0xd, 0x1766); //peisi tune
#else     //saving more power, 8196c pe-si tune
		HostPCIe_SetPhyMdioWrite(portnum, 0x09, 0x539c); 	
		HostPCIe_SetPhyMdioWrite(portnum, 0x0a, 0x20eb); 	
		HostPCIe_SetPhyMdioWrite(portnum, 0x0d, 0x1764); 			
#endif
		HostPCIe_SetPhyMdioWrite(portnum, 0x0b, 0x0511);   //for sloving low performance

		HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0a00);	
		HostPCIe_SetPhyMdioWrite(portnum, 0x19, 0xFCE0);
		 
		HostPCIe_SetPhyMdioWrite(portnum, 0x1e, 0xC280);	
	}
 
	//---------------------------------------
	PCIE_Device_PERST();

	PCIE_PHY_Reset(portnum);	  
	mdelay(500);
	REG32(0xb8b00000 + 0x04)= 0x00100007;
	REG8(0xb8b00000 + 0x78)=((REG8(0xb8b00000 + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);
	mdelay(500);
	result=PCIE_Check_Link(portnum);
	return result;
}
//========================================================================================

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


//========================================================================================
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
	//printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   //printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci0_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE0_H_CFG + where;

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
         addr = BSP_PCIE0_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

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
         WRITE_MEM32(BSP_PCIE0_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = BSP_PCIE0_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   #if DEBUG_PRINTK
	//printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   //printk("Read Value: 0x%08X\n", *val);
   #endif

   return PCIBIOS_SUCCESSFUL;
}

//========================================================================================
static int rtl8196b_pcibios0_write(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, unsigned int val)
{
   unsigned int data = 0;
   unsigned int addr = 0;

   static int pci0_bus_number = 0xff;
   if (pci0_bus_number == 0xff)
      pci0_bus_number = bus->number;

   #if DEBUG_PRINTK
   //printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   //printk("Bus: %d, Slot: %d, Func: %d, Where: %d, Size: %d\n", bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
   #endif

   if (bus->number == pci0_bus_number)
   {
      /* PCIE host controller */
      if (PCI_SLOT(devfn) == 0)
      {
         addr = BSP_PCIE0_H_CFG + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_READ | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
            return PCIBIOS_DEVICE_NOT_FOUND;

         if (size == 1)
            data = (data & ~(0xff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else if (size == 2)
            data = (data & ~(0xffff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
         else
            data = val;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | PCI_32BIT_ACCESS, addr & ~(0x3), &data))
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
         addr = BSP_PCIE0_D_CFG0 + (PCI_FUNC(devfn) << 12) + where;

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
         WRITE_MEM32(BSP_PCIE0_H_IPCFG, ((bus->number) << 8) | (PCI_SLOT(devfn) << 3) | PCI_FUNC(devfn));
         addr = BSP_PCIE0_D_CFG1 + where;

         if (rtl8196b_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
            return PCIBIOS_DEVICE_NOT_FOUND;
      }
      else
         return PCIBIOS_DEVICE_NOT_FOUND;
   }

   return PCIBIOS_SUCCESSFUL;
}
//========================================================================================

//========================================================================================
struct pci_ops rtl8196b_pci0_ops = {
   .read = rtl8196b_pcibios0_read,
   .write = rtl8196b_pcibios0_write
};

static struct pci_controller rtl8196b_pci0_controller = {
   .pci_ops        = &rtl8196b_pci0_ops,
   .mem_resource   = &rtl8196b_pci0_mem_resource,
   .io_resource    = &rtl8196b_pci0_io_resource,
};

//========================================================================================
int pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   printk("**Slot: %d\n", slot);
   printk("**Pin: %d\n", pin);
   printk("**Dev->BUS->Number: %d\n", dev->bus->number);
   #endif

   return BSP_PCIE_IRQ;
}
//========================================================================================
/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
   #if DEBUG_PRINTK
   printk("File: %s, Function: %s, Line: %d\n", __FILE__, __FUNCTION__, __LINE__);
   #endif

   return 0;
}

static __init int bsp_pcie_init(void)
{

	int result=0;

	printk("<<<<<Register 1st PCI Controller>>>>>\n");
	mdelay(1);

	result=PCIE_reset_procedure(0, 0, 1);
	if (result)
		register_pci_controller(&rtl8196b_pci0_controller);
	else
		REG32(CLK_MANAGE) &=  (~(1<<11));        //disable active_pcie0
	return 0;
}

arch_initcall(bsp_pcie_init);
