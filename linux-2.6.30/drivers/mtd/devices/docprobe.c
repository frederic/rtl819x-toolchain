
/* Linux driver for Disk-On-Chip devices			*/
/* Probe routines common to all DoC devices			*/
/* (C) 1999 Machine Vision Holdings, Inc.			*/
/* (C) 1999-2003 David Woodhouse <dwmw2@infradead.org>		*/


/* DOC_PASSIVE_PROBE:
   In order to ensure that the BIOS checksum is correct at boot time, and
   hence that the onboard BIOS extension gets executed, the DiskOnChip
   goes into reset mode when it is read sequentially: all registers
   return 0xff until the chip is woken up again by writing to the
   DOCControl register.

   Unfortunately, this means that the probe for the DiskOnChip is unsafe,
   because one of the first things it does is write to where it thinks
   the DOCControl register should be - which may well be shared memory
   for another device. I've had machines which lock up when this is
   attempted. Hence the possibility to do a passive probe, which will fail
   to detect a chip in reset mode, but is at least guaranteed not to lock
   the machine.

   If you have this problem, uncomment the following line:
#define DOC_PASSIVE_PROBE
*/


/* DOC_SINGLE_DRIVER:
   Millennium driver has been merged into DOC2000 driver.

   The old Millennium-only driver has been retained just in case there
   are problems with the new code. If the combined driver doesn't work
   for you, you can try the old one by undefining DOC_SINGLE_DRIVER
   below and also enabling it in your configuration. If this fixes the
   problems, please send a report to the MTD mailing list at
   <linux-mtd@lists.infradead.org>.
*/
#define DOC_SINGLE_DRIVER
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/types.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/doc2000.h>
#include <linux/mtd/compatmac.h>

/* Where to look for the devices? */
#ifndef CONFIG_MTD_DOCPROBE_ADDRESS
#define CONFIG_MTD_DOCPROBE_ADDRESS 0
#endif

#ifdef CONFIG_RTL_819X
// david ----------------------
/* MXIC */
#define MANUFACTURER_MXIC	0x00C2
#define MX29LV800B		0x225B
#define MX29LV160AB		0x2249
#define MX29LV320AB		0x22A8
#define MX29LV640AB		0x22CB

/*AMD*/
#define MANUFACTURER_AMD	0x0001
#define AM29LV800BB		0x225B
#define AM29LV160DB		0x2249
#define AM29LV320DB		0x22F9

/*ST*/
#define MANUFACTURER_ST		0x0020
#define M29W160DB		0X2249

/* ESMT */
#define MANUFACTURER_ESMT	0x008C
#define F49L160BA		0x2249

/* SAMSUNG */
#define MANUFACTURER_SAMSUNG	0x00EC
#define K8D1716UBC		0x2277

/* ESI */
#define MANUFACTURER_ESI	0x004A
#define ES29LV320D		0x22F9

/* EON */
#define MANUFACTURER_EON	0x007F
#define EN29LV160A		0x2249

#ifdef CONFIG_RTL8196B
#define FLASH_BASE 0xbd000000
#else
#define FLASH_BASE 0xbe000000
#endif

struct flash_info {
	const __u16 mfr_id;
	const __u16 dev_id;
	const char *name;
	const u_long size;
	const int shift;  // shift number of chip size
	const int numeraseregions;
	const struct mtd_erase_region_info regions[4];
};

static int probeChip(struct DiskOnChip *doc, struct mtd_info *mtd);
//-----------------------------
#endif /*#ifdef CONFIG_RTL_819X*/

//static unsigned long doc_config_location = CONFIG_MTD_DOCPROBE_ADDRESS;
//module_param(doc_config_location, ulong, 0);
//MODULE_PARM_DESC(doc_config_location, "Physical memory address at which to probe for DiskOnChip");

#ifdef CONFIG_RTL_819X
 /* Do nothing here*/
#else

