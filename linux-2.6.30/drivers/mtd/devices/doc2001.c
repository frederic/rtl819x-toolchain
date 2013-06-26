
/*
 * Linux driver for Disk-On-Chip Millennium
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/bitops.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/doc2000.h>

#ifdef CONFIG_RTL_819X
/*porting for RTL865xC-RTL8190 SDK by alva_zhang@2007.11*/
#include <linux/config.h>
#include <linux/mtd/partitions.h>

#define CALL_APP_TO_LOAD_DEFAULT	// call user program to load default
extern int flash_hw_start;
#define noCONFIG_MTD_DEBUG
#define CONFIG_MTD_DEBUG_VERBOSE 3
extern int flash_hw_start, flash_hw_len, flash_ds_start, flash_ds_len, flash_write_flag;
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
#define RTL_BOOTCODE_END (0x6000)
static struct mtd_partition rtl8196_partitions[ ] = {
        {
                name:         "boot+cfg+linux",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "root fs",
                   size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }
};
#else
static struct mtd_partition rtl8196_partitions[ ] = {
        {
                name:         "boot+cfg+linux",
                size:           0xF0000,
                offset:         0x00000000,
        },
        {
                name:           "root fs",
                   size:        0x110000,
                offset:         0xF0000,
        }
};
#endif
#define NB_OF(x)  (sizeof(x)/sizeof(x[0]))
#endif /*#ifdef CONFIG_RTL_819X */


/* #define ECC_DEBUG */

/* I have no idea why some DoC chips can not use memcop_form|to_io().
 * This may be due to the different revisions of the ASIC controller built-in or
 * simplily a QA/Bug issue. Who knows ?? If you have trouble, please uncomment
 * this:*/
#undef USE_MEMCPY

static int doc_read(struct mtd_info *mtd, loff_t from, size_t len,
		    size_t *retlen, u_char *buf);
static int doc_write(struct mtd_info *mtd, loff_t to, size_t len,
		     size_t *retlen, const u_char *buf);

#ifdef CONFIG_RTL_819X
/* Do nothing here*/
#else
static int doc_read_oob(struct mtd_info *mtd, loff_t ofs,
			struct mtd_oob_ops *ops);
static int doc_write_oob(struct mtd_info *mtd, loff_t ofs,
			 struct mtd_oob_ops *ops);
#endif

static int doc_erase (struct mtd_info *mtd, struct erase_info *instr);

#ifdef CONFIG_RTL_819X
/*porting for RTL865xC-RTL8190 SDK by alva_zhang@2007.11*/
static int erase_one_block(struct DiskOnChip *this, __u32 addr, __u32 len);
#endif

#ifdef CONFIG_RTL_819X
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
int find_section_boundary(struct mtd_info *mtd,unsigned int start, unsigned int end, unsigned int *rstart, unsigned *rend);
#endif
#endif

static struct mtd_info *docmillist = NULL;

/* Perform the required delay cycles by reading from the NOP register */
static void DoC_Delay(void __iomem * docptr, unsigned short cycles)
{
	volatile char dummy;
	int i;

	for (i = 0; i < cycles; i++)
		dummy = ReadDOC(docptr, NOP);
}

/* DOC_WaitReady: Wait for RDY line to be asserted by the flash chip */
static int _DoC_WaitReady(void __iomem * docptr)
{
	unsigned short c = 0xffff;

	DEBUG(MTD_DEBUG_LEVEL3,
	      "_DoC_WaitReady called for out-of-line wait\n");

	/* Out-of-line routine to wait for chip response */
	while (!(ReadDOC(docptr, CDSNControl) & CDSN_CTRL_FR_B) && --c)
		;

	if (c == 0)
		DEBUG(MTD_DEBUG_LEVEL2, "_DoC_WaitReady timed out.\n");

	return (c == 0);
}

static inline int DoC_WaitReady(void __iomem * docptr)
{
	/* This is inline, to optimise the common case, where it's ready instantly */
	int ret = 0;

	/* 4 read form NOP register should be issued in prior to the read from CDSNControl
	   see Software Requirement 11.4 item 2. */
	DoC_Delay(docptr, 4);

	if (!(ReadDOC(docptr, CDSNControl) & CDSN_CTRL_FR_B))
		/* Call the out-of-line routine to wait */
		ret = _DoC_WaitReady(docptr);

	/* issue 2 read from NOP register after reading from CDSNControl register
	   see Software Requirement 11.4 item 2. */
	DoC_Delay(docptr, 2);

	return ret;
}

/* DoC_Command: Send a flash command to the flash chip through the CDSN IO register
   with the internal pipeline. Each of 4 delay cycles (read from the NOP register) is
   required after writing to CDSN Control register, see Software Requirement 11.4 item 3. */

static void DoC_Command(void __iomem * docptr, unsigned char command,
			       unsigned char xtraflags)
{
	/* Assert the CLE (Command Latch Enable) line to the flash chip */
	WriteDOC(xtraflags | CDSN_CTRL_CLE | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);

	/* Send the command */
	WriteDOC(command, docptr, Mil_CDSN_IO);
	WriteDOC(0x00, docptr, WritePipeTerm);

	/* Lower the CLE line */
	WriteDOC(xtraflags | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);
}

/* DoC_Address: Set the current address for the flash chip through the CDSN IO register
   with the internal pipeline. Each of 4 delay cycles (read from the NOP register) is
   required after writing to CDSN Control register, see Software Requirement 11.4 item 3. */

