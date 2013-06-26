#include <linux/config.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/tpqic02.h>
#include <linux/ftape.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/pgalloc.h>

#include "lcm_mmap.h"

#define LCM_MMAP_MAJOR	242
#define LCM_MMAP_NAME	"lcm"
#define LCM_MMAP_SIZE	( PAGE_SIZE )	// page size = 4096 

/* specifiy 'bss' section to eliminate fragment */
__attribute__ ((aligned (4096),section("bss"))) unsigned char lcm_mmap[ LCM_MMAP_SIZE ];

static int mmap_lcm(struct file * file, struct vm_area_struct * vma)
{
	unsigned long physical = __pa( ( unsigned long )( void * )lcm_mmap );

	vma->vm_pgoff = physical >> PAGE_SHIFT;
	
	vma->vm_flags |= VM_RESERVED;
	
	printk( "remap(concise)\n" );

#if 1
	printk( "lcm_memroy:%08X\n", *( ( unsigned long * )lcm_mmap ) );
	
	printk( "start:%p,end:%p,physical:%p,prot:%X\n", vma->vm_start, vma->vm_end, physical, vma->vm_page_prot );
#endif
	
	if (remap_page_range(vma->vm_start, physical, vma->vm_end-vma->vm_start,
                 vma->vm_page_prot))
	{
		return -EAGAIN;
	}	
	
	return 0;
}

#if 0
static int open_lcm(struct inode * inode, struct file * filp)
{
	printk( "open_lcm\n" );
	return 0;
}
#endif

static struct file_operations lcm_fops = {
	//open: open_lcm,
    mmap: mmap_lcm,
};

int __init lcm_dev_init(void)
{
	int ret;

	ret = register_chrdev( LCM_MMAP_MAJOR, LCM_MMAP_NAME, &lcm_fops );

	printk( "lcm_dev_init:%d\n", ret );

	return 0;
}

__initcall(lcm_dev_init);