static unsigned long __initdata doc_locations[] = {
#if defined (__alpha__) || defined(__i386__) || defined(__x86_64__)
#ifdef CONFIG_MTD_DOCPROBE_HIGH
	0xfffc8000, 0xfffca000, 0xfffcc000, 0xfffce000,
	0xfffd0000, 0xfffd2000, 0xfffd4000, 0xfffd6000,
	0xfffd8000, 0xfffda000, 0xfffdc000, 0xfffde000,
	0xfffe0000, 0xfffe2000, 0xfffe4000, 0xfffe6000,
	0xfffe8000, 0xfffea000, 0xfffec000, 0xfffee000,
#else /*  CONFIG_MTD_DOCPROBE_HIGH */
	0xc8000, 0xca000, 0xcc000, 0xce000,
	0xd0000, 0xd2000, 0xd4000, 0xd6000,
	0xd8000, 0xda000, 0xdc000, 0xde000,
	0xe0000, 0xe2000, 0xe4000, 0xe6000,
	0xe8000, 0xea000, 0xec000, 0xee000,
#endif /*  CONFIG_MTD_DOCPROBE_HIGH */
#elif defined(__PPC__)
	0xe4000000,
#elif defined(CONFIG_MOMENCO_OCELOT)
	0x2f000000,
        0xff000000,
#elif defined(CONFIG_MOMENCO_OCELOT_G) || defined (CONFIG_MOMENCO_OCELOT_C)
        0xff000000,
#else
#warning Unknown architecture for DiskOnChip. No default probe locations defined
#endif
	0xffffffff };

/* doccheck: Probe a given memory window to see if there's a DiskOnChip present */

static inline int __init doccheck(void __iomem *potential, unsigned long physadr)
{
	void __iomem *window=potential;
	unsigned char tmp, tmpb, tmpc, ChipID;
#ifndef DOC_PASSIVE_PROBE
	unsigned char tmp2;
#endif

	/* Routine copied from the Linux DOC driver */

#ifdef CONFIG_MTD_DOCPROBE_55AA
	/* Check for 0x55 0xAA signature at beginning of window,
	   this is no longer true once we remove the IPL (for Millennium */
	if (ReadDOC(window, Sig1) != 0x55 || ReadDOC(window, Sig2) != 0xaa)
		return 0;
#endif /* CONFIG_MTD_DOCPROBE_55AA */

#ifndef DOC_PASSIVE_PROBE
	/* It's not possible to cleanly detect the DiskOnChip - the
	 * bootup procedure will put the device into reset mode, and
	 * it's not possible to talk to it without actually writing
	 * to the DOCControl register. So we store the current contents
	 * of the DOCControl register's location, in case we later decide
	 * that it's not a DiskOnChip, and want to put it back how we
	 * found it.
	 */
	tmp2 = ReadDOC(window, DOCControl);

	/* Reset the DiskOnChip ASIC */
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_RESET,
		 window, DOCControl);
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_RESET,
		 window, DOCControl);

	/* Enable the DiskOnChip ASIC */
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_NORMAL,
		 window, DOCControl);
	WriteDOC(DOC_MODE_CLR_ERR | DOC_MODE_MDWREN | DOC_MODE_NORMAL,
		 window, DOCControl);
