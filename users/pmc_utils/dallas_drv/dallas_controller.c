/*
#*******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#-------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#-------------------------------------------------------------------------------
# $RCSfile: dallas_controller.c,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.4 $
#-------------------------------------------------------------------------------
# Kernel module for Dallas Controller.
#-------------------------------------------------------------------------------
*/


#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/string.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/etherdevice.h>
//#include <asm/pmc-sierra/msp/msp_gpio_macros.h>

#define rtlRegRead(addr)        \
        (*(volatile u32 *)addr)

#define rtlRegWrite(addr, val)  \
        ((*(volatile u32 *)addr) = (val))

static inline u32 rtlRegMask(u32 addr, u32 mask, u32 value)
{
	u32 reg;

	reg = rtlRegRead(addr);
	reg &= ~mask;
	reg |= value & mask;
	rtlRegWrite(addr, reg);
	reg = rtlRegRead(addr); /* flush write to the hardware */

	return reg;
}

#define SHAREPIN_REGISTER_2 0xB8000044
#define GPABCDCNR	0xB8003500
#define GPABCDDIR	0xB8003508
#define GPABCDDATA	0xB800350C

#define MSP_GPIO_OUTPUT		1
#define MSP_GPIO_LO		0
#define MSP_GPIO_HI		1


int msp_gpio_pin_get_mode(int pin) { return 1&(rtlRegRead(GPABCDDIR)>>pin); }
int msp_gpio_pin_mode(int mode, int pin)
{
	if ( pin==19 ) { //pin C3
		rtlRegMask( SHAREPIN_REGISTER_2, (1<<22) | (1<<23), (1<<22) | (1<<23));
	}
	rtlRegMask( GPABCDCNR, 1<<pin, 1<<pin);
	rtlRegMask( GPABCDDIR, 1<<pin, mode<<pin);
	
	return 0;
}
int msp_gpio_pin_get(int pin) { return 1&(rtlRegRead(GPABCDDATA)>>pin); }
int msp_gpio_pin_lo(int pin)
{
	rtlRegMask( GPABCDDATA, 1<<pin, 0<<pin);
	return 0;
}
int msp_gpio_pin_hi(int pin)
{
	rtlRegMask( GPABCDDATA, 1<<pin, 1<<pin);
	return 0;
}

#define ENABLE_DALLAS_GPIO_IRQ 0

#include "pmc_typs.h"
#include "ksocket.h"
#include "pbrc_protocol.h"
#include "dallas_util.h"

MODULE_LICENSE("GPL");

static char *dallas_uni = "eth2";
static char *dallas_mac="00:0C:D5:62:90:04"; 
static int dallas_major_dev_no = -1;
static int dallas_device_open = 0;  /* Is device open?  Used to prevent multiple access to the device */

module_param(dallas_uni, charp, S_IRUGO);
module_param(dallas_mac, charp, S_IRUGO);

