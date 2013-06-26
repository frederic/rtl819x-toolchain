/*
 *	Realtek IIS Controller Driver Header File
 *
 *
 *
 *
 *
 *
 */

#ifndef _IIS_INTERFACE
#define _IIS_INTERFACE

#include <linux/config.h>
#include "rtk_voip.h"

//#define CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
//#define CONFIG_RTK_VOIP_DRIVERS_IP_PHONE

/*******************  Function Prototype ********************/
//void IIS_init(void);
//void IIS_shutdown(void);
//void IIS_restart(unsigned int chid);
//void iis_enableChan(unsigned int chid);
//void iis_disableChan(unsigned int chid);


/* clean interrupt pending bits */
//void iis_clean_isr(unsigned int statusval);

/* Get the Tx, Rx base address */
//unsigned int iis_get_tx_base_addr(unsigned int chid);
//unsigned int iis_get_rx_base_addr(unsigned int chid);

//void iis_enable(void);
//void iis_disable(void);

//void iis_tx_enable(unsigned int chid);
//void iis_tx_disable(unsigned int chid);
//void iis_rx_enable(unsigned int chid);
//void iis_rx_disable(unsigned int chid);

//void iis_imr_enable(unsigned int chid, unsigned char type);
//void iis_imr_disable(unsigned int chid, unsigned char type);
//void print_iis(void);
//void iis_set_tx_mute(unsigned char chid, int enable);
//void iis_set_rx_mute(unsigned char chid, int enable);

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM865xC

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
#define IIS_IRQ	26
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
  #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#include "bspchip.h"
#define IIS_IRQ BSP_I2S_IRQ
  #else
#define IIS_IRQ	26
  #endif
#endif

/* Interrupt Mask Type */
#define P0OK_TX		0
#define P1OK_TX		1
#define P2OK_TX		2
#define P3OK_TX		3
#define TPUA		4
#define TFEM		5
#define P0OK_RX		6
#define P1OK_RX		7
#define P2OK_RX		8
#define P3OK_RX		9
#define RPUA		10
#define RFFU		11


#define iis_outb(address, value)	writeb(value, address)
#define iis_outw(address, value)	writew(value, address)
#define iis_outl(address, value)	writel(value, address)

#define iis_inb(address)		readb(address)
#define iis_inw(address)		readw(address)
#define iis_inl(address)		readl(address)

#ifdef REG32
#undef REG32
#endif
#define REG32(reg) (*(volatile unsigned int *)(reg))

#define rtl_outl(address, value)	(REG32(address) = value)
#define rtl_inl(address)			REG32(address)

#ifndef BIT
#define BIT(x)	(1 << x)
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#define IIS_BASE (0xb8009000)
#endif

#define IISCR		(IIS_BASE + 0x00)	//IIS Interface Control Register
#define TX_PAGE_PTR	(IIS_BASE + 0x04)	//TX Page Pointer Register
#define RX_PAGE_PTR	(IIS_BASE + 0x08)	//RX Page Pointer Register
#define IIS_SETTING	(IIS_BASE + 0x0C)	//IIS Page size and Sampling rate setting  Register
#define IIS_TX_IMR	(IIS_BASE + 0x10)	//IIS TX Interrupt Mask Register
#define IIS_TX_ISR	(IIS_BASE + 0x14)	//IIS TX Interrupt Status Register
#define IIS_RX_IMR	(IIS_BASE + 0x18)	//IIS RX Interrupt Mask Register
#define IIS_RX_ISR	(IIS_BASE + 0x1C)	//IIS RX Interrupt Status Register
#define IIS_TX_P0OWN	(IIS_BASE + 0x20)	//IIS TX Page 0 Own bit
#define IIS_TX_P1OWN	(IIS_BASE + 0x24)	//IIS TX Page 1 Own bit
#define IIS_TX_P2OWN	(IIS_BASE + 0x28)	//IIS TX Page 2 Own bit
#define IIS_TX_P3OWN	(IIS_BASE + 0x2C)	//IIS TX Page 3 Own bit
#define IIS_RX_P0OWN	(IIS_BASE + 0x30)	//IIS RX Page 0 Own bit
#define IIS_RX_P1OWN	(IIS_BASE + 0x34)	//IIS RX Page 1 Own bit
#define IIS_RX_P2OWN	(IIS_BASE + 0x38)	//IIS RX Page 2 Own bit
#define IIS_RX_P3OWN	(IIS_BASE + 0x3C)	//IIS RX Page 3 Own bit

//IISCR
#define SW_RSTN		BIT(31)
#define DACLRSWAP	BIT(10)
#define IIS_LOOP_BACK	BIT(7)
#define IIS_WL_16BIT	0
#define IIS_EDGE_N	0
#define IIS_EDGE_P	BIT(5)
#define IIS_MODE_MONO	BIT(4)
#define IIS_TXRXACT	BIT(2)
#define IIS_ENABLE	BIT(0)

//IIS_TX_ISR, IIS_TX_IMR
#define IIS_TX_P0OK	BIT(0)
#define IIS_TX_P1OK	BIT(1)
#define IIS_TX_P2OK	BIT(2)
#define IIS_TX_P3OK	BIT(3)
#define IIS_TX_PUA	BIT(4)	/* tx page unavailable */
#define IIS_TX_FEM	BIT(5)	/* tx fifo empty */

//IIS_RX_ISR, IIS_RX_IMR
#define IIS_RX_P0OK	BIT(0)
#define IIS_RX_P1OK	BIT(1)
#define IIS_RX_P2OK	BIT(2)
#define IIS_RX_P3OK	BIT(3)
#define IIS_RX_PUA	BIT(4)	/* rx page unavailable */
#define IIS_RX_FFU	BIT(5)	/* rx fifo full */



//---------------------------------------------------------------------------------------------
//	Debug
//---------------------------------------------------------------------------------------------

//#define IIS_DEBUG

#undef IISDBUG

#ifdef IIS_DEBUG
#define IISDBUG(fmt, args...) printk("-%s:" fmt, __FUNCTION__, ## args)
#else
#define IISDBUG(fmt, args...)
#endif





#endif