#endif /* !DOC_PASSIVE_PROBE */

	/* We need to read the ChipID register four times. For some
	   newer DiskOnChip 2000 units, the first three reads will
	   return the DiskOnChip Millennium ident. Don't ask. */
	ChipID = ReadDOC(window, ChipID);

	switch (ChipID) {
	case DOC_ChipID_Doc2k:
		/* Check the TOGGLE bit in the ECC register */
		tmp  = ReadDOC(window, 2k_ECCStatus) & DOC_TOGGLE_BIT;
		tmpb = ReadDOC(window, 2k_ECCStatus) & DOC_TOGGLE_BIT;
		tmpc = ReadDOC(window, 2k_ECCStatus) & DOC_TOGGLE_BIT;
		if (tmp != tmpb && tmp == tmpc)
				return ChipID;
		break;

	case DOC_ChipID_DocMil:
		/* Check for the new 2000 with Millennium ASIC */
		ReadDOC(window, ChipID);
		ReadDOC(window, ChipID);
		if (ReadDOC(window, ChipID) != DOC_ChipID_DocMil)
			ChipID = DOC_ChipID_Doc2kTSOP;

		/* Check the TOGGLE bit in the ECC register */
		tmp  = ReadDOC(window, ECCConf) & DOC_TOGGLE_BIT;
		tmpb = ReadDOC(window, ECCConf) & DOC_TOGGLE_BIT;
		tmpc = ReadDOC(window, ECCConf) & DOC_TOGGLE_BIT;
		if (tmp != tmpb && tmp == tmpc)
				return ChipID;
		break;

	case DOC_ChipID_DocMilPlus16:
	case DOC_ChipID_DocMilPlus32:
	case 0:
		/* Possible Millennium+, need to do more checks */
#ifndef DOC_PASSIVE_PROBE
		/* Possibly release from power down mode */
		for (tmp = 0; (tmp < 4); tmp++)
			ReadDOC(window, Mplus_Power);

		/* Reset the DiskOnChip ASIC */
		tmp = DOC_MODE_RESET | DOC_MODE_MDWREN | DOC_MODE_RST_LAT |
			DOC_MODE_BDECT;
		WriteDOC(tmp, window, Mplus_DOCControl);
		WriteDOC(~tmp, window, Mplus_CtrlConfirm);

		mdelay(1);
		/* Enable the DiskOnChip ASIC */
		tmp = DOC_MODE_NORMAL | DOC_MODE_MDWREN | DOC_MODE_RST_LAT |
			DOC_MODE_BDECT;
		WriteDOC(tmp, window, Mplus_DOCControl);
		WriteDOC(~tmp, window, Mplus_CtrlConfirm);
		mdelay(1);
#endif /* !DOC_PASSIVE_PROBE */

		ChipID = ReadDOC(window, ChipID);

		switch (ChipID) {
		case DOC_ChipID_DocMilPlus16:
		case DOC_ChipID_DocMilPlus32:
			/* Check the TOGGLE bit in the toggle register */
			tmp  = ReadDOC(window, Mplus_Toggle) & DOC_TOGGLE_BIT;
			tmpb = ReadDOC(window, Mplus_Toggle) & DOC_TOGGLE_BIT;
			tmpc = ReadDOC(window, Mplus_Toggle) & DOC_TOGGLE_BIT;
			if (tmp != tmpb && tmp == tmpc)
					return ChipID;
		default:
			break;
		}
		/* FALL TRHU */

	default:

#ifdef CONFIG_MTD_DOCPROBE_55AA
		printk(KERN_DEBUG "Possible DiskOnChip with unknown ChipID %2.2X found at 0x%lx\n",
		       ChipID, physadr);
#endif
#ifndef DOC_PASSIVE_PROBE
		/* Put back the contents of the DOCControl register, in case it's not
		 * actually a DiskOnChip.
		 */
		WriteDOC(tmp2, window, DOCControl);
#endif
		return 0;
	}

	printk(KERN_WARNING "DiskOnChip failed TOGGLE test, dropping.\n");

#ifndef DOC_PASSIVE_PROBE
	/* Put back the contents of the DOCControl register: it's not a DiskOnChip */
	WriteDOC(tmp2, window, DOCControl);
#endif
	return 0;
}
#endif /*#ifdef CONFIG_RTL_819X*/

//static int docfound;

extern void DoC2k_init(struct mtd_info *);
extern void DoCMil_init(struct mtd_info *);
extern void DoCMilPlus_init(struct mtd_info *);

