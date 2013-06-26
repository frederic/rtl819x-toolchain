/*
 *	RTL8186 PCM Controller Driver
 *
 *	version 0.1 - 2005.02.16 by Ziv Huang
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>	// error codes
#include <linux/types.h>	// size_t
#include <linux/delay.h>  	// udelay
#include <linux/interrupt.h>	// probe_irq_on, probe_irq_off
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/rtl8186.h>

#ifdef TIMER_MODE
#include <linux/spinlock.h>
#include <linux/timer.h>
#endif

#include "pcmctrl.h"

int pcmctrl_major = PCM_MAJOR;
int pcmctrl_devs = PCM_DEVS;	//nunmber of device

struct pcm_priv* pcmctrl_devices;


//-----------------------------------------------------------------------------
//	Open
//-----------------------------------------------------------------------------

int pcmctrl_open(struct inode* pinode, struct file* flip)
{
	MOD_INC_USE_COUNT;

	unsigned int channel = MINOR(pinode->i_rdev);
	
#ifdef BLOCK_MODE
	
	if(flip->f_flags & O_NONBLOCK)
		return -EBUSY;
#endif

	PDBUG("PCMISR = 0x%X\n", PCMISR);
	PDBUG("MINOR number = %d\n", channel);
 
	//check device number
	if(channel >= pcmctrl_devs)
		return -ENODEV;

	if(pcmctrl_devices)
	{
		char strbuf[NAME_SIZE];
		sprintf(strbuf, "pcm%d", channel);
		memcpy(pcmctrl_devices[channel].name, strbuf, NAME_SIZE);

		printk("------------- name = %s ------------\n", pcmctrl_devices[channel].name);

		struct pcm_priv* pcm_dev = &pcmctrl_devices[channel];
		PDBUG("pcmctrl_devices at 0x%X, pcm_dev at 0x%X\n", (unsigned int)pcmctrl_devices, (unsigned int)pcm_dev);
		PDBUG("Device name %s\n", pcm_dev->name);

		int result;

		if(!(result = pcmctrl_init_dev_sw(pcm_dev, channel)))
		{
			
			flip->private_data = pcm_dev;

#ifndef TIMER_MODE
			int result = request_irq(PCM_IRQ, pcm_interrupt, SA_SHIRQ, pcm_dev->name, pcm_dev);

			if(result)
			{
				PERR(KERN_ERR "Can't request IRQ for minior number %d.\n", channel);
				return result;
			}
		
			PDBUG("Channel %d got IRQ = %d\n", channel, PCM_IRQ);
#endif

			pcmctrl_init_dev_hw(pcm_dev, channel);

			return 0;
		}

		return result;
	}

	PDBUG("No memory allocated for pcmctrl_devices\n");

	return -ENODEV;
}

//---------------------------------------------------------------------------------------------
//	Release
//---------------------------------------------------------------------------------------------

int pcmctrl_release(struct inode* pinode, struct file* flip)
{
	int channel = MINOR(pinode->i_rdev);

	PDBUG("release pcm\n");
	
	//check device number
	if(channel >= pcmctrl_devs)
		return -ENODEV;

	struct pcm_priv* pcm_dev = &pcmctrl_devices[channel];	

	//disable tx, rx
	pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & ~pcm_dev->CHCNRValue);
	//pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & ~(0xFF << ((3-channel)*8)));

#ifdef TIMER_MODE
	del_timer_sync(&pcm_dev->timer);
#else
	//interrupt mask
	pcm_outl(PCMIMR, (pcm_inl(PCMIMR) & ~( 0xF << ((3 - channel)*4))) );

	pcm_outl(TX_BSA(channel), 0);
	pcm_outl(RX_BSA(channel), 0);

	printk("PCMCHCNR = 0x%X, PCMIMR = 0x%X, TXBSA = 0x%X, RXBSA = 0x%X\n", pcm_inl(PCMCHCNR), pcm_inl(PCMIMR), pcm_inl(TX_BSA(channel)), pcm_inl(RX_BSA(channel)));
	


	free_irq(PCM_IRQ, pcm_dev);
#endif

	kfree(pcm_dev->tx_allocated);
	kfree(pcm_dev->rx_allocated);
	kfree(pcm_dev->rx_circular);

	MOD_DEC_USE_COUNT;
	return 0;
}

//---------------------------------------------------------------------------------------------
//	Read
//---------------------------------------------------------------------------------------------

//count should be less or equal PCMPAGE_SIZE
int pcmctrl_read_cpage(struct pcm_priv* pcm_dev, char* buf, size_t count)
{
	//printk("%s : rx_R = %d, rx_W = %d\n", __FUNCTION__ ,pcm_dev->rx_R, pcm_dev->rx_W);
	//if(copy_to_user(buf, (unsigned char*)(pcm_dev->rx_cb[pcm_dev->rx_R] + pcm_dev->pagepointer), count))
	if(copy_to_user(buf, (unsigned char*)(pcm_dev->rx_circular  + (pcm_dev->rx_R * PCMPAGE_SIZE) + pcm_dev->pagepointer), count))
	{
		return -EFAULT;
	}

	unsigned int pageoffset = pcm_dev->pagepointer + count;
	pcm_dev->pagepointer = pageoffset%PCMPAGE_SIZE;

	if(pageoffset >= PCMPAGE_SIZE)
	{
		pcm_dev->rx_R = (pcm_dev->rx_R + 1)%CBUF_PAGE_NUM;
//		printk("%s : Move rx_R to %d, and rx_W = %d\n",__FUNCTION__, pcm_dev->rx_R, pcm_dev->rx_W);
	}

	//printk("------------------------------------------------------\n");
	return 0;

#if 0

	PDBUG("read size = %d, rx_R = %d\n", count, pcm_dev->rx_R);
	if(fullpage)
	{
		if(copy_to_user(buf, pcm_dev->rx_cb[pcm_dev->rx_R], PCMPAGE_SIZE))
		{
			return -EFAULT;
		}
		pcm_dev->rx_R = (pcm_dev->rx_R + 1)%CBUF_PAGE_NUM;
	}
	else
	{
		if(copy_to_user(buf, (unsigned char *)(pcm_dev->rx_cb[pcm_dev->rx_R] + pcm_dev->pagepointer), count))
		{
			return -EFAULT;
		}

		unsigned int pageoffset = pcm_dev->pagepointer + count;
		pcm_dev->pagepointer = (pageoffset)%PCMPAGE_SIZE;

		if(pageoffset >= PCMPAGE_SIZE)
		{
			//roll to next page.
			pcm_dev->rx_R = (pcm_dev->rx_R + 1)%CBUF_PAGE_NUM;
		}
	}

	PDBUG("Move rx_R to %d pagepointer = %d\n", pcm_dev->rx_R, pcm_dev->pagepointer);
	return 0;
#endif
}

#if 0

ssize_t pcmctrl_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;
	struct pcm_priv* pcm_dev = filp->private_data;
	unsigned int channel = pcm_dev->channel;

	while(!((pcm_inl(RX_BSA(channel)) >> pcm_dev->rxpindex) & 0x1)){}

	unsigned char *pageaddr = pcm_dev->rx_buffer + (pcm_dev->rxpindex * PCMPAGE_SIZE);

	//just for testing. should check the buf size!!
	if(copy_to_user(buf, pageaddr, PCMPAGE_SIZE))
		return -EFAULT;

	retval = PCMPAGE_SIZE;

//	printk("Move rxpindex to %d\n", pcm_dev->rxpindex);
	pcm_dev->rxpindex = (pcm_dev->rxpindex + 1)%2;

	pcm_outl(RX_BSA(channel), pcm_inl(RX_BSA(channel)) | BIT(pcm_dev->rxpindex));

	return retval;
}


#endif

ssize_t pcmctrl_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;
	ssize_t readsize = 0;
	
	struct pcm_priv* pcm_dev = filp->private_data;

#ifdef TIMER_MODE
	spin_lock(&pcm_dev->lock);
#else
	unsigned long flags;
	save_flags(flags); cli();
#endif

#ifdef BLOCK_MODE
	while(count > 0)
#else
	while(count > 0 && pcm_dev->rx_R != pcm_dev->rx_W)
#endif
	{

		if(pcm_dev->rx_overflow == TRUE)
		{
			if(pcm_dev->rx_W == 0)
			{
				pcm_dev->rx_R = CBUF_PAGE_NUM - 1;
			}
			else
			{
				pcm_dev->rx_R = pcm_dev->rx_W - 1;
			}
			pcm_dev->rx_overflow = FALSE;
			pcm_dev->pagepointer = 0;

			PDBUG("rx overflow, Set rx_R to rx_W - 1 and pagepointer = 0\n");
		}


		if(pcm_dev->pagepointer)
		{
			unsigned int remain = PCMPAGE_SIZE - pcm_dev->pagepointer;
			readsize = (count < remain) ? count : remain;			

			if(pcmctrl_read_cpage(pcm_dev, (buf + retval), readsize))
			{
				retval = -EFAULT;
				goto rfail;
			}
		}
		else
		{
			readsize = (count < PCMPAGE_SIZE) ? count : PCMPAGE_SIZE;

			PDBUG("Read size = %d, rx_R = %2d, rx_W = %2d\n", readsize, pcm_dev->rx_R, pcm_dev->rx_W);

			if(pcmctrl_read_cpage(pcm_dev, (buf + retval), readsize))
			{
				retval = -EFAULT;
				goto rfail;
			}
		}

		retval += readsize;
		count -= readsize;
	}

rfail:
#ifdef TIMER_MODE
	spin_unlock(&pcm_dev->lock);
#else
	restore_flags(flags);
#endif

	return retval;
}


//---------------------------------------------------------------------------------------------
//	Write
//---------------------------------------------------------------------------------------------

//write data to PCM controller.
int pcmctrl_write_page(struct pcm_priv* pcm_dev, const char* buf, size_t count, BOOL fullpage)
{
	unsigned char* pageaddr = (unsigned char *)((unsigned int)pcm_dev->tx_buffer + (pcm_dev->txpindex * PCMPAGE_SIZE));
	
	if(copy_from_user(pageaddr, buf, count))
	{
		printk("Can't copy from user\n");
		return -EFAULT;
	}

	//PDBUG("tx_buffer = 0x%X, Write to 0x%X, %d page, size =  %d\n", (unsigned int)pcm_dev->tx_buffer, (unsigned int)pageaddr, pageindex, count);
	//pending zero
	if(!fullpage)
	{
		memset((unsigned char *)(pageaddr + count), 0, PCMPAGE_SIZE - count);
	}

	unsigned int channel = pcm_dev->channel;
	//PDBUG("PCMCHCNR befor poll = 0x%X\n", (unsigned int)pcm_inl(PCMCHCNR));
	//poll tx
	//PDBUG("Before poll tx, Channel %d PageIndex = %d, OWN = 0x%X\n", channel, pageindex, pcm_inl(TX_BSA(channel)) & OWN_MASK);
	pcm_outl(TX_BSA(channel), (pcm_inl(TX_BSA(channel)) | BIT(pcm_dev->txpindex)));

	pcm_dev->txpindex = (pcm_dev->txpindex + 1)%2;
	return 0;
}

//write data to circular buffer.
int pcmctrl_write_cpage(struct pcm_priv* pcm_dev, const char* buf, size_t count, BOOL fullpage)
{
#if 0
	//if(copy_from_user(pcm_dev->tx_cb[pcm_dev->tx_W], buf, count))
	if(copy_from_user((pcm_dev->tx_circular + pcm_dev->tx_W * PCMPAGE_SIZE), buf, count))
		return -EFAULT;

	PDBUG("Write data to circular W-%d, count = %d\n", pcm_dev->tx_W, count);
	//pending zero
	if(!fullpage)
	{
		//memset(pcm_dev->tx_cb[pcm_dev->tx_W] + count, 0, PCMPAGE_SIZE - count);
	}
#endif
	return 0;
}

#if 0
//---------------------------------------------------------------------------------------------
// For testing. OWN two pages at the same time.
//---------------------------------------------------------------------------------------------
int pcmctrl_write_pages(struct pcm_priv* pcm_dev, const char* buf, size_t count, BOOL fullpage)
{
	if(copy_from_user(pcm_dev->tx_buffer, buf, count))
	{
		return -EFAULT;
	}
	
#ifdef PCM_TEST
	pcm_outl(PCMBSIZE, (pcm_inl(PCMBSIZE) & ~(0xFF << ((3 - pcm_dev->channel)*8))) | (PCM_SIZE_N << ((3 - pcm_dev->channel)*8)));
	memset(pcm_dev->tx_buffer, 0xAA, count);
#endif

	if(!fullpage)
	{
		memset((void *)(pcm_dev->tx_buffer + count), 0, BUFFER_SIZE - count);
	}
	
	PDBUG("Before poll tx. Channel %d OWN = 0x%X\n", pcm_dev->channel, (pcm_inl(TX_BSA(pcm_dev->channel)) & OWN_MASK) );

	pcm_outl(TX_BSA(pcm_dev->channel), pcm_inl(TX_BSA(pcm_dev->channel)) | 0x3);
	pcm_outl(PCMCHCNR, CH0TE);

	PDBUG("After poll tx. Channel %d OWN = 0x%X\n", pcm_dev->channel, (pcm_inl(TX_BSA(pcm_dev->channel)) & OWN_MASK) );

	return 0;
}

#endif
//-------------------------------------------------------------------------

unsigned int mycount=0;

ssize_t pcmctrl_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos)
{
	//int i;
	ssize_t retval = 0;
	struct pcm_priv* pcm_dev = filp->private_data;
	unsigned int channel = pcm_dev->channel;
	//unsigned int fullpagenum = count/PCMPAGE_SIZE;
	//unsigned int rest = count%PCMPAGE_SIZE;
	//unsigned int allpagenum = fullpagenum + ( rest ? 1 : 0);
	//char* bufadd = buf;
	//unsigned long flags;

	//PDBUG("count = %d, fullpagenum = %d, rest = %d, allpagenum = %d\n", count, fullpagenum, rest, allpagenum);

	//if(down_interruptible (&pcm_dev->sem))
	//	return -ERESTARTSYS;

	//unsigned long flags;
	//save_flags(flags); cli();
	//if(down(&pcm_dev->sem))
	//{
	//	PDBUG("Can't down sem.\n");
	//	return -ERESTARTSYS;	
	//}

	//unsigned int own_bits = pcm_inl(TX_BSA(channel)) & OWN_MASK;

//	unsigned int own_bits = pcm_inl(TX_BSA(channel)) & OWN_MASK;


	//----------------------------------------------------------------
/*
	//unsigned char tx_buffer[BUFFER_SIZE];
	//unsigned int mycount = 0;
	unsigned int bsa;

	//while(mycount < 100)
	{	
		bsa = pcm_inl(TX_BSA(channel)) & OWN_MASK;
		PDBUG("mycount = %d, BSA = 0x%X\n", mycount, bsa);
		if( bsa == 0)
		{
			memset(pcm_dev->tx_buffer, mycount, BUFFER_SIZE);
			PDBUG("Set buffer content to 0x%X and poll\n", mycount);
			pcm_outl(TX_BSA(channel), TX_BSA(channel) | 0X3);
			mycount++;
		}
	}

	return 1;
*/

	//---------------------------------------------------------------
