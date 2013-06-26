
/*
 * Flash mapping for rtl8196 board
 *
 * Copyright (C) 2008 Realtek Corporation
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/root_dev.h>
#include <linux/mtd/partitions.h>
#include <linux/config.h>
#include <linux/delay.h>
#include <linux/autoconf.h>

#ifdef CONFIG_MTD_CONCAT
#include <linux/mtd/concat.h>
#endif

#define WINDOW_ADDR 0xbfe00000
#ifdef CONFIG_SPANSION_16M_FLASH	
#define WINDOW_SIZE 0x1000000
#define FLASH_BANK_SIZE 0x400000
#else 
#define WINDOW_SIZE 0x400000
#endif 
#define BUSWIDTH 2

#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#define MAX_SPI_CS 2		/* Number of CS we are going to test */
#else
#define MAX_SPI_CS 1
#endif



static struct mtd_info *rtl8196_mtd;

__u8 rtl8196_map_read8(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readb(map->map_priv_1 + ofs);
}

__u16 rtl8196_map_read16(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readw(map->map_priv_1 + ofs);
}

__u32 rtl8196_map_read32(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readl(map->map_priv_1 + ofs);
}

void rtl8196_map_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	//printk("enter to %x from  %x len %d\n",to, map->map_priv_1+from , len);
	//11/15/05' hrchen, change the size to fit file systems block size if use different fs
	//4096 for cramfs, 1024 for squashfs
	if (from>0x10000)
	    memcpy(to, map->map_priv_1 + from, (len<=1024)?len:1024);//len);
	else
	    memcpy(to, map->map_priv_1 + from, (len<=4096)?len:4096);//len);
	//printk("enter %s %d\n", __FILE__,__LINE__);

}

void rtl8196_map_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	__raw_writeb(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	__raw_writew(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	__raw_writel(d, map->map_priv_1 + adr);
	mb();
}

void rtl8196_map_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	memcpy_toio(map->map_priv_1 + to, from, len);
}

static struct map_info spi_map[MAX_SPI_CS] = {
        {
			name: 		"flash_bank_1",
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
			size: 		CONFIG_RTL_SPI_FLASH1_SIZE,
#else
			size:			WINDOW_SIZE,
#endif
			phys: 		0xbd000000,
			virt: 			0xbd000000,
			bankwidth: 	BUSWIDTH

        },
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
        {
			name: 		"flash_bank_2",
			size: 		CONFIG_RTL_SPI_FLASH2_SIZE,
			phys: 		0xbe000000,
			virt: 			0xbe000000,
			bankwidth: 	BUSWIDTH
        }
#endif
};

static struct mtd_info *my_sub_mtd[MAX_SPI_CS] = {
	NULL,
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
	NULL
#endif
};

static struct mtd_info *mymtd;