static void __init DoC_Probe(unsigned long physadr)
{
	void __iomem *docptr;
	struct DiskOnChip *this;
	struct mtd_info *mtd;
	//int ChipID;
	char namebuf[15];
	char *name = namebuf;

	
	char *im_funcname = NULL;
	char *im_modname = NULL;
	
	
	void (*initroutine)(struct mtd_info *) = NULL;

#ifndef CONFIG_RTL_819X
	docptr = ioremap(physadr, DOC_IOREMAP_LEN);

	if (!docptr)
		return;

	if ((ChipID = doccheck(docptr, physadr))) {
		if (ChipID == DOC_ChipID_Doc2kTSOP) {
			/* Remove this at your own peril. The hardware driver works but nothing prevents you from erasing bad blocks */
			printk(KERN_NOTICE "Refusing to drive DiskOnChip 2000 TSOP until Bad Block Table is correctly supported by INFTL\n");
			iounmap(docptr);
			return;
		}
		docfound = 1;
		mtd = kmalloc(sizeof(struct DiskOnChip) + sizeof(struct mtd_info), GFP_KERNEL);

		if (!mtd) {
			printk(KERN_WARNING "Cannot allocate memory for data structures. Dropping.\n");
			iounmap(docptr);
			return;
		}

		this = (struct DiskOnChip *)(&mtd[1]);

		memset((char *)mtd,0, sizeof(struct mtd_info));
		memset((char *)this, 0, sizeof(struct DiskOnChip));

		mtd->priv = this;
		this->virtadr = docptr;
		this->physadr = physadr;
		this->ChipID = ChipID;
		sprintf(namebuf, "with ChipID %2.2X", ChipID);

		switch(ChipID) {
		case DOC_ChipID_Doc2kTSOP:
			name="2000 TSOP";
			initroutine = symbol_request(DoC2k_init);
			break;

		case DOC_ChipID_Doc2k:
			name="2000";
			initroutine = symbol_request(DoC2k_init);
			break;

		case DOC_ChipID_DocMil:
			name="Millennium";
#ifdef DOC_SINGLE_DRIVER
			initroutine = symbol_request(DoC2k_init);
#else
			initroutine = symbol_request(DoCMil_init);
#endif /* DOC_SINGLE_DRIVER */
			break;

		case DOC_ChipID_DocMilPlus16:
		case DOC_ChipID_DocMilPlus32:
			name="MillenniumPlus";
			initroutine = symbol_request(DoCMilPlus_init);
			break;
		}

		if (initroutine) {
			(*initroutine)(mtd);
			symbol_put_addr(initroutine);
			return;
		}
		printk(KERN_NOTICE "Cannot find driver for DiskOnChip %s at 0x%lX\n", name, physadr);
		kfree(mtd);
	}
	iounmap(docptr);
#else
      docptr = FLASH_BASE;
      //-----------------------------


	mtd = kmalloc(sizeof(struct DiskOnChip) + sizeof(struct mtd_info), GFP_KERNEL);

	if (!mtd) {
			printk(KERN_WARNING "Cannot allocate memory for data structures. Dropping.\n");
			iounmap((void *)docptr);
			return;
}

	this = (struct DiskOnChip *)(&mtd[1]);

	memset((char *)mtd,0, sizeof(struct mtd_info));
	memset((char *)this, 0, sizeof(struct DiskOnChip));

	mtd->priv = this;
	this->virtadr = docptr;
	this->physadr = physadr;
	this->ChipID = DOC_ChipID_DocMil;

	name="Millennium";
	im_funcname = "DoCMil_init";
	im_modname = "doc2001";

	if ( probeChip(this, mtd) == 0) // david added,
		initroutine = &DoCMil_init;

	if (initroutine) {
		(*initroutine)(mtd);
		return;
	}
	printk(KERN_NOTICE "Cannot find driver for DiskOnChip %s at 0x%lX\n", name, physadr);
	iounmap((void *)docptr);

#endif /*#ifdef CONFIG_RTL_819X*/
}