/*
	while(count > 0 && own_bits == 0)
	{
		if(count < BUFFER_SIZE)
		{
			if(pcmctrl_write_pages(pcm_dev, (buf + retval), count, FALSE))
			{
				retval = -EFAULT;
				goto wfailed;
			}
			
			count = 0;
			retval += count;
		}
		else
		{
			if(pcmctrl_write_pages(pcm_dev, (buf + retval), BUFFER_SIZE, TRUE))
			{
				retval = -EFAULT;
				goto wfailed;
			}
			
			count -= BUFFER_SIZE;
			retval += BUFFER_SIZE;
		}
		own_bits = pcm_inl(TX_BSA(channel)) & OWN_MASK;
	}

	return retval;

	//---------------------------------------------------------------
*/

	while(count > 0 &&  !(((pcm_inl(TX_BSA(channel)) & OWN_MASK) >> pcm_dev->txpindex ) & 0x1))
	{
		//printk("count = %d, retval = %d, txpindex = %d\n", count, retval, pcm_dev->txpindex);
		if(count < PCMPAGE_SIZE)
		{
			if(pcmctrl_write_page(pcm_dev, (buf + retval), count, FALSE))
			{
				retval = -EFAULT;
				break;
			}
			retval += count;
			count = 0;
		}
		else
		{
			if(pcmctrl_write_page(pcm_dev, (buf + retval), PCMPAGE_SIZE, TRUE))
			{
				retval = -EFAULT;
				break;
			}
			count -= PCMPAGE_SIZE;
			retval += PCMPAGE_SIZE;
		//	printk("PAGE_SIZE count = %d, retval = %d\n", count, retval);

		}
		//pcm_dev->txpindex = (pcm_dev->txpindex + 1)%2;
	}

	//printk("retval = %d\n", retval);
	return retval;