/*******************************************************************************
**  FUNCTION: pbrc_drv_get
**  ____________________________________________________________________________
**  
**  Description:
**  This API is used for getting value of specific address on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**      The value of specified address will be set to buff field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
*******************************************************************************/
static int pbrc_drv_get(struct pbrc_cmd * pbrc)
{
	int rev = 0;
	
	rev = pbrc_get(pbrc->address, pbrc->buff, pbrc->length, PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = rev;
	return 0;
}

/*******************************************************************************
**  FUNCTION: pbrc_drv_set
**  ____________________________________________________________________________
**  
**  Description:
**  This API is used for setting value to specific address on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
*******************************************************************************/
static int pbrc_drv_set(struct pbrc_cmd * pbrc)
{
	int rev = 0;
	
	rev = pbrc_set(pbrc->address, pbrc->buff, pbrc->length, PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = rev;
	return 0;
}

/*******************************************************************************
**  FUNCTION: pbrc_drv_set_field
**  ____________________________________________________________________________
**  
**  Description:
**  This API is used for setting value to specific address field on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
*******************************************************************************/
static int pbrc_drv_set_field(struct pbrc_cmd * pbrc)
{
	int rev = 0;
	
	rev = pbrc_set_field(pbrc->address, pbrc->buff[0], pbrc->mask, PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = rev;
	return 0;
}

/*******************************************************************************
**  FUNCTION: pbrc_drv_branch
**  ____________________________________________________________________________
**  
**  Description:
**  This API is used for jumping to specific address on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
*******************************************************************************/
int pbrc_drv_branch(struct pbrc_cmd * pbrc)
{
	int rev = 0;
	
	rev = pbrc_branch(pbrc->address, PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = rev;
	return 0;
}

/*******************************************************************************
**  FUNCTION: pbrc_drv_reset
**  ____________________________________________________________________________
**  
**  Description:
**  This API is used for resetting Dallas.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
*******************************************************************************/
int pbrc_drv_reset(void)
{
	int ret = -1;

	if ((ret = msp_gpio_pin_get_mode(DALLAS_GPIO_RESET)) != MSP_GPIO_OUTPUT) {
		ret = msp_gpio_pin_mode(MSP_GPIO_OUTPUT, DALLAS_GPIO_RESET);
		if (ret != 0) {
			printk("Failed to set GPIO[%d] mode to OUTPUT.\n", DALLAS_GPIO_RESET);
			return -1;
		}
	}

	if ((ret = msp_gpio_pin_get_mode(DALLAS_GPIO_RESET)) != MSP_GPIO_OUTPUT) {
		printk("Failed to set GPIO[%d] mode to OUTPUT.\n", DALLAS_GPIO_RESET);
		return -1;
	}

	/* Reset Dallas by GPIO. */
	msp_gpio_pin_lo(DALLAS_GPIO_RESET);
	msleep(150);
	msp_gpio_pin_hi(DALLAS_GPIO_RESET);
	
	if (msp_gpio_pin_get(DALLAS_GPIO_RESET) != MSP_GPIO_HI) {
		printk("Failed to reset Dallas.\n");
		return -1;
	}
	
	return 0;
}
EXPORT_SYMBOL(pbrc_drv_reset);

int pbrc_drv_dallas_irq(void)
{
	int ret = -1;
#if ENABLE_DALLAS_GPIO_IRQ
	if ((ret = msp_gpio_pin_get_mode(DALLAS_GPIO_IRQ)) != MSP_GPIO_OUTPUT) {
		ret = msp_gpio_pin_mode(MSP_GPIO_OUTPUT, DALLAS_GPIO_IRQ);
		if (ret != 0) {
			printk("Failed to set GPIO[%d] mode to OUTPUT.\n", DALLAS_GPIO_IRQ);
			return -1;
		}
	}

	if ((ret = msp_gpio_pin_get_mode(DALLAS_GPIO_IRQ)) != MSP_GPIO_OUTPUT) {
		printk("Failed to set GPIO[%d] mode to OUTPUT.\n", DALLAS_GPIO_IRQ);
		return -1;
	}
	
	/* assert an interrupt to Dallas */
	msp_gpio_pin_hi(DALLAS_GPIO_IRQ);
	msp_gpio_pin_lo(DALLAS_GPIO_IRQ);
	if (msp_gpio_pin_get(DALLAS_GPIO_IRQ) != MSP_GPIO_LO) {
		printk("Failed to assert interrupt to Dallas.\n");
		return -1;
	}
#endif /* ENABLE_DALLAS_GPIO_IRQ */
	return 0;
}
EXPORT_SYMBOL(pbrc_drv_dallas_irq);

int pbrc_drv_read_uni_mac(struct pbrc_cmd * pbrc)
{
	unsigned long rev = 0;
	
	rev = pbrc_read_uni_mac(pbrc->address, pbrc->buff, PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = (unsigned short)rev;
	return 0;
}

int pbrc_drv_write_uni_mac(struct pbrc_cmd * pbrc)
{
	unsigned long rev = 0;
	
	rev = pbrc_write_uni_mac(pbrc->address, pbrc->buff[0], PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = (unsigned short)rev;
	return 0;
}

int pbrc_drv_read_lag(struct pbrc_cmd * pbrc)
{
	unsigned long rev = 0;
	
	rev = pbrc_read_lag(pbrc->address, pbrc->buff, PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = (unsigned short)rev;
	return 0;
}

int pbrc_drv_write_lag(struct pbrc_cmd * pbrc)
{
	unsigned long rev = 0;

	rev = pbrc_write_lag(pbrc->address, pbrc->buff[0], PBRC_PACKET_TIMEOUT, 0);
	pbrc->error_code = (unsigned short)rev;
	return 0;
}

static int pbrc_drv_dallas_rpc(dallas_rpc * rpc)
{
	
	pbrc_dallas_rpc(rpc->ioctrl_number, &rpc->legal_size, &rpc->status, rpc->data, 200, 0);
	
	return 0;
}

static int dallas_controller_open(struct inode *inode, struct file *file)
{
	if (dallas_device_open) 
		return -EBUSY;
	dallas_device_open++;
	
	return 0;
}

static int dallas_controller_close(struct inode *inode, struct file *file)
{
	dallas_device_open--;
	return 0;
}

/*******************************************************************************
**  FUNCTION: dallas_controller_ioctl
**  ____________________________________________________________________________
**  
**  Description:
**  IOCTL process function of Dallas Controller module.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
*******************************************************************************/
static int dallas_controller_ioctl (struct inode *inode, struct file *fp, unsigned int cmd, unsigned long arg)
{
	struct pbrc_cmd pbrc;
	int retval = 0;
	dallas_rpc rpc;
	
	switch (cmd) {
	case PBRC_GET:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_get(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_SET:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_set(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_SET_FIELD:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_set_field(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_BRANCH:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_branch(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_READ_UNI_MAC:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_read_uni_mac(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_WRITE_UNI_MAC:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_write_uni_mac(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_READ_LAG:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_read_lag(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_WRITE_LAG:
		if (copy_from_user(&pbrc, (struct pbrc_cmd *) arg, sizeof(pbrc))) {
			retval = -1;
		} else {
			pbrc_drv_write_lag(&pbrc);
			if (copy_to_user((struct pbrc_cmd *) arg, &pbrc, sizeof(pbrc))) {
			    retval = -1;
			}
		}
		break;
	case PBRC_RESET:
		retval = pbrc_drv_reset();
		break;
	case DALLAS_IRQ:
		retval = pbrc_drv_dallas_irq();
		break;
	case DALLAS_RPC:
		if (copy_from_user(&rpc, (dallas_rpc *) arg, sizeof(rpc))) {
			retval = -1;
		} else {
			pbrc_drv_dallas_rpc(&rpc);
			if (copy_to_user((dallas_rpc *) arg, &rpc, sizeof(rpc))) {
			    retval = -1;
			}
		}
		break;
	default:
		printk("In dallas_controller_ioctl, unknown cmd.\n");
		break;
	}
	return retval;
}

struct file_operations dallas_controoller_fops = 
{
	.owner =    THIS_MODULE,
	.ioctl =    dallas_controller_ioctl,
	.open =     dallas_controller_open,
	.release =    dallas_controller_close,
};


static int dallas_controller_init_device(void)
{
	/* register the driver and fops */
	dallas_major_dev_no = register_chrdev(0, "pbrc", &dallas_controoller_fops);
	if(dallas_major_dev_no < 0) {
		printk("Failed to register dallas device!\n");
		return (-1);
	}
	return 0;
}

static int dallas_controller_shutdown_device(void)
{
	unregister_chrdev(dallas_major_dev_no, "pbrc");
	return(0);
}

static int dallas_controller_init(void)
{
	struct pbrc_initparam_t s_initparam;
	struct net_device *dev;
	
	printk("Dallas controller initializing, using device %s.\n", dallas_uni);
	
	if(0 != dallas_controller_init_device()) {
		printk("Failed to init device.\n");
		return(-1);
	}
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
{
	extern struct net init_net; //linux global variable
	dev = (struct net_device *) dev_get_by_name(&init_net, dallas_uni);
}
#else
	dev = dev_get_by_name(dallas_uni);
#endif

	memset(s_initparam.dev_string,0,10);
	memcpy(s_initparam.dev_string, dallas_uni, strlen(dallas_uni));
	memcpy(s_initparam.local_mac, dev->dev_addr, sizeof(dev->dev_addr));  /*MAC address of local network device*/
	if (set_hw_addr(s_initparam.dallas_mac, dallas_mac)) {	/*MAC address of Dallas device*/
		return(-1);
	}
	/* pbrc protocol kernel thread */
	if(pbrc_init(&s_initparam)) {
		printk("Failed to init PBRC protocol!\n");
		dallas_controller_shutdown_device();
		return(-1);
	}
	
	return 0;
}

static void dallas_controller_exit(void)
{
	printk("Dallas Controller exiting.Bye Bye.\n");
	pbrc_uninit();
	dallas_controller_shutdown_device();
	return;
}

module_init(dallas_controller_init);
module_exit(dallas_controller_exit);