/////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
#if defined( CONFIG_ROOTFS_JFFS2 )
static struct mtd_partition rtl8196_parts1[] = {
        {
                name:           "boot+cfg",
                size:           (CONFIG_RTL_LINUX_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "jffs2(linux+root fs)",                
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE+CONFIG_RTL_SPI_FLASH2_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#else
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
#else
                size:        (WINDOW_SIZE - CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
                offset:      (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }
};
#elif defined( CONFIG_ROOTFS_RAMFS )
static struct mtd_partition rtl8196_parts1[] = {
        {
                name:        "boot+cfg+linux+rootfs",
                size:        (CONFIG_RTL_FLASH_SIZE-0),
                offset:      0x00000000,
        },
};

#elif defined( CONFIG_ROOTFS_SQUASH )
#ifndef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
static struct mtd_partition rtl8196_parts1[] = {
        {
                name: "boot+cfg+linux",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "root fs",  
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
#ifdef CONFIG_MTD_CONCAT
                size:        (CONFIG_RTL_SPI_FLASH1_SIZE+CONFIG_RTL_SPI_FLASH2_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#else
		  size:        (CONFIG_RTL_SPI_FLASH1_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
#else
                size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
#endif
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }
};

#else //!CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
static struct mtd_partition rtl8196_parts1[] = {
        {
                name: "boot+cfg+linux(bank1)",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         0x00000000,
        },
        {
                name:           "root fs(bank1)",                
                size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
                offset:         (CONFIG_RTL_ROOT_IMAGE_OFFSET),
        },
        {
                name: "inux(bank2)",
                size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0),
                offset:         CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET,
        },
        {
                name:           "root fs(bank2)",                
                size:        (CONFIG_RTL_FLASH_SIZE-CONFIG_RTL_ROOT_IMAGE_OFFSET),
                offset:         CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET+(CONFIG_RTL_ROOT_IMAGE_OFFSET),
        }

};
#endif //CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE

#else
#error "unknow flash filesystem type"
#endif

#else // !CONFIG_RTL_FLASH_MAPPING_ENABLE
static struct mtd_partition rtl8196_parts1[] = {
        {
                name: "boot+cfg+linux",
                size:  0x00130000,
                offset:0x00000000,
        },
        {
                name:           "root fs",                
		   		size:        	0x002D0000,
                offset:         0x00130000,
        }
};
#endif

#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE
static struct mtd_partition rtl8196_parts2[] = {
        {
                name: 		"data",
                size:  		CONFIG_RTL_SPI_FLASH2_SIZE,
                offset:		0x00000000,
        }
};
#endif

#if LINUX_VERSION_CODE < 0x20212 && defined(MODULE)
#define init_rtl8196_map init_module
#define cleanup_rtl8196_map cleanup_module
#endif

#define mod_init_t  static int __init
#define mod_exit_t  static void __exit

mod_init_t init_rtl8196_map(void)
{
        int i,chips;
#ifdef SIZE_REMAINING
	struct mtd_partition *last_partition;
#endif
   	chips = 0;
	for (i=0;i<MAX_SPI_CS;i++) {
		simple_map_init(&spi_map[i]);
		my_sub_mtd[i] = do_map_probe(spi_map[i].name, &spi_map[i]);

		if (my_sub_mtd[i]) {
			my_sub_mtd[i]->owner = THIS_MODULE;
			chips++;
			//printk("%s, %d, i=%d, chips=%d\n", __FUNCTION__, __LINE__, i, chips);
		}
	}
	
	#ifdef CONFIG_MTD_CONCAT
	if (chips == 1) 
		mymtd = my_sub_mtd[0];
	else 		
		{
			//printk("%s, %d\n, size=0x%x\n", __FUNCTION__, __LINE__, my_sub_mtd[0]->size);
			mymtd = mtd_concat_create(&my_sub_mtd[0], chips,"flash_concat");
			//printk("%s, %d, size=0x%x\n", __FUNCTION__, __LINE__, (mymtd->size));
		}
	
	if (!mymtd) {
		printk("Cannot create flash concat device\n");
		return -ENXIO;
	}
	#endif
	
#ifdef SIZE_REMAINING
#ifdef CONFIG_MTD_CONCAT
		last_partition = &rtl8196_parts1[ ARRAY_SIZE(rtl8196_parts1) - 1 ];
		if( last_partition->size == SIZE_REMAINING ) {
			if( last_partition->offset > mymtd->size ) {
				printk( "Warning: partition offset larger than mtd size\n" );
			}else{
				last_partition->size = mymtd->size - last_partition->offset;
			}
#ifdef DEBUG_MAP
			printk(KERN_NOTICE "last_partition size: 0x%x\n",last_partition->size );
#endif
		}
#else
		//for (i=0;i<chips;i++) 
		{
				last_partition = &rtl8196_parts1[ ARRAY_SIZE(rtl8196_parts1) - 1 ];
				if( last_partition->size == SIZE_REMAINING ) {
					if( last_partition->offset > my_sub_mtd[0]->size ) {
						printk( "Warning: partition offset larger than mtd size\n" );
					}else{
						last_partition->size = my_sub_mtd[0]->size - last_partition->offset;
					}
#ifdef DEBUG_MAP
					printk(KERN_NOTICE "last_partition size: 0x%x\n",last_partition->size );
#endif
				}
				last_partition = &rtl8196_parts2[ ARRAY_SIZE(rtl8196_parts2) - 1 ];
				if( last_partition->size == SIZE_REMAINING ) {
					if( last_partition->offset > my_sub_mtd[1]->size ) {
						printk( "Warning: partition offset larger than mtd size\n" );
					}else{
						last_partition->size = my_sub_mtd[1]->size - last_partition->offset;
					}
		#ifdef DEBUG_MAP
					printk(KERN_NOTICE "last_partition size: 0x%x\n",last_partition->size );
		#endif
				}
		}
#endif
#endif

	
	#ifdef CONFIG_MTD_CONCAT
	add_mtd_partitions(mymtd, rtl8196_parts1, ARRAY_SIZE(rtl8196_parts1));
	#ifdef DEBUG_MAP
    	printk(KERN_NOTICE "name=%s, size=0x%x\n", mymtd->name, mymtd->size);
	#endif
	#else
	if (my_sub_mtd[0]) {
		add_mtd_partitions(my_sub_mtd[0], rtl8196_parts1, 
					ARRAY_SIZE(rtl8196_parts1));
	#ifdef DEBUG_MAP
    	printk(KERN_NOTICE "name=%s, size=0x%x\n", my_sub_mtd[0]->name,
					 my_sub_mtd[0]->size);
	#endif
	}
#ifdef CONFIG_RTL_TWO_SPI_FLASH_ENABLE	
	if (my_sub_mtd[1]) {
		add_mtd_partitions(my_sub_mtd[1], rtl8196_parts2, 
					ARRAY_SIZE(rtl8196_parts2));
	#ifdef DEBUG_MAP
    	printk(KERN_NOTICE "name=%s, size=0x%x\n", my_sub_mtd[1]->name,
					 my_sub_mtd[1]->size);
	#endif
	}
#endif
	#endif

	ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, 0);
	return 0;
}

mod_exit_t cleanup_rtl8196_map(void)
{
	int i;

	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
	}

	for(i=0; i<MAX_SPI_CS; i++)
	{
		if (my_sub_mtd[i])
		{
			del_mtd_partitions(my_sub_mtd[i]);
			map_destroy(my_sub_mtd[i]);
		}
	}
}

MODULE_LICENSE("GPL");
module_init(init_rtl8196_map);
module_exit(cleanup_rtl8196_map);