#if 0


	unsigned long flags;
	save_flags(flags); cli();	
	
	unsigned int unNextWrite = (pcm_dev->tx_W + 1)%CBUF_PAGE_NUM;

	//page full, write data to circular buffer.
	while(count > 0 && pcm_dev->tx_R != unNextWrite)
	{
		if(count < PCMPAGE_SIZE)
		{
			if(pcmctrl_write_cpage(pcm_dev, (buf + retval), count, FALSE))
			{
				PDBUG("write circular buffer failed.\n");
				retval = -EFAULT;
				break;
			}

			count = 0;
			retval += count;		
		}
		else
		{
			if(pcmctrl_write_cpage(pcm_dev, (buf + retval), PCMPAGE_SIZE, TRUE))
			{
				PDBUG("write circular buffer failed.\n");
				retval = -EFAULT;
				break;
			}
			count -= PCMPAGE_SIZE;
			retval += PCMPAGE_SIZE;
		}
		
		pcm_dev->tx_W = (pcm_dev->tx_W + 1)%CBUF_PAGE_NUM;
		unNextWrite = (pcm_dev->tx_W + 1)%CBUF_PAGE_NUM;
	}
//bfailed:
	restore_flags(flags);

wfailed:
	return retval;

	
	if( !(own_bits == 0x3) && count > 0)	//at least one page available.
	{
		PDBUG("We have at least one free page.\n");
		BOOL fullflag = FALSE;

		//write data to PCM controller directly.
		//do
		while(pcm_dev->tx_R == pcm_dev->tx_W && count != 0 && fullflag == FALSE)
		{
			PDBUG("Write PCM to page(s) directly.\n");

			for(i = 0 ; i < fullpagenum ; i++)
			{
				//check page's OWN is 0
				if( !(((pcm_inl(TX_BSA(pcm_dev->channel)) & OWN_MASK) >> pcm_dev->txpindex ) & 0x1))
				//unsigned int own = pcm_inl(tx_bsa[pcm_dev->channel]) & OWN_MASK;
				//unsigned int re = own >> pcm_dev->txpindex;
				//if(!(re & 0x1))
				{
					if(pcmctrl_write_page(pcm_dev, (buf + retval), PCMPAGE_SIZE, pcm_dev->txpindex, TRUE))
					{
						retval = -EFAULT;
						goto fail;
					}

					count -= PCMPAGE_SIZE;
					retval += PCMPAGE_SIZE;
					pcm_dev->txpindex = (pcm_dev->txpindex + 1)%2;	//point to next page
					PDBUG("Point to next page - %d\n", pcm_dev->txpindex);
				}
				else 
				{
					fullflag = TRUE;
					break;
				}
			}

			//PDBUG("fullflag = %s, count = %d\n");

			//write the remaind data
			if( (fullflag == FALSE) && count &&  !(((pcm_inl(TX_BSA(pcm_dev->channel)) & OWN_MASK) >> pcm_dev->txpindex ) & 0x1))
			{
				if(pcmctrl_write_page(pcm_dev, (buf + retval), rest, pcm_dev->txpindex, FALSE))
				{
					retval = -EFAULT;
					goto fail;
				}
				count = 0;
				retval += count;

				pcm_dev->txpindex = (pcm_dev->txpindex + 1)%2;	//point to next page
				break;	//last data
			}

		}//while(count != 0 || fullflag);

		
		save_flags(flags); cli();
	
		unsigned int unNextWrite = (pcm_dev->tx_W + 1)%CBUF_PAGE_NUM;

		while(count > 0 && unNextWrite != pcm_dev->tx_R)
		{
			if(count >= PCMPAGE_SIZE)
			{
				if(pcmctrl_write_cpage(pcm_dev, (buf + retval), PCMPAGE_SIZE, TRUE))
				{
					retval = -EFAULT;
					goto out;
				}

				count -= PCMPAGE_SIZE;
				retval += PCMPAGE_SIZE;
			}
			else
			{
				if(pcmctrl_write_cpage(pcm_dev, (buf + retval), count, FALSE))
				{
					retval = -EFAULT;
					goto out;
				}

				count = 0;
				retval += count;
			}
			pcm_dev->tx_W = unNextWrite;
			unNextWrite = (pcm_dev->tx_W + 1)%CBUF_PAGE_NUM;
			
			if(unNextWrite == pcm_dev->tx_R && count > 0)
			{
				PERR("Circular buffer overflow.\n");
			}
		}
	}