#ifdef CONFIG_RTL_819X
// david -------------------------------------------------------------------
static int probeChip(struct DiskOnChip *doc, struct mtd_info *mtd)
{
	/* Keep this table on the stack so that it gets deallocated after the
	 * probe is done.
	 */
	const struct flash_info table[] = {
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV800B,
		name: "MXIC MX29LV800B",
		size: 0x00100000,
		shift: 20,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
		}
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV160AB,
		name: "MXIC MX29LV160AB",
		size: 0x00200000,
		shift: 21,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV320AB,
		name: "MXIC MX29LV320AB",
		size: 0x00400000,
		shift: 22,
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 63 }
		}
	},
	{
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV800BB,
		name: "AMD AM29LV800BB",
		size: 0x00100000,
		shift: 20,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
		}
	},
	{
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV160DB,
		name: "AMD AM29LV160DB",
		size: 0x00200000,
		shift: 21,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV320DB,
		name: "AMD AM29LV320DB",
		size: 0x00400000,
		shift: 22,
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 63 }
		}
	},
	{
		mfr_id: MANUFACTURER_ST,
		dev_id: M29W160DB,
		name: "ST M29W160DB",
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV640AB,
		name: "MXIC MX29LV640AB",
		size: 0x00800000,
		shift: 23,/*22 bit=> that is 8 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:   8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 127 }
		}
	},
	{
		mfr_id: MANUFACTURER_SAMSUNG,
		dev_id: K8D1716UBC,
		name: "SAMSUNG K8D1716UBC",
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_ESMT,
		dev_id: F49L160BA,
		name: "ESMT F49L160BA",
		size: 0x00200000,
		shift: 21,/*21 bit=> that is 2 MByte size*/
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	},
	{
		mfr_id: MANUFACTURER_ESI,
		dev_id: ES29LV320D,
		name: "ESI ES29LV320D",
		size: 0x00400000,
		shift: 22,/*22 bit=> that is 4 MByte size*/
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 63 }
		}
	},
	{
		mfr_id: MANUFACTURER_EON,
		dev_id: EN29LV160A,
		name: "EON EN29LV160A",
		size: 0x00200000,
		shift: 21,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	}
	};

	struct DiskOnChip *this = (struct DiskOnChip *)mtd->priv;
	unsigned long docptr = this->virtadr;
	__u16 mfid, devid;
	int i, j, k, interleave=1, chipsize;

	// issue reset and auto-selection command
	*(volatile unsigned short *)(FLASH_BASE) = 0xf0;

	*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(FLASH_BASE + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(FLASH_BASE + 0x555 * 2) = 0x90;

	mfid = *((volatile unsigned short *)docptr);
	devid = *((volatile unsigned short *)(docptr + 1*2));

        *(volatile unsigned short *)(FLASH_BASE) = 0xf0;

	for (i=0; i< sizeof(table)/sizeof(table[0]); i++) {
		if ( mfid==table[i].mfr_id && devid==table[i].dev_id)
			break;
	}
	if ( i == sizeof(table)/sizeof(table[0]) )
		return -1;

	// Look for 2nd flash
	*(volatile unsigned short *)(FLASH_BASE + table[i].size) = 0xf0;
	*(volatile unsigned short *)(FLASH_BASE + table[i].size + 0x555 * 2) = 0xaa;
	*(volatile unsigned short *)(FLASH_BASE + table[i].size + 0x2aa * 2) = 0x55;
	*(volatile unsigned short *)(FLASH_BASE + table[i].size + 0x555 * 2) = 0x90;

	mfid = *((volatile unsigned short *)(docptr + table[i].size));
	devid = *((volatile unsigned short *)(docptr + table[i].size + 1*2));

	*(volatile unsigned short *)(FLASH_BASE+table[i].size) = 0xf0;
	if ( mfid==table[i].mfr_id && devid==table[i].dev_id) {
		interleave++;
	}

	printk(KERN_NOTICE "Found %d x %ldM Byte %s at 0x%lx\n",
		interleave, (table[i].size)/(1024*1024), table[i].name, docptr);

	mtd->size = table[i].size*interleave;
	mtd->numeraseregions = table[i].numeraseregions*interleave;

	mtd->eraseregions = kmalloc(sizeof(struct mtd_erase_region_info) *
				    mtd->numeraseregions*interleave, GFP_KERNEL);
	if (!mtd->eraseregions) {
		printk(KERN_WARNING "Failed to allocate "
		       "memory for MTD erase region info\n");
		kfree(mtd);
		return -1;
	}

	for (k=0, chipsize=0; interleave>0; interleave--, chipsize+=table[i].size) {
		for (j=0; j<table[i].numeraseregions; j++, k++) {
			mtd->eraseregions[k].offset = table[i].regions[j].offset+chipsize;
			mtd->eraseregions[k].erasesize = table[i].regions[j].erasesize;
			mtd->eraseregions[k].numblocks = table[i].regions[j].numblocks;
			if (mtd->erasesize < mtd->eraseregions[k].erasesize)
				mtd->erasesize = mtd->eraseregions[k].erasesize;
		}
	}

	this->totlen = mtd->size;
	this->numchips = interleave;
	this->chipshift = table[i].shift;

	return 0;
}
//---------------------------------------------------------------------------
#endif /*#ifdef CONFIG_RTL_819X */


/****************************************************************************
 *
 * Module stuff
 *
 ****************************************************************************/

static int __init init_doc(void)
{
#ifdef CONFIG_RTL_819X
      printk(KERN_NOTICE "RealTek E-Flash System Driver. (C) 2002 RealTek Corp.\n");
	DoC_Probe(CONFIG_MTD_DOCPROBE_ADDRESS);
	/* So it looks like we've been used and we get unloaded */
//	MOD_INC_USE_COUNT;
//	MOD_DEC_USE_COUNT;
	return 0;
#else
	int i;

	if (doc_config_location) {
		printk(KERN_INFO "Using configured DiskOnChip probe address 0x%lx\n", doc_config_location);
		DoC_Probe(doc_config_location);
	} else {
		for (i=0; (doc_locations[i] != 0xffffffff); i++) {
			DoC_Probe(doc_locations[i]);
		}
	}
	/* No banner message any more. Print a message if no DiskOnChip
	   found, so the user knows we at least tried. */
	if (!docfound)
		printk(KERN_INFO "No recognised DiskOnChip devices found\n");
	return -EAGAIN;
#endif	
}

module_init(init_doc);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org>");
MODULE_DESCRIPTION("Probe code for DiskOnChip 2000 and Millennium devices");

