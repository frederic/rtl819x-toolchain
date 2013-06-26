/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 - 1999, 2000, 03 Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_PAGE_H
#define _ASM_PAGE_H

#include <spaces.h>

/*
 * PAGE_SHIFT determines the page size
 */
#define PAGE_SHIFT	12
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK   (~((1 << PAGE_SHIFT) - 1))

#ifndef __ASSEMBLY__

#include <linux/pfn.h>
#include <asm/io.h>

extern void build_clear_page(void);
extern void build_copy_page(void);

/*
 * It's normally defined only for FLATMEM config but it's
 * used in our early mem init code for all memory models.
 * So always define it.
 */
#define ARCH_PFN_OFFSET		PFN_UP(PHYS_OFFSET)

extern void clear_page(void * page);
extern void copy_page(void * to, void * from);

extern unsigned long shm_align_mask;

static inline unsigned long pages_do_alias(unsigned long addr1,
	unsigned long addr2)
{
	return (addr1 ^ addr2) & shm_align_mask;
}

struct page;

static inline void clear_user_page(void *addr, unsigned long vaddr,
	struct page *page)
{
	extern void (*flush_data_cache_page)(unsigned long addr);

	clear_page(addr);
	if (pages_do_alias((unsigned long) addr, vaddr & PAGE_MASK))
		flush_data_cache_page((unsigned long)addr);
}

static inline void copy_user_page(void *vto, void *vfrom, unsigned long vaddr, 
                                  struct page *to) 
{ 
  extern void (*flush_data_cache_page)(unsigned long addr); 
  copy_page(vto, vfrom); 
  if (pages_do_alias((unsigned long)vto, vaddr & PAGE_MASK)) 
    flush_data_cache_page((unsigned long)vto); 
} 

/*
 * These are used to make use of C type-checking..
 */
typedef struct { unsigned long pte; } pte_t;
#define pte_val(x)	((x).pte)
#define __pte(x)	((pte_t) { (x) } )
typedef struct page *pgtable_t;

/*
 * Right now we don't support 4-level pagetables, so all pud-related
 * definitions come from <asm-generic/pgtable-nopud.h>.
 */

/*
 * Finall the top of the hierarchy, the pgd
 */
typedef struct { unsigned long pgd; } pgd_t;
#define pgd_val(x)	((x).pgd)
#define __pgd(x)	((pgd_t) { (x) } )

/*
 * Manipulate page protection bits
 */
typedef struct { unsigned long pgprot; } pgprot_t;
#define pgprot_val(x)	((x).pgprot)
#define __pgprot(x)	((pgprot_t) { (x) } )

/*
 * On R4000-style MMUs where a TLB entry is mapping a adjacent even / odd
 * pair of pages we only have a single global bit per pair of pages.  When
 * writing to the TLB make sure we always have the bit set for both pages
 * or none.  This macro is used to access the `buddy' of the pte we're just
 * working on.
 */
#define ptep_buddy(x)	((pte_t *)((unsigned long)(x) ^ sizeof(pte_t)))

#endif /* !__ASSEMBLY__ */

/*
 * __pa()/__va() should be used only during mem init.
 */
#define __pa(x)								\
    ((unsigned long)(x) - PAGE_OFFSET + PHYS_OFFSET)
#define __va(x)		((void *)((unsigned long)(x) + PAGE_OFFSET - PHYS_OFFSET))
#define __pa_symbol(x)	__pa(RELOC_HIDE((unsigned long)(x), 0))

#define pfn_to_kaddr(pfn)	__va((pfn) << PAGE_SHIFT)
#define pfn_valid(pfn)                                              \
({                                                                  \
    unsigned long __pfn = (pfn);                                    \
    /* avoid <linux/bootmem.h> include hell */                      \
    extern unsigned long min_low_pfn;                               \
                                                                    \
    __pfn >= min_low_pfn && __pfn < max_mapnr;                      \
})

#define virt_to_page(kaddr)	pfn_to_page(PFN_DOWN(virt_to_phys(kaddr)))
#define virt_addr_valid(kaddr)	pfn_valid(PFN_DOWN(virt_to_phys(kaddr)))

#define VM_DATA_DEFAULT_FLAGS	(VM_READ | VM_WRITE | VM_EXEC | \
				 VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC)

#define UNCAC_ADDR(addr)	((addr) - PAGE_OFFSET + UNCAC_BASE)
#define CAC_ADDR(addr)		((addr) - UNCAC_BASE + PAGE_OFFSET)

#include <asm-generic/memory_model.h>
#include <asm-generic/getorder.h>

#endif /* _ASM_PAGE_H */