out:
	//up(&pcm_dev->sem);	
	restore_flags(flags);
fail:
	return retval;
#endif

}




//---------------------------------------------------------------------------------------------
//	IOCTL
//---------------------------------------------------------------------------------------------

#define PCM_LINEAR	0xBE01
#define PCM_ALAW	0xBE02
#define PCM_ULAW	0xBE03


int pcmctrl_ioctl(struct inode *inode, struct file *filp,  unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	struct pcm_priv* pcm_dev = filp->private_data;

	unsigned long flags;
	save_flags(flags);cli();

	switch(cmd)
	{
		case PCM_LINEAR:	//Linear
			if(pcm_dev->mode != LINEAR)
			{
				pcmctrl_init_hardware(LINEAR);
			}
			break;
		case PCM_ALAW:		//A-law
			if(pcm_dev->mode != A_LAW)
			{
				pcmctrl_init_hardware(A_LAW);

			}
			break;
		case PCM_ULAW:		//u-law
			if(pcm_dev->mode != U_LAW)
			{
				pcmctrl_init_hardware(U_LAW);
			}
			break;
	}

	restore_flags(flags);
	return retval;
}



struct file_operations pcmctrl_fops = {
	read:pcmctrl_read,
	write:pcmctrl_write,
	ioctl:pcmctrl_ioctl,
	open:pcmctrl_open,
	release:pcmctrl_release,
};


//---------------------------------------------------------------------------------------------
//	Init
//---------------------------------------------------------------------------------------------