static inline void DoC_Address(void __iomem * docptr, int numbytes, unsigned long ofs,
			       unsigned char xtraflags1, unsigned char xtraflags2)
{
	/* Assert the ALE (Address Latch Enable) line to the flash chip */
	WriteDOC(xtraflags1 | CDSN_CTRL_ALE | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);

	/* Send the address */
	switch (numbytes)
	    {
	    case 1:
		    /* Send single byte, bits 0-7. */
		    WriteDOC(ofs & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC(0x00, docptr, WritePipeTerm);
		    break;
	    case 2:
		    /* Send bits 9-16 followed by 17-23 */
		    WriteDOC((ofs >> 9)  & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC((ofs >> 17) & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC(0x00, docptr, WritePipeTerm);
		break;
	    case 3:
		    /* Send 0-7, 9-16, then 17-23 */
		    WriteDOC(ofs & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC((ofs >> 9)  & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC((ofs >> 17) & 0xff, docptr, Mil_CDSN_IO);
		    WriteDOC(0x00, docptr, WritePipeTerm);
		break;
	    default:
		return;
	    }

	/* Lower the ALE line */
	WriteDOC(xtraflags1 | xtraflags2 | CDSN_CTRL_CE, docptr, CDSNControl);
	DoC_Delay(docptr, 4);
}

#ifdef CONFIG_RTL_819X
/*porting for RTL865xC-RTL8190 SDK by alva_zhang@2007.11*/
/* Do nothing here*/
#else
/* DoC_SelectChip: Select a given flash chip within the current floor */
static int DoC_SelectChip(void __iomem * docptr, int chip)
{
	/* Select the individual flash chip requested */
	WriteDOC(chip, docptr, CDSNDeviceSelect);
	DoC_Delay(docptr, 4);

	/* Wait for it to be ready */
	return DoC_WaitReady(docptr);
}

/* DoC_SelectFloor: Select a given floor (bank of flash chips) */
static int DoC_SelectFloor(void __iomem * docptr, int floor)
{
	/* Select the floor (bank) of chips required */
	WriteDOC(floor, docptr, FloorSelect);

	/* Wait for the chip to be ready */
	return DoC_WaitReady(docptr);
}

/* DoC_IdentChip: Identify a given NAND chip given {floor,chip} */
static int DoC_IdentChip(struct DiskOnChip *doc, int floor, int chip)
{
	int mfr, id, i, j;
	volatile char dummy;

	/* Page in the required floor/chip
	   FIXME: is this supported by Millennium ?? */
	DoC_SelectFloor(doc->virtadr, floor);
	DoC_SelectChip(doc->virtadr, chip);

	/* Reset the chip, see Software Requirement 11.4 item 1. */
	DoC_Command(doc->virtadr, NAND_CMD_RESET, CDSN_CTRL_WP);
	DoC_WaitReady(doc->virtadr);

	/* Read the NAND chip ID: 1. Send ReadID command */
	DoC_Command(doc->virtadr, NAND_CMD_READID, CDSN_CTRL_WP);

	/* Read the NAND chip ID: 2. Send address byte zero */
	DoC_Address(doc->virtadr, 1, 0x00, CDSN_CTRL_WP, 0x00);

	/* Read the manufacturer and device id codes of the flash device through
	   CDSN IO register see Software Requirement 11.4 item 5.*/
	dummy = ReadDOC(doc->virtadr, ReadPipeInit);
	DoC_Delay(doc->virtadr, 2);
	mfr = ReadDOC(doc->virtadr, Mil_CDSN_IO);

	DoC_Delay(doc->virtadr, 2);
	id  = ReadDOC(doc->virtadr, Mil_CDSN_IO);
	dummy = ReadDOC(doc->virtadr, LastDataRead);

	/* No response - return failure */
	if (mfr == 0xff || mfr == 0)
		return 0;

	/* FIXME: to deal with multi-flash on multi-Millennium case more carefully */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if ( id == nand_flash_ids[i].id) {
			/* Try to identify manufacturer */
			for (j = 0; nand_manuf_ids[j].id != 0x0; j++) {
				if (nand_manuf_ids[j].id == mfr)
					break;
			}
			printk(KERN_INFO "Flash chip found: Manufacturer ID: %2.2X, "
			       "Chip ID: %2.2X (%s:%s)\n",
			       mfr, id, nand_manuf_ids[j].name, nand_flash_ids[i].name);
			doc->mfr = mfr;
			doc->id = id;
			doc->chipshift = ffs((nand_flash_ids[i].chipsize << 20)) - 1;
			break;
		}
	}

	if (nand_flash_ids[i].name == NULL)
		return 0;
	else
		return 1;
}

/* DoC_ScanChips: Find all NAND chips present in a DiskOnChip, and identify them */
static void DoC_ScanChips(struct DiskOnChip *this)
{
	int floor, chip;
	int numchips[MAX_FLOORS_MIL];
	int ret;

	this->numchips = 0;
	this->mfr = 0;
	this->id = 0;

	/* For each floor, find the number of valid chips it contains */
	for (floor = 0,ret = 1; floor < MAX_FLOORS_MIL; floor++) {
		numchips[floor] = 0;
		for (chip = 0; chip < MAX_CHIPS_MIL && ret != 0; chip++) {
			ret = DoC_IdentChip(this, floor, chip);
			if (ret) {
				numchips[floor]++;
				this->numchips++;
			}
		}
	}
	/* If there are none at all that we recognise, bail */
	if (!this->numchips) {
		printk("No flash chips recognised.\n");
		return;
	}

	/* Allocate an array to hold the information for each chip */
	this->chips = kmalloc(sizeof(struct Nand) * this->numchips, GFP_KERNEL);
	if (!this->chips){
		printk("No memory for allocating chip info structures\n");
		return;
	}

	/* Fill out the chip array with {floor, chipno} for each
	 * detected chip in the device. */
	for (floor = 0, ret = 0; floor < MAX_FLOORS_MIL; floor++) {
		for (chip = 0 ; chip < numchips[floor] ; chip++) {
			this->chips[ret].floor = floor;
			this->chips[ret].chip = chip;
			this->chips[ret].curadr = 0;
			this->chips[ret].curmode = 0x50;
			ret++;
		}
	}

	/* Calculate and print the total size of the device */
	this->totlen = this->numchips * (1 << this->chipshift);
	printk(KERN_INFO "%d flash chips found. Total DiskOnChip size: %ld MiB\n",
	       this->numchips ,this->totlen >> 20);
}
#endif /*#ifdef CONFIG_RTL_819X */

static int DoCMil_is_alias(struct DiskOnChip *doc1, struct DiskOnChip *doc2)
{
	int tmp1, tmp2, retval;

	if (doc1->physadr == doc2->physadr)
		return 1;

	/* Use the alias resolution register which was set aside for this
	 * purpose. If it's value is the same on both chips, they might
	 * be the same chip, and we write to one and check for a change in
	 * the other. It's unclear if this register is usuable in the
	 * DoC 2000 (it's in the Millenium docs), but it seems to work. */
	tmp1 = ReadDOC(doc1->virtadr, AliasResolution);
	tmp2 = ReadDOC(doc2->virtadr, AliasResolution);
	if (tmp1 != tmp2)
		return 0;

	WriteDOC((tmp1+1) % 0xff, doc1->virtadr, AliasResolution);
	tmp2 = ReadDOC(doc2->virtadr, AliasResolution);
	if (tmp2 == (tmp1+1) % 0xff)
		retval = 1;
	else
		retval = 0;

	/* Restore register contents.  May not be necessary, but do it just to
	 * be safe. */
	WriteDOC(tmp1, doc1->virtadr, AliasResolution);

	return retval;
}

/* This routine is found from the docprobe code by symbol_get(),
 * which will bump the use count of this module. */
void DoCMil_init(struct mtd_info *mtd)
{
	struct DiskOnChip *this = mtd->priv;
#ifdef CONFIG_RTL_819X 
/*do nothing here*/
#else
	struct DiskOnChip *old = NULL;
#endif

	/* We must avoid being called twice for the same device. */
#ifdef CONFIG_RTL_819X 
/*do nothing here*/
#else
	if (docmillist)
		old = docmillist->priv;

	while (old) {
		if (DoCMil_is_alias(this, old)) {
			printk(KERN_NOTICE "Ignoring DiskOnChip Millennium at "
			       "0x%lX - already configured\n", this->physadr);
			iounmap(this->virtadr);
			kfree(mtd);
			return;
		}
		if (old->nextdoc)
			old = old->nextdoc->priv;
		else
			old = NULL;
	}
#endif /*CONFIG_RTL_819X*/

	mtd->name = "DiskOnChip Millennium";
#ifdef CONFIG_RTL_819X 
/*do nothing here*/
#else
	printk(KERN_NOTICE "DiskOnChip Millennium found at address 0x%lX\n",
	       this->physadr);
#endif

	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
#ifdef CONFIG_RTL_819X
#else
	mtd->ecctype = MTD_ECC_RS_DiskOnChip;
#endif

#ifdef CONFIG_RTL_819X 
/*do nothing here*/
#else	
	mtd->size = 0;

	/* FIXME: erase size is not always 8KiB */
	mtd->erasesize = 0x2000;
#endif

	mtd->writesize = 512;
	mtd->oobsize = 16;
	mtd->owner = THIS_MODULE;
	mtd->erase = doc_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = doc_read;
	mtd->write = doc_write;

#ifdef CONFIG_RTL_819X 
/*do nothing here*/
#else		
	mtd->read_oob = doc_read_oob;
	mtd->write_oob = doc_write_oob;
#endif

	mtd->sync = NULL;

#ifdef CONFIG_RTL_819X 
/*do nothing here*/
#else	
	this->totlen = 0;
	this->numchips = 0;
	this->curfloor = -1;
	this->curchip = -1;

	/* Ident all the chips present. */
	DoC_ScanChips(this);
#endif

	if (!this->totlen) {
		kfree(mtd);
		iounmap(this->virtadr);
	} else {
		this->nextdoc = docmillist;
		docmillist = mtd;
		mtd->size  = this->totlen;
//#ifdef CONFIG_RTK_MTD_ROOT
#ifdef CONFIG_RTL_819X
		add_mtd_partitions(mtd, rtl8196_partitions,  NB_OF(rtl8196_partitions));
#else
		add_mtd_device(mtd);
#endif
		return;
	}
}
EXPORT_SYMBOL_GPL(DoCMil_init);

#ifdef CONFIG_RTL_819X 
static int doc_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const u_char *buf, u_char *eccbuf)
{
	int i,ret;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long docptr =(unsigned long) this->virtadr;
	unsigned int ofs;
	unsigned short val,val1;
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
	unsigned int rstart,rend;
	unsigned int start,end;
#endif
// david ------------
unsigned long timeo, offset;
unsigned long flags; 
//-------------------

	/* Don't allow write past end of device */
	if (to >= this->totlen)
	{
// david
//		printk("write to >= total len\n");
	printk(KERN_WARNING "write to >= total len\n");
		return -EINVAL;
	}
	DEBUG(MTD_DEBUG_LEVEL1,"going to write len=0x%x,to =0x%x\n", (int)len, (int)to);

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
	start=to;
	end=0xFFFFFFF;
	if(flash_write_flag & 1)
	{
		if(0 == start)
			start = CONFIG_RTL_HW_SETTING_OFFSET;
		else if( start > CONFIG_RTL_HW_SETTING_OFFSET )
			start = CONFIG_RTL_HW_SETTING_OFFSET;
		end=CONFIG_RTL_DEFAULT_SETTING_OFFSET;
	}
			
	if(flash_write_flag & 2 )
	{
		if(0 == start)
			start = CONFIG_RTL_DEFAULT_SETTING_OFFSET;
		else if( start > CONFIG_RTL_DEFAULT_SETTING_OFFSET )
			start = CONFIG_RTL_DEFAULT_SETTING_OFFSET;
		
		end = CONFIG_RTL_CURRENT_SETTING_OFFSET;
	}
			
	if( flash_write_flag & 4 )
	{
		if(0 == start)
			start = CONFIG_RTL_CURRENT_SETTING_OFFSET;
		else if( start > CONFIG_RTL_CURRENT_SETTING_OFFSET )
			start = CONFIG_RTL_CURRENT_SETTING_OFFSET;	
		end = CONFIG_RTL_WEB_PAGES_OFFSET;
	}
	find_section_boundary(mtd,start,end,&rstart,&rend);

#endif

	*retlen = len;
	ofs = docptr + to;
	for(i=0; i< len; i+=2)
	{
// david -----------------------------------------------------
#if 0
		val = *(unsigned short *)buf;

		*(volatile unsigned short *)(0xbfc00000 + 0x555 * 2)= 0xaa;
		*(volatile unsigned short *)(0xbfc00000 + 0x2aa * 2)= 0x55;
		*(volatile unsigned short *)(0xbfc00000 + 0x555 * 2)= 0xa0;
		*(volatile unsigned short *)(ofs )= val;

	       	j=0xfffff1;
        	do{
                	val1=*(volatile unsigned short *)(ofs);
  	            	if( ((val1^val) & 0x80)==0 )
 				break;

       		}while(j--!=1);
		if (j <= 2)
			printk("program fails\n");
#else

//		if ( ofs < (docptr+CONFIG_MTD_DOCPROBE_ADDRESS) )
//			goto next_word;

		offset = (to >> this->chipshift)*(1 << this->chipshift);
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
		if(ofs <(docptr+rstart))
			goto next_word;
		if(ofs >= (docptr+rend))
		{
			return 0;
		}
#else
#if !defined(COMPACK_SETTING) && !defined(NO_CHECK_REGION)
		if ( flash_write_flag != 0x8000
//#ifdef CONFIG_RTK_MTD_ROOT
#ifdef CONFIG_RTL_819X
			|| offset < (rtl8196_partitions[0].size+ rtl8196_partitions[0].offset)
#endif
			)
		{

			if ( (flash_write_flag & 1) && (ofs < (docptr+flash_hw_start)) )
				goto next_word;

			if ( (flash_write_flag & 2) && (ofs < (docptr+flash_ds_start)) )
				goto next_word;

			if ( (flash_write_flag & 4) && (ofs < (docptr+flash_ds_start+flash_ds_len)) )
				goto next_word;
		}
#endif // COMPACK_SETTING && NO_CHECK_REGION
#endif //CONFIG_RTL_FLASH_MAPPING_ENABLE
		val = *(unsigned short *)buf;

		mtd_save_flags(flags);mtd_cli(); // david
		
		*(volatile unsigned short *)(0xbfc00000 + offset + 0x555 * 2)= 0xaa;
		*(volatile unsigned short *)(0xbfc00000 + offset + 0x2aa * 2)= 0x55;
		*(volatile unsigned short *)(0xbfc00000 + offset + 0x555 * 2)= 0xa0;
		*(volatile unsigned short *)(ofs )= val;
		
		mtd_restore_flags(flags); // david

		timeo = jiffies + (HZ * 50);
        	do{    		
#if 0
			val1=*(volatile unsigned short *)(ofs);
			if ( val1 == val )
				break;
#endif
			unsigned short val2;
      	
			val2=*(volatile unsigned short *)(ofs);
			val1=*(volatile unsigned short *)(ofs);	
			
			if (((val1^val2) & 0x40) != 0)
				continue;	 	        
	 	        if (((val1^val) & 0x80) != 0)
 				continue;
 			if ( val1 == val )		
				break;
//--------------	      	    	
		} while ( !time_after(jiffies, timeo) );

		if ( time_after(jiffies, timeo)) {
			printk(KERN_WARNING "program timeout!");
			printk(KERN_WARNING " write: %x, read:%x, addr: %x\n", val, val1, ofs);
			return -1;
		}

#ifndef COMPACK_SETTING
next_word:
#endif

#endif
//---------------------------------------------------------
		ofs += 2;
		buf += 2;
	
	}

	ret = 0 ;
//	printk("in doc_write_ecc ret=%08x\n", ret);
	return ret;
}
#endif /* #ifdef CONFIG_RTL_819X  */

static int doc_read (struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, u_char *buf)
{
#ifdef CONFIG_RTL_819X 
static int doc_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
			 size_t *retlen, u_char *buf, u_char *eccbuf);
/* Just a special case of doc_read_ecc */
	return doc_read_ecc(mtd, from, len, retlen, buf, NULL);
#else
	int i, ret;
	volatile char dummy;
	unsigned char syndrome[6], eccbuf[6];
	struct DiskOnChip *this = mtd->priv;
	void __iomem *docptr = this->virtadr;
	struct Nand *mychip = &this->chips[from >> (this->chipshift)];

	/* Don't allow read past end of device */
	if (from >= this->totlen)
		return -EINVAL;

	/* Don't allow a single read to cross a 512-byte block boundary */
	if (from + len > ((from | 0x1ff) + 1))
		len = ((from | 0x1ff) + 1) - from;

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(docptr, mychip->floor);
		DoC_SelectChip(docptr, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(docptr, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* issue the Read0 or Read1 command depend on which half of the page
	   we are accessing. Polling the Flash Ready bit after issue 3 bytes
	   address in Sequence Read Mode, see Software Requirement 11.4 item 1.*/
	DoC_Command(docptr, (from >> 8) & 1, CDSN_CTRL_WP);
	DoC_Address(docptr, 3, from, CDSN_CTRL_WP, 0x00);
	DoC_WaitReady(docptr);

	/* init the ECC engine, see Reed-Solomon EDC/ECC 11.1 .*/
	WriteDOC (DOC_ECC_RESET, docptr, ECCConf);
	WriteDOC (DOC_ECC_EN, docptr, ECCConf);

	/* Read the data via the internal pipeline through CDSN IO register,
	   see Pipelined Read Operations 11.3 */
	dummy = ReadDOC(docptr, ReadPipeInit);
#ifndef USE_MEMCPY
	for (i = 0; i < len-1; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		buf[i] = ReadDOC(docptr, Mil_CDSN_IO + (i & 0xff));
	}
#else
	memcpy_fromio(buf, docptr + DoC_Mil_CDSN_IO, len - 1);
#endif
	buf[len - 1] = ReadDOC(docptr, LastDataRead);

	/* Let the caller know we completed it */
	*retlen = len;
        ret = 0;

	/* Read the ECC data from Spare Data Area,
	   see Reed-Solomon EDC/ECC 11.1 */
	dummy = ReadDOC(docptr, ReadPipeInit);
#ifndef USE_MEMCPY
	for (i = 0; i < 5; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		eccbuf[i] = ReadDOC(docptr, Mil_CDSN_IO + i);
	}
#else
	memcpy_fromio(eccbuf, docptr + DoC_Mil_CDSN_IO, 5);
#endif
	eccbuf[5] = ReadDOC(docptr, LastDataRead);

	/* Flush the pipeline */
	dummy = ReadDOC(docptr, ECCConf);
	dummy = ReadDOC(docptr, ECCConf);

	/* Check the ECC Status */
	if (ReadDOC(docptr, ECCConf) & 0x80) {
		int nb_errors;
		/* There was an ECC error */
#ifdef ECC_DEBUG
		printk("DiskOnChip ECC Error: Read at %lx\n", (long)from);
#endif
		/* Read the ECC syndrom through the DiskOnChip ECC logic.
		   These syndrome will be all ZERO when there is no error */
		for (i = 0; i < 6; i++) {
			syndrome[i] = ReadDOC(docptr, ECCSyndrome0 + i);
		}
		nb_errors = doc_decode_ecc(buf, syndrome);
#ifdef ECC_DEBUG
		printk("ECC Errors corrected: %x\n", nb_errors);
#endif
		if (nb_errors < 0) {
			/* We return error, but have actually done the read. Not that
			   this can be told to user-space, via sys_read(), but at least
			   MTD-aware stuff can know about it by checking *retlen */
			ret = -EIO;
		}
	}

#ifdef PSYCHO_DEBUG
	printk("ECC DATA at %lx: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
	       (long)from, eccbuf[0], eccbuf[1], eccbuf[2], eccbuf[3],
	       eccbuf[4], eccbuf[5]);
#endif

	/* disable the ECC engine */
	WriteDOC(DOC_ECC_DIS, docptr , ECCConf);

	return ret;

#endif /* #ifdef CONFIG_RTL_819X  */
}

#ifdef CONFIG_RTL_819X 
static int doc_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
			 size_t *retlen, u_char *buf, u_char *eccbuf)
{
	int i;
	unsigned short tmp;
	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long docptr = this->virtadr+from;

	/* Don't allow read past end of device */
	if (from >= this->totlen)
		return -EINVAL;
	for(i=0; i< len; i+=2)
	{
		tmp = *(volatile unsigned short *)(docptr);
		*(unsigned short *)buf = tmp;
		buf += 2;
		docptr +=2;
	}
	if (len & 0x01)
	{
		tmp = *(volatile unsigned long *)(docptr);
        *(unsigned char *)buf = (tmp >> 8) & 0xff;
	}

	/* Let the caller know we completed it */
	*retlen = len;

	return 0;
}
#endif /*#ifdef CONFIG_RTL_819X */

static int doc_write (struct mtd_info *mtd, loff_t to, size_t len,
		      size_t *retlen, const u_char *buf)
{
#ifdef CONFIG_RTL_819X 
       char eccbuf[6];
	return doc_write_ecc(mtd, to, len, retlen, buf, eccbuf);
#else
	int i,ret = 0;
	char eccbuf[6];
	volatile char dummy;
	struct DiskOnChip *this = mtd->priv;
	void __iomem *docptr = this->virtadr;
	struct Nand *mychip = &this->chips[to >> (this->chipshift)];

	/* Don't allow write past end of device */
	if (to >= this->totlen)
		return -EINVAL;

#if 0
	/* Don't allow a single write to cross a 512-byte block boundary */
	if (to + len > ( (to | 0x1ff) + 1))
		len = ((to | 0x1ff) + 1) - to;
#else
	/* Don't allow writes which aren't exactly one block */
	if (to & 0x1ff || len != 0x200)
		return -EINVAL;
#endif

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(docptr, mychip->floor);
		DoC_SelectChip(docptr, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(docptr, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* Reset the chip, see Software Requirement 11.4 item 1. */
	DoC_Command(docptr, NAND_CMD_RESET, 0x00);
	DoC_WaitReady(docptr);
	/* Set device to main plane of flash */
	DoC_Command(docptr, NAND_CMD_READ0, 0x00);

	/* issue the Serial Data In command to initial the Page Program process */
	DoC_Command(docptr, NAND_CMD_SEQIN, 0x00);
	DoC_Address(docptr, 3, to, 0x00, 0x00);
	DoC_WaitReady(docptr);

	/* init the ECC engine, see Reed-Solomon EDC/ECC 11.1 .*/
	WriteDOC (DOC_ECC_RESET, docptr, ECCConf);
	WriteDOC (DOC_ECC_EN | DOC_ECC_RW, docptr, ECCConf);

	/* Write the data via the internal pipeline through CDSN IO register,
	   see Pipelined Write Operations 11.2 */
#ifndef USE_MEMCPY
	for (i = 0; i < len; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		WriteDOC(buf[i], docptr, Mil_CDSN_IO + i);
	}
#else
	memcpy_toio(docptr + DoC_Mil_CDSN_IO, buf, len);
#endif
	WriteDOC(0x00, docptr, WritePipeTerm);

	/* Write ECC data to flash, the ECC info is generated by the DiskOnChip ECC logic
	   see Reed-Solomon EDC/ECC 11.1 */
	WriteDOC(0, docptr, NOP);
	WriteDOC(0, docptr, NOP);
	WriteDOC(0, docptr, NOP);

	/* Read the ECC data through the DiskOnChip ECC logic */
	for (i = 0; i < 6; i++) {
		eccbuf[i] = ReadDOC(docptr, ECCSyndrome0 + i);
	}

	/* ignore the ECC engine */
	WriteDOC(DOC_ECC_DIS, docptr , ECCConf);

#ifndef USE_MEMCPY
	/* Write the ECC data to flash */
	for (i = 0; i < 6; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		WriteDOC(eccbuf[i], docptr, Mil_CDSN_IO + i);
	}
#else
	memcpy_toio(docptr + DoC_Mil_CDSN_IO, eccbuf, 6);
#endif

	/* write the block status BLOCK_USED (0x5555) at the end of ECC data
	   FIXME: this is only a hack for programming the IPL area for LinuxBIOS
	   and should be replace with proper codes in user space utilities */
	WriteDOC(0x55, docptr, Mil_CDSN_IO);
	WriteDOC(0x55, docptr, Mil_CDSN_IO + 1);

	WriteDOC(0x00, docptr, WritePipeTerm);

#ifdef PSYCHO_DEBUG
	printk("OOB data at %lx is %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
	       (long) to, eccbuf[0], eccbuf[1], eccbuf[2], eccbuf[3],
	       eccbuf[4], eccbuf[5]);
#endif

	/* Commit the Page Program command and wait for ready
	   see Software Requirement 11.4 item 1.*/
	DoC_Command(docptr, NAND_CMD_PAGEPROG, 0x00);
	DoC_WaitReady(docptr);

	/* Read the status of the flash device through CDSN IO register
	   see Software Requirement 11.4 item 5.*/
	DoC_Command(docptr, NAND_CMD_STATUS, CDSN_CTRL_WP);
	dummy = ReadDOC(docptr, ReadPipeInit);
	DoC_Delay(docptr, 2);
	if (ReadDOC(docptr, Mil_CDSN_IO) & 1) {
		printk("Error programming flash\n");
		/* Error in programming
		   FIXME: implement Bad Block Replacement (in nftl.c ??) */
		*retlen = 0;
		ret = -EIO;
	}
	dummy = ReadDOC(docptr, LastDataRead);

	/* Let the caller know we completed it */
	*retlen = len;

	return ret;
#endif /*#ifdef CONFIG_RTL_819X  */
}

#ifdef CONFIG_RTL_819X 
/*do nothing here*/
#else
static int doc_read_oob(struct mtd_info *mtd, loff_t ofs,
			struct mtd_oob_ops *ops)
{
#ifndef USE_MEMCPY
	int i;
#endif
	volatile char dummy;
	struct DiskOnChip *this = mtd->priv;
	void __iomem *docptr = this->virtadr;
	struct Nand *mychip = &this->chips[ofs >> this->chipshift];
	uint8_t *buf = ops->oobbuf;
	size_t len = ops->len;

	BUG_ON(ops->mode != MTD_OOB_PLACE);

	ofs += ops->ooboffs;

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(docptr, mychip->floor);
		DoC_SelectChip(docptr, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(docptr, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* disable the ECC engine */
	WriteDOC (DOC_ECC_RESET, docptr, ECCConf);
	WriteDOC (DOC_ECC_DIS, docptr, ECCConf);

	/* issue the Read2 command to set the pointer to the Spare Data Area.
	   Polling the Flash Ready bit after issue 3 bytes address in
	   Sequence Read Mode, see Software Requirement 11.4 item 1.*/
	DoC_Command(docptr, NAND_CMD_READOOB, CDSN_CTRL_WP);
	DoC_Address(docptr, 3, ofs, CDSN_CTRL_WP, 0x00);
	DoC_WaitReady(docptr);

	/* Read the data out via the internal pipeline through CDSN IO register,
	   see Pipelined Read Operations 11.3 */
	dummy = ReadDOC(docptr, ReadPipeInit);
#ifndef USE_MEMCPY
	for (i = 0; i < len-1; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		buf[i] = ReadDOC(docptr, Mil_CDSN_IO + i);
	}
#else
	memcpy_fromio(buf, docptr + DoC_Mil_CDSN_IO, len - 1);
#endif
	buf[len - 1] = ReadDOC(docptr, LastDataRead);

	ops->retlen = len;

	return 0;
}

static int doc_write_oob(struct mtd_info *mtd, loff_t ofs,
			 struct mtd_oob_ops *ops)
{
#ifndef USE_MEMCPY
	int i;
#endif
	volatile char dummy;
	int ret = 0;
	struct DiskOnChip *this = mtd->priv;
	void __iomem *docptr = this->virtadr;
	struct Nand *mychip = &this->chips[ofs >> this->chipshift];
	uint8_t *buf = ops->oobbuf;
	size_t len = ops->len;

	BUG_ON(ops->mode != MTD_OOB_PLACE);

	ofs += ops->ooboffs;

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(docptr, mychip->floor);
		DoC_SelectChip(docptr, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(docptr, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	/* disable the ECC engine */
	WriteDOC (DOC_ECC_RESET, docptr, ECCConf);
	WriteDOC (DOC_ECC_DIS, docptr, ECCConf);

	/* Reset the chip, see Software Requirement 11.4 item 1. */
	DoC_Command(docptr, NAND_CMD_RESET, CDSN_CTRL_WP);
	DoC_WaitReady(docptr);
	/* issue the Read2 command to set the pointer to the Spare Data Area. */
	DoC_Command(docptr, NAND_CMD_READOOB, CDSN_CTRL_WP);

	/* issue the Serial Data In command to initial the Page Program process */
	DoC_Command(docptr, NAND_CMD_SEQIN, 0x00);
	DoC_Address(docptr, 3, ofs, 0x00, 0x00);

	/* Write the data via the internal pipeline through CDSN IO register,
	   see Pipelined Write Operations 11.2 */
#ifndef USE_MEMCPY
	for (i = 0; i < len; i++) {
		/* N.B. you have to increase the source address in this way or the
		   ECC logic will not work properly */
		WriteDOC(buf[i], docptr, Mil_CDSN_IO + i);
	}
#else
	memcpy_toio(docptr + DoC_Mil_CDSN_IO, buf, len);
#endif
	WriteDOC(0x00, docptr, WritePipeTerm);

	/* Commit the Page Program command and wait for ready
	   see Software Requirement 11.4 item 1.*/
	DoC_Command(docptr, NAND_CMD_PAGEPROG, 0x00);
	DoC_WaitReady(docptr);

	/* Read the status of the flash device through CDSN IO register
	   see Software Requirement 11.4 item 5.*/
	DoC_Command(docptr, NAND_CMD_STATUS, 0x00);
	dummy = ReadDOC(docptr, ReadPipeInit);
	DoC_Delay(docptr, 2);
	if (ReadDOC(docptr, Mil_CDSN_IO) & 1) {
		printk("Error programming oob data\n");
		/* FIXME: implement Bad Block Replacement (in nftl.c ??) */
		ops->retlen = 0;
		ret = -EIO;
	}
	dummy = ReadDOC(docptr, LastDataRead);

	ops->retlen = len;

	return ret;
}
#endif /*#ifdef CONFIG_RTL_819X  */

#ifdef CONFIG_RTL_819X
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
int find_section_boundary(struct mtd_info *mtd,unsigned int start, unsigned int end, unsigned int *rstart, unsigned *rend)
{
	int i = 0;
	int j = 0;
	struct mtd_erase_region_info *regions = mtd->eraseregions;
	while ((i < mtd->numeraseregions) &&
		(start >= regions[i].offset)) {
		i++;
	}
	i--;

	j = 1;
	while((j <= regions[i].numblocks) &&
		(start >= (regions[i].offset+regions[i].erasesize*j))) {
		j++;
	}
	*rstart=(regions[i].offset+regions[i].erasesize*(j-1));
	
	i=0;
	while ((i < mtd->numeraseregions) &&
		(end >= regions[i].offset)) {
		 i++;
	}
	i--;

	j = 1;
	while((j <= regions[i].numblocks) &&
		(end >= (regions[i].offset+regions[i].erasesize*j))) {
		j++;
	}
	*rend=(regions[i].offset+regions[i].erasesize*j);
	
}
#endif
#endif

int doc_erase (struct mtd_info *mtd, struct erase_info *instr)
{
#ifdef CONFIG_RTL_819X 

struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long adr, len;
#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE	
	unsigned int rstart,rend;
        unsigned int start,end;
#endif
	int i;
	int first;
	struct mtd_erase_region_info *regions = mtd->eraseregions;

	DEBUG(MTD_DEBUG_LEVEL1, "going to erase sector addr=%08x,len=%08x\n",
		instr->addr, instr->len);

	if (instr->addr > mtd->size) {
		printk(KERN_WARNING "Erase addr greater than max size (0x%x > 0x%x)\n",
			instr->addr, mtd->size );
		return -EINVAL;
	}

	if ((instr->len + instr->addr) > mtd->size) {
		printk(KERN_WARNING "Erase size greater than max size (0x%x + 0x%x > 0x%x)\n",
		       instr->addr, instr->len, mtd->size );
		return -EINVAL;
	}

	/* Check that both start and end of the requested erase are
	 * aligned with the erasesize at the appropriate addresses.
	 */

	i = 0;

        /* Skip all erase regions which are ended before the start of
           the requested erase. Actually, to save on the calculations,
           we skip to the first erase region which starts after the
           start of the requested erase, and then go back one.
        */

        while ((i < mtd->numeraseregions) &&
	       (instr->addr >= regions[i].offset)) {
               i++;
	}
        i--;

	/* OK, now i is pointing at the erase region in which this
	 * erase request starts. Check the start of the requested
	 * erase range is aligned with the erase size which is in
	 * effect here.
	 */

	if (instr->addr & (regions[i].erasesize-1)) {
		return -EINVAL;
	}

	/* Remember the erase region we start on. */

	first = i;

	/* Next, check that the end of the requested erase is aligned
	 * with the erase region at that address.
	 */

	while ((i < mtd->numeraseregions) &&
	       ((instr->addr + instr->len) >= regions[i].offset)) {
                i++;
	}

	/* As before, drop back one to point at the region in which
	 * the address actually falls.
	 */

	i--;

	if ((instr->addr + instr->len) & (regions[i].erasesize-1)) {
                return -EINVAL;
	}


	adr = instr->addr;
	len = instr->len;

	i = first;
	instr->state = MTD_ERASING;


#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
	start=adr;
	end=0xFFFFFFF;
	if(flash_write_flag & 1)
	{
		if(0 == start)
			start = CONFIG_RTL_HW_SETTING_OFFSET;
		else if( start > CONFIG_RTL_HW_SETTING_OFFSET )
			start = CONFIG_RTL_HW_SETTING_OFFSET;
		end = CONFIG_RTL_DEFAULT_SETTING_OFFSET;
	}
			
       if(flash_write_flag & 2 )
       {
    		if(0 == start)
			start = CONFIG_RTL_DEFAULT_SETTING_OFFSET;
		else if( start > CONFIG_RTL_DEFAULT_SETTING_OFFSET )
			start = CONFIG_RTL_DEFAULT_SETTING_OFFSET;
		end = CONFIG_RTL_CURRENT_SETTING_OFFSET;
    	}
			
    	if(flash_write_flag & 4 )
   	{
   		if(0 == start)
			start = CONFIG_RTL_CURRENT_SETTING_OFFSET;
		else if( start > CONFIG_RTL_CURRENT_SETTING_OFFSET )
			start = CONFIG_RTL_CURRENT_SETTING_OFFSET;
		
		end = CONFIG_RTL_WEB_PAGES_OFFSET;
   	}

	find_section_boundary(mtd,start,end,&rstart,&rend);

	//printk("line[%d] rstart 0x%x rend 0x%x\n",__LINE__,rstart,rend);
	
	/*don't erase bootcode*/
	if(rstart < RTL_BOOTCODE_END)
		rstart = RTL_BOOTCODE_END;
	
	//printk("line[%d] rstart 0x%x rend 0x%x\n",__LINE__,rstart,rend);
#endif

	while (len) {
//		if (adr >= CONFIG_MTD_DOCPROBE_ADDRESS) {

#if defined(COMPACK_SETTING) || defined(NO_CHECK_REGION)
		if ( erase_one_block(this, adr, regions[i].erasesize) )
			return -1;

#else

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
	   if(adr >= rstart)
#else
		if ( ((flash_write_flag & 1) && (adr == flash_hw_start)) ||
				((flash_write_flag & 2) &&(adr >= flash_ds_start && adr < (flash_ds_start+flash_ds_len))) 
				|| ((flash_write_flag & 4) && (adr >= (flash_ds_start+flash_ds_len))) 
//#ifdef CONFIG_RTK_MTD_ROOT
#ifdef CONFIG_RTL_819X
				|| (adr >= (rtl8196_partitions[0].size+ rtl8196_partitions[0].offset))
#endif			
				|| (flash_write_flag == 0x8000)
				) 
#endif	
		{
			if ( erase_one_block(this, adr, regions[i].erasesize) )
				return -1;
		}
			
#endif // COMPACK_SETTING || NO_CHECK_REGION

		adr += regions[i].erasesize;
		if (len < regions[i].erasesize)
			len = 0;
		else
			len -= regions[i].erasesize;

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
		if(rend <= adr)
		{
			/*no need to erase other block*/
			len=0;
		}
#endif
		if ( adr >= (regions[i].offset + regions[i].erasesize*regions[i].numblocks))
			i++;
	}

	instr->state = MTD_ERASE_DONE;
	if (instr->callback) {
		instr->callback(instr);
	}

	return 0;
#else
	volatile char dummy;
	struct DiskOnChip *this = mtd->priv;
	__u32 ofs = instr->addr;
	__u32 len = instr->len;
	void __iomem *docptr = this->virtadr;
	struct Nand *mychip = &this->chips[ofs >> this->chipshift];

	if (len != mtd->erasesize)
		printk(KERN_WARNING "Erase not right size (%x != %x)n",
		       len, mtd->erasesize);

	/* Find the chip which is to be used and select it */
	if (this->curfloor != mychip->floor) {
		DoC_SelectFloor(docptr, mychip->floor);
		DoC_SelectChip(docptr, mychip->chip);
	} else if (this->curchip != mychip->chip) {
		DoC_SelectChip(docptr, mychip->chip);
	}
	this->curfloor = mychip->floor;
	this->curchip = mychip->chip;

	instr->state = MTD_ERASE_PENDING;

	/* issue the Erase Setup command */
	DoC_Command(docptr, NAND_CMD_ERASE1, 0x00);
	DoC_Address(docptr, 2, ofs, 0x00, 0x00);

	/* Commit the Erase Start command and wait for ready
	   see Software Requirement 11.4 item 1.*/
	DoC_Command(docptr, NAND_CMD_ERASE2, 0x00);
	DoC_WaitReady(docptr);

	instr->state = MTD_ERASING;

	/* Read the status of the flash device through CDSN IO register
	   see Software Requirement 11.4 item 5.
	   FIXME: it seems that we are not wait long enough, some blocks are not
	   erased fully */
	DoC_Command(docptr, NAND_CMD_STATUS, CDSN_CTRL_WP);
	dummy = ReadDOC(docptr, ReadPipeInit);
	DoC_Delay(docptr, 2);
	if (ReadDOC(docptr, Mil_CDSN_IO) & 1) {
		printk("Error Erasing at 0x%x\n", ofs);
		/* There was an error
		   FIXME: implement Bad Block Replacement (in nftl.c ??) */
		instr->state = MTD_ERASE_FAILED;
	} else
		instr->state = MTD_ERASE_DONE;
	dummy = ReadDOC(docptr, LastDataRead);

	mtd_erase_callback(instr);

	return 0;

#endif /*#ifdef CONFIG_RTL_819X */
}

#ifdef CONFIG_RTL_819X 
static int erase_one_block(struct DiskOnChip *this, __u32 addr, __u32 len)
{
	unsigned long timeo;
	unsigned long docptr = this->virtadr;
	__u32 ofs, offset;
	unsigned long flags; // david


	DEBUG(MTD_DEBUG_LEVEL1, "Erase sector, addr=0x%x, docptr=0x%x, len=0x%x\n",
		(int)addr, (int)docptr, (int)len);

	// issue erase command!
	ofs = docptr + addr;

	offset = (addr >> this->chipshift)*(1 << this->chipshift);

	mtd_save_flags(flags);mtd_cli(); // david
	*(volatile unsigned short *)(docptr + offset + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(docptr + offset + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(docptr + offset + 0x555 * 2) = 0x80;
	*(volatile unsigned short *)(docptr + offset + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(docptr + offset + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(ofs ) = 0x30;
	mtd_restore_flags(flags); // david

	timeo = jiffies + (HZ * 40);

	while (1) {
     		if ((*(volatile unsigned short *)(ofs))==0xffff) {
			DEBUG(MTD_DEBUG_LEVEL1, "Erase success!\n");
			break;
		}
		if (time_after(jiffies, timeo)) {
			printk(KERN_WARNING "Erase timeout!\n");
			return -1;
		}		
		udelay(1);
     	}
	return 0;
}
#endif /*#ifdef CONFIG_RTL_819X */

/****************************************************************************
 *
 * Module stuff
 *
 ****************************************************************************/

static void __exit cleanup_doc2001(void)
{
	struct mtd_info *mtd;
	struct DiskOnChip *this;

	while ((mtd=docmillist)) {
		this = mtd->priv;
		docmillist = this->nextdoc;

		del_mtd_device(mtd);

		iounmap(this->virtadr);
		kfree(this->chips);
		kfree(mtd);
	}
}

module_exit(cleanup_doc2001);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org> et al.");
MODULE_DESCRIPTION("Alternative driver for DiskOnChip Millennium");