//int pcmctrl_init_dev_sw(unsigned int channel)
int pcmctrl_init_dev_sw(struct pcm_priv* pcm_dev, unsigned int channel)
{

	if(pcm_dev->sw_init == FALSE)
	{
		//struct pcm_priv* pcm_dev = &pcmctrl_devices[channel];
		//memset(pcm_dev, 0, sizeof(struct pcm_priv));
		//void* tx_circular = NULL;
		//void* rx_circular = NULL;

		PDBUG("PCM_SIZE_N =%d, PCMPAGE_SIZE =%d, BUFFER_SIZE = %d\n", PCM_SIZE_N, PCMPAGE_SIZE, BUFFER_SIZE);

		pcm_dev->tx_allocated = kmalloc(BUFFER_SIZE + 4, GFP_KERNEL);
	
		if(pcm_dev->tx_allocated)
		{
			PDBUG("Channel %d get tx buffer at 0x%X\n", channel, (unsigned int)pcm_dev->tx_allocated);
			pcm_dev->rx_allocated = kmalloc(BUFFER_SIZE + 4, GFP_KERNEL);
		
			if(pcm_dev->rx_allocated)
			{
				PDBUG("Channel %d get rx buffer at 0x%X\n", channel, (unsigned int)pcm_dev->rx_allocated);

				memset(pcm_dev->tx_allocated, 0, BUFFER_SIZE + 4);
				memset(pcm_dev->rx_allocated, 0, BUFFER_SIZE + 4);
			
			//word-alignment
				if((unsigned int)pcm_dev->tx_allocated & 0x3){
					pcm_dev->tx_buffer = (unsigned char *)(((unsigned int)pcm_dev->tx_allocated & 0xFFFFFFF0) + 4);
				}
				else{
					pcm_dev->tx_buffer = pcm_dev->tx_allocated;
				}

				if((unsigned int)pcm_dev->rx_allocated & 0x3){
					pcm_dev->rx_buffer = (unsigned char *)(((unsigned int)pcm_dev->rx_allocated & 0xFFFFFFF0) + 4);
				}
				else{
					pcm_dev->rx_buffer = pcm_dev->rx_allocated;
				}

#if 0
			PDBUG("Get alignment tx buffer at 0x%X\n", (unsigned int)pcm_dev->tx_buffer);
			PDBUG("Get alignment rx buffer at 0x%X\n", (unsigned int)pcm_dev->rx_buffer);
			PDBUG("TX  virt_to_bus address at 0x%X\n", (unsigned int)virt_to_bus(pcm_dev->tx_buffer));
			PDBUG("RX  virt_to_bus address at 0x%X\n", (unsigned int)virt_to_bus(pcm_dev->rx_buffer));
#endif

				//allocate rx circular buffer
				pcm_dev->rx_circular = kmalloc(CBUF_PAGE_NUM * PCMPAGE_SIZE, GFP_KERNEL);
				if(pcm_dev->rx_circular)
				{
					memset(pcm_dev->rx_circular, 0, CBUF_PAGE_NUM * PCMPAGE_SIZE);
					pcm_dev->channel = channel;
					pcm_dev->sw_init = TRUE;
#ifdef BLOCK_MODE
					init_waitqueue_head(&pcm_dev->wait_q);
#endif

#ifdef USE_TASKLET
					tasklet_init(&pcm_dev->rx_tasklet, pcm_rx_isr, (unsigned long)pcm_dev);
#endif
#ifdef TIMER_MODE
					spin_lock_init(&pcm_dev->lock);
					init_timer(&pcm_dev->timer);
					pcm_dev->timer.data = (unsigned long)pcm_dev;
					pcm_dev->timer.function = pcmctrl_rx_timer;
#endif
					return 0;
				}

				goto circular_fail;

#if 0
			//allocate tx, rx circular buffer
			tx_circular = kmalloc(CBUF_PAGE_NUM * PCMPAGE_SIZE, GFP_KERNEL);
			
			if(tx_circular)
			{
				rx_circular = kmalloc(CBUF_PAGE_NUM * PCMPAGE_SIZE, GFP_KERNEL);
				
				if(rx_circular)
				{
					memset(tx_circular, 0, CBUF_PAGE_NUM * PCMPAGE_SIZE);
					memset(rx_circular, 0, CBUF_PAGE_NUM * PCMPAGE_SIZE);

					unsigned int index;

					for(index = 0 ; index < CBUF_PAGE_NUM ; index++)
					{
						pcm_dev->tx_cb[index] = (unsigned char *)(tx_circular + (index * PCMPAGE_SIZE));
						pcm_dev->rx_cb[index] = (unsigned char *)(rx_circular + (index * PCMPAGE_SIZE));
					}

					pcm_dev->channel = channel;
					tasklet_init(&pcm_dev->rx_tasklet, pcm_rx_isr, (unsigned long)pcm_dev);
					//sema_init(&pcmctrl_devices[index].sem, 1);

					return 0;
				}

				goto circular_fail;
			}
			
			goto circular_fail;
#endif
			}
		
			goto pcmbf_fail;
		//{
		//	kfree(pcm_dev->tx_allocated);
		//	pcm_dev->tx_allocated = 0;
		//
		//}
		}
	}
	else
	{
		memset(pcm_dev->tx_allocated, 0, BUFFER_SIZE + 4);
		memset(pcm_dev->rx_allocated, 0, BUFFER_SIZE + 4);
		memset(pcm_dev->rx_circular, 0, CBUF_PAGE_NUM * PCMPAGE_SIZE);
		pcm_dev->rx_R = pcm_dev->rx_W = pcm_dev->rxpindex = pcm_dev->txpindex = pcm_dev->pagepointer = pcm_dev->CHCNRValue = 0;
		pcm_dev->rx_overflow = FALSE;
		return 0;
	}
circular_fail:
	pcm_dev->rx_circular = 0;
#if 0
	kfree(tx_circular);
	memset(pcm_dev->tx_cb, 0, sizeof(unsigned char *) * CBUF_PAGE_NUM);
	kfree(rx_circular);
	memset(pcm_dev->rx_cb, 0, sizeof(unsigned char *) * CBUF_PAGE_NUM);
#endif
pcmbf_fail:
	kfree(pcm_dev->tx_allocated);
	pcm_dev->tx_buffer = pcm_dev->tx_allocated = 0;
	kfree(pcm_dev->rx_allocated);
	pcm_dev->rx_buffer = pcm_dev->rx_allocated = 0;

	PDBUG("Can't allocate Memory for tx/rx buffer.\n");

	return -ENOMEM;
}

void pcmctrl_init_dev_hw(struct pcm_priv* pcm_dev, unsigned int channel)
{
	//struct pcm_priv* pcm_dev = &pcmctrl_devices[channel];
	
	//pcm_outl(TX_BSA(channel), (unsigned int)(Virtual2Physical(pcm_dev->tx_buffer) & 0xFFFFFF));
	//pcm_outl(RX_BSA(channel), (unsigned int)((Virtual2Physical(pcm_dev->rx_buffer) & 0xFFFFFF) | 0x3));	//set RX buffer and OWN bits
	pcm_outl(TX_BSA(channel), (unsigned int)Virtual2Physical(pcm_dev->tx_buffer));
	pcm_outl(RX_BSA(channel), (unsigned int)(Virtual2Physical(pcm_dev->rx_buffer) | 0x3));	//set RX buffer and OWN bits.
	

	PDBUG("TXBSA = 0x%X, RXBSA = 0x%X\n", pcm_inl(TX_BSA(channel)), pcm_inl(RX_BSA(channel)));
	printk("TXBSA = 0x%X, RXBSA = 0x%X\n", pcm_inl(TX_BSA(channel)), pcm_inl(RX_BSA(channel)));


	//set tx, rx buffer size
	pcm_outl(PCMBSIZE, (pcm_inl(PCMBSIZE) & ~(0xFF << ((3 - channel)*8))) | BSIZE(channel, PCM_SIZE_N));//(PCM_SIZE_N << ((3 - channel)*8)));
	PDBUG("PCMBSIZE = 0x%X\n", (unsigned int)pcm_inl(PCMBSIZE));

	pcm_dev->CHCNRValue = 0;

#ifdef CH0_LOOP
	pcm_dev->CHCNRValue = (CHxRE(channel) | CHxTE(channel) | C0ILBE);
	printk("Use Channel 0 Internal Loop-back.\n");
#else
	switch(pcm_dev->mode)
	{
		case LINEAR:
			pcm_dev->CHCNRValue = (CHxRE(channel) | CHxTE(channel));
			break;
		case A_LAW:
			pcm_dev->CHCNRValue = (CHxRE(channel) | CHxTE(channel) | CHxUA(channel) | CxCMPE(channel));
			break;
		case U_LAW:
			pcm_dev->CHCNRValue = (CHxRE(channel) | CHxTE(channel) | CxCMPE(channel));
			break;
		default:
			pcm_dev->CHCNRValue = (CHxRE(channel) | CHxTE(channel));
			break;
	}
#endif

/*
#ifdef USE_LINEAR
	pcm_dev->CHCNRValue = (CHxRE(channel) | CHxTE(channel));
#else
	//enable tx, rx
	pcm_dev->CHCNRValue = (CHxRE(channel) | CHxTE(channel) | CHxUA(channel) | CxCMPE(channel));
#endif
*/
	pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) | pcm_dev->CHCNRValue);
	
	PDBUG("PCMCHCNR = 0x%X\n", (unsigned int)pcm_inl(PCMCHCNR));

#ifdef TIMER_MODE
	pcm_dev->timer.expires = jiffies + TIME_INTERVAL;
	add_timer(&pcm_dev->timer);
//#else
#endif
	//interrupt mask
#ifdef IMR_OK_ONLY_ON
	pcm_outl(PCMIMR, pcm_inl(PCMIMR) | IMR_OK_ONLY(channel));
#else
	pcm_outl(PCMIMR, pcm_inl(PCMIMR) | IMR_ALL(channel));
#endif

	PDBUG("Channel %d interrupt enable. PCMIMR = 0x%X\n", channel, (unsigned int)pcm_inl(PCMIMR) );

//#endif	//TIMER_MODE

}

//void pcmctrl_init_hardware(void)
void pcmctrl_init_hardware(unsigned int mode)
{
	//reset pcm hardware.
	pcm_outl(PCMCR, PCM_ENABLE);
	pcm_outl(PCMCR, 0);

	codec_init(mode);

#if 0

	PDBUG("PCMCR = 0x%X\n", pcm_inl(PCMCR));
	PDBUG("PCMISR = 0x%X\n",pcm_inl(PCMISR));
	PDBUG("PCMCHCNR = 0x%X\n", pcm_inl(PCMCHCNR));
	PDBUG("PCMTSR = 0x%X\n", pcm_inl(PCMTSR));
	PDBUG("PCMBSIZE = 0x%X\n", pcm_inl(PCMBSIZE));
	PDBUG("PCMIMR = 0x%X\n", pcm_inl(PCMIMR));
#endif

	pcm_outl(PCMCR, PCM_ENABLE);	//enable PCM interface
	PDBUG("Enable PCM interface. PCMCR = 0x%X\n", pcm_inl(PCMCR));

	if(pcmctrl_devices)
	{
		int i;
		//struct pcm_priv* pcm_dev;

		for(i = 0 ; i < pcmctrl_devs ; i++)
		{
			pcmctrl_devices[i].mode = mode;
		}
	}

#ifdef LOOPBACK
	unsigned int result;

	struct pcm_priv* pcm_dev = &pcmctrl_devices[0];
	pcm_dev->mode = mode;
	
	if(!(result = pcmctrl_init_dev_sw(pcm_dev, 0)))
	{
			

		int result = request_irq(PCM_IRQ, pcm_interrupt, SA_SHIRQ, "pcmdev0", pcm_dev);

		if(result)
		{
			PERR(KERN_ERR "Can't request IRQ for minior number %d.\n", pcm_dev->channel);
			return;
		}
		
		PDBUG("Channel %d got IRQ = %d\n", pcm_dev->channel, PCM_IRQ);

		pcmctrl_init_dev_hw(pcm_dev, 0);
		PDBUG("Initialize PCM Device 0 OK.\n");
	}

#endif

}



int pcmctrl_init(void)
{
	int result = register_chrdev(pcmctrl_major, DEV_NAME, &pcmctrl_fops);

	if(result < 0){
		printk(KERN_ERR "Can't register PCM controller devices.");
		return result;	
	}

	//dynamic assign major number.
	if(pcmctrl_major == 0){
		pcmctrl_major = result;
		PDBUG("Get Major number %d\n", result);
	}

	pcmctrl_devices = kmalloc(pcmctrl_devs * sizeof(struct pcm_priv), GFP_KERNEL);

	if(pcmctrl_devices)
	{
		memset(pcmctrl_devices, 0, pcmctrl_devs * sizeof(struct pcm_priv));

#ifndef LINEAR_MODE
		pcmctrl_init_hardware(A_LAW);
#else
		pcmctrl_init_hardware(LINEAR);
#endif
		result = 0;
	}
	else
	{
		printk(KERN_ERR "PCM - Allocate Memory failed. Unregister Device.\n");
		result = -ENOMEM;
		unregister_chrdev(pcmctrl_major, DEV_NAME);
	}

	return result;

}


//---------------------------------------------------------------------------------------------
//	Clean up
//---------------------------------------------------------------------------------------------

void pcmctrl_cleanup(void)
{

	PDBUG("Now cleanup driver module.\n");
	//Disable PCM interface.
	pcm_outl(PCMCR, 0);
	unregister_chrdev(pcmctrl_major, DEV_NAME);

	kfree(pcmctrl_devices);
}

#ifdef TIMER_MODE

void pcmctrl_rx_timer(unsigned long task_priv)
{
	struct pcm_priv* pcm_dev = (struct pcm_priv *)task_priv;
	unsigned int channel = pcm_dev->channel;	
	unsigned int count = 0;
	unsigned char* pageaddr;
	unsigned int unNextWrite;

	//printk("TIME_INTERVAL = %d\n", TIME_INTERVAL);
	
	spin_lock(&pcm_dev->lock);

	unNextWrite = (pcm_dev->rx_W + 1)%CBUF_PAGE_NUM;

	printk("RX_BSA = 0x%X\n", pcm_inl(RX_BSA(channel)));

	while(count < 2 && !((pcm_inl(RX_BSA(channel)) >> pcm_dev->rxpindex) & 0x1))
	{
		//buffer overflow. move R pointer to next one. Discard old data.
		if(unNextWrite == pcm_dev->rx_R)	
		{
			pcm_dev->rx_R = (pcm_dev->rx_R + 1)%CBUF_PAGE_NUM;
			pcm_dev->rx_overflow = TRUE;
		}

		pageaddr = pcm_dev->rx_buffer + (pcm_dev->rxpindex * PCMPAGE_SIZE);

		printk("rxpindex = %d, rx_W = %d, rx_R = %d, count = %d\n", pcm_dev->rxpindex, pcm_dev->rx_W, pcm_dev->rx_R, count);		
		memcpy((pcm_dev->rx_circular + (pcm_dev->rx_W * PCMPAGE_SIZE)), pageaddr, PCMPAGE_SIZE);
		
		pcm_outl(RX_BSA(channel), pcm_inl(RX_BSA(channel)) | BIT(pcm_dev->rxpindex) );
		pcm_dev->rx_W = unNextWrite;
		//pcm_dev->rx_W = (pcm_dev->rx_W + 1)%CBUF_PAGE_NUM;
		pcm_dev->rxpindex = (pcm_dev->rxpindex + 1)%2;

		unNextWrite = (pcm_dev->rx_W + 1)%CBUF_PAGE_NUM;

		count++;
	}

	spin_unlock(&pcm_dev->lock);

	mod_timer(&pcm_dev->timer, jiffies + TIME_INTERVAL);
}

#else

//---------------------------------------------------------------------------------------------
//	Interrupt service routine
//---------------------------------------------------------------------------------------------
/*
void pcm_isr(struct pcm_priv* pcm_dev, unsigned int isr)
{

	unsigned int channel = pcm_dev->channel;

	

}*/

//write data to PCM controller from circular buffer.
void pcm_tx_isr(struct pcm_priv* pcm_dev, unsigned int isr)
{

#if 0
	//if(down_interruptible (&pcm_dev->sem))
	//	return;

	unsigned char *pageaddr;
	unsigned int channel = pcm_dev->channel;
	
	while(pcm_dev->tx_R != pcm_dev->tx_W && (pcm_inl(TX_BSA(pcm_dev->channel)) & (0x1 << pcm_dev->txpindex)) == 0 )
	{
		pageaddr = (unsigned char *)((unsigned int)pcm_dev->tx_buffer + (pcm_dev->txpindex * PCMPAGE_SIZE));
		memcpy(pageaddr, pcm_dev->tx_cb[pcm_dev->tx_R], PCMPAGE_SIZE);
		PDBUG("tx_R = %d, tx_W = %d\n", pcm_dev->tx_R, pcm_dev->tx_W);
		pcm_dev->tx_R = (pcm_dev->tx_R + 1)%CBUF_PAGE_NUM;
		//poll tx
		pcm_outl(TX_BSA(channel), (pcm_inl(TX_BSA(channel)) | (0x1 << pcm_dev->txpindex)));
	}

	//up(&pcm_dev->sem);
#endif
}

#ifdef READ_TEST
unsigned char g_seed;
#endif


//write received data to circular buffer of rx
void pcmctrl_rx_write_cpage(struct pcm_priv* pcm_dev, const unsigned char* buf)
{
	//PDBUG("RX: rx_R = %d, rx_W = %d, rxpindex = %d\n", pcm_dev->rx_R, pcm_dev->rx_W, pcm_dev->rxpindex);
	
	//printk("%s : rx_R = %d, rx_W = %d, rxpindex = %d\n", __FUNCTION__, pcm_dev->rx_R, pcm_dev->rx_W, pcm_dev->rxpindex);

#ifdef READ_TEST
	memset(pcm_dev->rx_circular + (pcm_dev->rx_W * PCMPAGE_SIZE), g_seed, PCMPAGE_SIZE);
	g_seed++;
#else
	//printk("%s : rx_circular + rx_W * PCMPAGE_SIZE = 0x%X\n", __FUNCTION__, pcm_dev->rx_circular + (pcm_dev->rx_W * PCMPAGE_SIZE));
	memcpy(pcm_dev->rx_circular + (pcm_dev->rx_W * PCMPAGE_SIZE), buf, PCMPAGE_SIZE);
#endif
	

	//There may be a bug.  e.g. CPU read OWN=01 => CPU set OWN=11 => hardware set OWN=00 => CPU set OWN=11
	pcm_outl(RX_BSA(pcm_dev->channel), pcm_inl(RX_BSA(pcm_dev->channel)) | BIT(pcm_dev->rxpindex));

	pcm_dev->rx_W = (pcm_dev->rx_W + 1)%CBUF_PAGE_NUM;
	pcm_dev->rxpindex = (pcm_dev->rxpindex + 1)%2;
}

#ifdef LOOPBACK
unsigned int g_test;
#endif


//void pcm_rx_isr(struct pcm_priv* pcm_dev, unsigned int isr)
void pcm_rx_isr(unsigned long task_priv)
{
	struct pcm_priv* pcm_dev = (struct pcm_priv *)task_priv;
	
	unsigned char* pageaddr;
	unsigned int count = 0;
	unsigned int channel = pcm_dev->channel;

#ifndef LOOPBACK
	unsigned int unNextWrite = (pcm_dev->rx_W + 1)%CBUF_PAGE_NUM;
#endif

/*
	while( !((pcm_inl(RX_BSA(pcm_dev->channel)) >> pcm_dev->rxpindex) & 0x1) && (unNextWrite != pcm_dev->rx_R) )
	{
		pageaddr = (unsigned char *)((unsigned int)pcm_dev->rx_buffer + (PCMPAGE_SIZE * pcm_dev->rxpindex));
		pcmctrl_rx_write_cpage(pcm_dev, (char *)pageaddr);
		pcm_dev->rxpindex = (pcm_dev->rxpindex + 1)%2;
		unNextWrite = (pcm_dev->rx_W + 1)%CBUF_PAGE_NUM;
	}
*/
//	unsigned int rx_own = pcm_inl(RX_BSA(channel)) & OWN_MASK;
	//printk("\n");
	//PDBUG("Enter rx ISR, ISR = 0x%X, OWN = 0x%X\n", isr, rx_own);

	//PDBUG("pcm_inl(RX_BSA()) = 0x%X, rx_buffer = 0x%X, rxpindex = %d\n", pcm_inl(RX_BSA(pcm_dev->channel)),pcm_dev->rx_buffer,pcm_dev->rxpindex );

	while( !((pcm_inl(RX_BSA(channel)) >> pcm_dev->rxpindex) & 0x1) && count < 2)
	//if( !((pcm_inl(RX_BSA(pcm_dev->channel)) >> pcm_dev->rxpindex) & 0x1))
	{
		//PDBUG("RXBSA=0x%X ,TXBSA=0x%X,ISR=0x%X,rxpindex=%d,pindex=%d\n", pcm_inl(RX_BSA(pcm_dev->channel)), pcm_inl(TX_BSA(pcm_dev->channel)), isr, pcm_dev->rxpindex, pcm_dev->txpindex);


		//PDBUG("rx_R = %2d, rx_W = %2d\n", pcm_dev->rx_R, pcm_dev->rx_W);


#ifndef LOOPBACK
		//buffer overflow. move R pointer to next one. Discard old data.
		if(unNextWrite == pcm_dev->rx_R)	
		{
			pcm_dev->rx_R = (pcm_dev->rx_R + 1)%CBUF_PAGE_NUM;
			pcm_dev->rx_overflow = TRUE;
		}
#endif

		pageaddr = (unsigned char *)(pcm_dev->rx_buffer + (PCMPAGE_SIZE * pcm_dev->rxpindex));


#ifdef LOOPBACK

		if(!((pcm_inl(TX_BSA(channel)) >> pcm_dev->txpindex) & 0x1))
		{
			unsigned char* txaddr = (unsigned char *)((unsigned int)pcm_dev->tx_buffer + (PCMPAGE_SIZE * pcm_dev->txpindex));
//			printk("\n");
			PDBUG("txaddr = 0x%X, pageaddr = 0x%X\n", (unsigned int)txaddr, (unsigned int)pageaddr);

			memcpy(txaddr, pageaddr, PCMPAGE_SIZE);
			
			pcm_outl(TX_BSA(channel), (pcm_inl(TX_BSA(channel))  | BIT(pcm_dev->txpindex)));

			pcm_dev->txpindex = (pcm_dev->txpindex + 1)%2;
		}
		else 
		{

			PDBUG("TXUW\n");
		}

		pcm_outl(RX_BSA(channel), pcm_inl(RX_BSA(channel)) | BIT(pcm_dev->rxpindex));
		pcm_dev->rxpindex = (pcm_dev->rxpindex + 1)%2;


#else
		//write page to circular buffer.
		pcmctrl_rx_write_cpage(pcm_dev, pageaddr);
		unNextWrite = (pcm_dev->rx_W + 1)%CBUF_PAGE_NUM;

#endif	//LOOPBACK

		//pcm_dev->rxpindex = (pcm_dev->rxpindex + 1)%2;
		count++;
	}

	//PDBUG("----------------------------------------\n");


}


void pcm_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	//unsigned int status;

	struct pcm_priv* pcm_dev = (struct pcm_priv *)dev_id;
	unsigned int channel = pcm_dev->channel;
	unsigned int maskval;

	
	//if(pcm_inl(PCMIMR) && (status = pcm_inl(PCMISR)))
	if(pcm_inl(PCMIMR) && (maskval = (pcm_inl(PCMISR) & ISR_MASK(channel))))
	{
		unsigned long flags;
		save_flags(flags); cli();

		//if((maskval = (status & ISR_MASK(channel))))
		{

			if(maskval & ROK_MASK(channel))
			{
#ifdef USE_TASKLET
				tasklet_hi_schedule(&pcm_dev->rx_tasklet);
#else
				pcm_rx_isr((unsigned long)pcm_dev);
#endif
			}

			/*if(maskval & TOK_MASK(channel))
			{
				PDBUG("TOK\n");
			}
			*/

			if(maskval & RBU_MASK(channel))
			{
				//PDBUG("P0 or P1 rx buffer unavailable.\n");
				printk("RBU, RX_BSA = 0x%X, rxpindex=%d\n", pcm_inl(RX_BSA(channel)), pcm_dev->rxpindex);
			}

/*
			if(maskval & TBU_MASK(channel))
			{
				PDBUG("TBU, TX_BSA = 0x%X\n", pcm_inl(TX_BSA(channel)));
				//PDBUG("P0 or P1 tx buffer unavailable.\n");
			}
*/			
			//clean interrupt pending bits.
			pcm_outl(PCMISR, maskval);

			
		}

		restore_flags(flags);

	}
}

#endif	//TIMER_MODE

module_init(pcmctrl_init);
module_exit(pcmctrl_cleanup);


