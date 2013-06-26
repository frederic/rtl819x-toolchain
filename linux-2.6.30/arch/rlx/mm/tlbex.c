/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Synthesize TLB refill handlers at runtime.
 *
 * Copyright (C) 2004, 2005, 2006, 2008  Thiemo Seufer
 * Copyright (C) 2005, 2007  Maciej W. Rozycki
 * Copyright (C) 2006  Ralf Baechle (ralf@linux-mips.org)
 *
 * ... and the days got worse and worse and now you see
 * I've gone completly out of my mind.
 *
 * They're coming to take me a away haha
 * they're coming to take me a away hoho hihi haha
 * to the funny farm where code is beautiful all the time ...
 *
 * (Condolences to Napoleon XIV)
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/init.h>

#include <asm/mmu_context.h>

#include "uasm.h"

/* Handle labels (which must be positive integers). */
enum label_id
{
  label_second_part = 1,
  label_leave,
#ifdef MODULE_START
  label_module_alloc,
#endif
  label_vmalloc,
  label_vmalloc_done,
  label_tlbw_hazard,
  label_split,
  label_nopage_tlbl,
  label_nopage_tlbs,
  label_nopage_tlbm,
  label_rlx_write_probe_fail,
};

UASM_L_LA (_second_part) UASM_L_LA (_leave)
#ifdef MODULE_START
  UASM_L_LA (_module_alloc)
#endif
  UASM_L_LA (_vmalloc)
UASM_L_LA (_vmalloc_done)
UASM_L_LA (_tlbw_hazard)
UASM_L_LA (_split)
UASM_L_LA (_nopage_tlbl)
UASM_L_LA (_nopage_tlbs)
UASM_L_LA (_nopage_tlbm)
UASM_L_LA (_rlx_write_probe_fail)
/*
 * For debug purposes.
 */
static inline void
dump_handler (const u32 * handler, int count)
{
  int i;

  pr_debug ("\t.set push\n");
  pr_debug ("\t.set noreorder\n");

  for (i = 0; i < count; i++)
    pr_debug ("\t%p\t.word 0x%08x\n", &handler[i], handler[i]);

  pr_debug ("\t.set pop\n");
}

/* The only general purpose registers allowed in TLB handlers. */
#define K0		26
#define K1		27

/* Some CP0 registers */
#define C0_INDEX	0, 0
#define C0_ENTRYLO0	2, 0
#define C0_TCBIND	2, 2
#define C0_ENTRYLO1	3, 0
#define C0_CONTEXT	4, 0
#define C0_BADVADDR	8, 0
#define C0_ENTRYHI	10, 0
#define C0_EPC		14, 0
#define C0_XCONTEXT	20, 0

#define GET_CONTEXT(buf, reg) UASM_i_MFC0(buf, reg, C0_CONTEXT)

/* The worst case length of the handler is around 18 instructions for
 * R3000-style TLBs and up to 63 instructions for R4000-style TLBs.
 * Maximum space available is 32 instructions for R3000 and 64
 * instructions for R4000.
 *
 * We deliberately chose a buffer size of 128, so we won't scribble
 * over anything important on overflow before we panic.
 */
static u32 tlb_handler[128] __cpuinitdata;

/* simply assume worst case size for labels and relocs */
static struct uasm_label labels[128] __cpuinitdata;
static struct uasm_reloc relocs[128] __cpuinitdata;

#define RLX_TRAP_TLB_BASE   0x80000000
#define RLX_TRAP_TLB_SIZE   0x80

/*
 * The R3000 TLB handler is simple.
 */
static void __cpuinit
build_rlx_tlb_refill_handler (void)
{
  long pgdc = (long) pgd_current;
  u32 *p;

  memset (tlb_handler, 0, sizeof (tlb_handler));
  p = tlb_handler;

  uasm_i_mfc0 (&p, K0, C0_BADVADDR);
  uasm_i_lui (&p, K1, uasm_rel_hi (pgdc));	/* cp0 delay */
  uasm_i_lw (&p, K1, uasm_rel_lo (pgdc), K1);
  uasm_i_srl (&p, K0, K0, 22);	/* load delay */
  uasm_i_sll (&p, K0, K0, 2);
  uasm_i_addu (&p, K1, K1, K0);
  uasm_i_mfc0 (&p, K0, C0_CONTEXT);
  uasm_i_lw (&p, K1, 0, K1);	/* cp0 delay */
  uasm_i_andi (&p, K0, K0, 0xffc);	/* load delay */
  uasm_i_addu (&p, K1, K1, K0);
  uasm_i_lw (&p, K0, 0, K1);
  uasm_i_nop (&p);		/* load delay */
  uasm_i_mtc0 (&p, K0, C0_ENTRYLO0);
  uasm_i_mfc0 (&p, K1, C0_EPC);	/* cp0 delay */
  uasm_i_tlbwr (&p);		/* cp0 delay */
  uasm_i_jr (&p, K1);
  uasm_i_rfe (&p);		/* branch delay */

  if (p > tlb_handler + 32)
    panic ("TLB refill handler space exceeded");

  pr_debug ("Wrote TLB refill handler (%u instructions).\n",
	    (unsigned int) (p - tlb_handler));

  memcpy ((void *) RLX_TRAP_TLB_BASE, tlb_handler, RLX_TRAP_TLB_SIZE);

  dump_handler ((u32 *) RLX_TRAP_TLB_BASE, 32);
}

#if 0
static void __cpuinit
build_adjust_context (u32 ** p, unsigned int ctx)
{
  unsigned int shift = 4 - (PTE_T_LOG2 + 1) + PAGE_SHIFT - 12;
  unsigned int mask = (PTRS_PER_PTE / 2 - 1) << (PTE_T_LOG2 + 1);

  if (shift)
    UASM_i_SRL (p, ctx, ctx, shift);

  uasm_i_andi (p, ctx, ctx, mask);
}
#endif

/*
 * TLB load/store/modify handlers.
 *
 * Only the fastpath gets synthesized at runtime, the slowpath for
 * do_page_fault remains normal asm.
 */
extern void tlb_do_page_fault_0 (void);
extern void tlb_do_page_fault_1 (void);

/*
 * 128 instructions for the fastpath handler is generous and should
 * never be exceeded.
 */
#define FASTPATH_SIZE 128

u32 handle_tlbl[FASTPATH_SIZE] __cacheline_aligned;
u32 handle_tlbs[FASTPATH_SIZE] __cacheline_aligned;
u32 handle_tlbm[FASTPATH_SIZE] __cacheline_aligned;

static void __cpuinit
iPTE_LW (u32 ** p, struct uasm_label **l, unsigned int pte, unsigned int ptr)
{
  UASM_i_LW (p, pte, 0, ptr);
}

static void __cpuinit
iPTE_SW (u32 ** p, struct uasm_reloc **r, unsigned int pte, unsigned int ptr,
	 unsigned int mode)
{
  uasm_i_ori (p, pte, pte, mode);
  UASM_i_SW (p, pte, 0, ptr);
}

/*
 * Check if PTE is present, if not then jump to LABEL. PTR points to
 * the page table where this PTE is located, PTE will be re-loaded
 * with it's original value.
 */
static void __cpuinit
build_pte_present (u32 ** p, struct uasm_label **l, struct uasm_reloc **r,
		   unsigned int pte, unsigned int ptr, enum label_id lid)
{
  uasm_i_andi (p, pte, pte, _PAGE_PRESENT | _PAGE_READ);
  uasm_i_xori (p, pte, pte, _PAGE_PRESENT | _PAGE_READ);
  uasm_il_bnez (p, r, pte, lid);
  iPTE_LW (p, l, pte, ptr);
}

/* Make PTE valid, store result in PTR. */
static void __cpuinit
build_make_valid (u32 ** p, struct uasm_reloc **r, unsigned int pte,
		  unsigned int ptr)
{
  unsigned int mode = _PAGE_VALID | _PAGE_ACCESSED;

  iPTE_SW (p, r, pte, ptr, mode);
}

/*
 * Check if PTE can be written to, if not branch to LABEL. Regardless
 * restore PTE with value from PTR when done.
 */
static void __cpuinit
build_pte_writable (u32 ** p, struct uasm_label **l, struct uasm_reloc **r,
		    unsigned int pte, unsigned int ptr, enum label_id lid)
{
  uasm_i_andi (p, pte, pte, _PAGE_PRESENT | _PAGE_WRITE);
  uasm_i_xori (p, pte, pte, _PAGE_PRESENT | _PAGE_WRITE);
  uasm_il_bnez (p, r, pte, lid);
  iPTE_LW (p, l, pte, ptr);
}

/* Make PTE writable, update software status bits as well, then store
 * at PTR.
 */
static void __cpuinit
build_make_write (u32 ** p, struct uasm_reloc **r, unsigned int pte,
		  unsigned int ptr)
{
  unsigned int mode = (_PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_VALID
		       | _PAGE_DIRTY);

  iPTE_SW (p, r, pte, ptr, mode);
}

/*
 * Check if PTE can be modified, if not branch to LABEL. Regardless
 * restore PTE with value from PTR when done.
 */
static void __cpuinit
build_pte_modifiable (u32 ** p, struct uasm_label **l, struct uasm_reloc **r,
		      unsigned int pte, unsigned int ptr, enum label_id lid)
{
  uasm_i_andi (p, pte, pte, _PAGE_WRITE);
  uasm_il_beqz (p, r, pte, lid);
  iPTE_LW (p, l, pte, ptr);
}

/*
 * R3000 style TLB load/store/modify handlers.
 */

/*
 * This places the pte into ENTRYLO0 and writes it with tlbwi.
 * Then it returns.
 */
static void __cpuinit
build_rlx_pte_reload_tlbwi (u32 ** p, unsigned int pte, unsigned int tmp)
{
  uasm_i_mtc0 (p, pte, C0_ENTRYLO0);	/* cp0 delay */
  uasm_i_mfc0 (p, tmp, C0_EPC);	/* cp0 delay */
  uasm_i_tlbwi (p);
  uasm_i_jr (p, tmp);
  uasm_i_rfe (p);		/* branch delay */
}

/*
 * This places the pte into ENTRYLO0 and writes it with tlbwi
 * or tlbwr as appropriate.  This is because the index register
 * may have the probe fail bit set as a result of a trap on a
 * kseg2 access, i.e. without refill.  Then it returns.
 */
static void __cpuinit
build_rlx_tlb_reload_write (u32 ** p, struct uasm_label **l,
			    struct uasm_reloc **r, unsigned int pte,
			    unsigned int tmp)
{
  uasm_i_mfc0 (p, tmp, C0_INDEX);
  uasm_i_mtc0 (p, pte, C0_ENTRYLO0);	/* cp0 delay */
  uasm_il_bltz (p, r, tmp, label_rlx_write_probe_fail);	/* cp0 delay */
  uasm_i_mfc0 (p, tmp, C0_EPC);	/* branch delay */
  uasm_i_tlbwi (p);		/* cp0 delay */
  uasm_i_jr (p, tmp);
  uasm_i_rfe (p);		/* branch delay */
  uasm_l_rlx_write_probe_fail (l, *p);
  uasm_i_tlbwr (p);		/* cp0 delay */
  uasm_i_jr (p, tmp);
  uasm_i_rfe (p);		/* branch delay */
}

static void __cpuinit
build_rlx_tlbchange_handler_head (u32 ** p, unsigned int pte,
				  unsigned int ptr)
{
  long pgdc = (long) pgd_current;

  uasm_i_mfc0 (p, pte, C0_BADVADDR);
  uasm_i_lui (p, ptr, uasm_rel_hi (pgdc));	/* cp0 delay */
  uasm_i_lw (p, ptr, uasm_rel_lo (pgdc), ptr);
  uasm_i_srl (p, pte, pte, 22);	/* load delay */
  uasm_i_sll (p, pte, pte, 2);
  uasm_i_addu (p, ptr, ptr, pte);
  uasm_i_mfc0 (p, pte, C0_CONTEXT);
  uasm_i_lw (p, ptr, 0, ptr);	/* cp0 delay */
  uasm_i_andi (p, pte, pte, 0xffc);	/* load delay */
  uasm_i_addu (p, ptr, ptr, pte);
  uasm_i_lw (p, pte, 0, ptr);
  uasm_i_tlbp (p);		/* load delay */
}

static void __cpuinit
build_rlx_tlb_load_handler (void)
{
  u32 *p = handle_tlbl;
  struct uasm_label *l = labels;
  struct uasm_reloc *r = relocs;

  memset (handle_tlbl, 0, sizeof (handle_tlbl));
  memset (labels, 0, sizeof (labels));
  memset (relocs, 0, sizeof (relocs));

  build_rlx_tlbchange_handler_head (&p, K0, K1);
  build_pte_present (&p, &l, &r, K0, K1, label_nopage_tlbl);
  uasm_i_nop (&p);		/* load delay */
  build_make_valid (&p, &r, K0, K1);
  build_rlx_tlb_reload_write (&p, &l, &r, K0, K1);

  uasm_l_nopage_tlbl (&l, p);
  uasm_i_j (&p, (unsigned long) tlb_do_page_fault_0 & 0x0fffffff);
  uasm_i_nop (&p);

  if ((p - handle_tlbl) > FASTPATH_SIZE)
    panic ("TLB load handler fastpath space exceeded");

  uasm_resolve_relocs (relocs, labels);
  pr_debug ("Wrote TLB load handler fastpath (%u instructions).\n",
	    (unsigned int) (p - handle_tlbl));

  dump_handler (handle_tlbl, ARRAY_SIZE (handle_tlbl));
}

static void __cpuinit
build_rlx_tlb_store_handler (void)
{
  u32 *p = handle_tlbs;
  struct uasm_label *l = labels;
  struct uasm_reloc *r = relocs;

  memset (handle_tlbs, 0, sizeof (handle_tlbs));
  memset (labels, 0, sizeof (labels));
  memset (relocs, 0, sizeof (relocs));

  build_rlx_tlbchange_handler_head (&p, K0, K1);
  build_pte_writable (&p, &l, &r, K0, K1, label_nopage_tlbs);
  uasm_i_nop (&p);		/* load delay */
  build_make_write (&p, &r, K0, K1);
  build_rlx_tlb_reload_write (&p, &l, &r, K0, K1);

  uasm_l_nopage_tlbs (&l, p);
  uasm_i_j (&p, (unsigned long) tlb_do_page_fault_1 & 0x0fffffff);
  uasm_i_nop (&p);

  if ((p - handle_tlbs) > FASTPATH_SIZE)
    panic ("TLB store handler fastpath space exceeded");

  uasm_resolve_relocs (relocs, labels);
  pr_debug ("Wrote TLB store handler fastpath (%u instructions).\n",
	    (unsigned int) (p - handle_tlbs));

  dump_handler (handle_tlbs, ARRAY_SIZE (handle_tlbs));
}

static void __cpuinit
build_rlx_tlb_modify_handler (void)
{
  u32 *p = handle_tlbm;
  struct uasm_label *l = labels;
  struct uasm_reloc *r = relocs;

  memset (handle_tlbm, 0, sizeof (handle_tlbm));
  memset (labels, 0, sizeof (labels));
  memset (relocs, 0, sizeof (relocs));

  build_rlx_tlbchange_handler_head (&p, K0, K1);
  build_pte_modifiable (&p, &l, &r, K0, K1, label_nopage_tlbm);
  uasm_i_nop (&p);		/* load delay */
  build_make_write (&p, &r, K0, K1);
  build_rlx_pte_reload_tlbwi (&p, K0, K1);

  uasm_l_nopage_tlbm (&l, p);
  uasm_i_j (&p, (unsigned long) tlb_do_page_fault_1 & 0x0fffffff);
  uasm_i_nop (&p);

  if ((p - handle_tlbm) > FASTPATH_SIZE)
    panic ("TLB modify handler fastpath space exceeded");

  uasm_resolve_relocs (relocs, labels);
  pr_debug ("Wrote TLB modify handler fastpath (%u instructions).\n",
	    (unsigned int) (p - handle_tlbm));

  dump_handler (handle_tlbm, ARRAY_SIZE (handle_tlbm));
}

void __cpuinit
build_tlb_refill_handler (void)
{
  /*
   * The refill handler is generated per-CPU, multi-node systems
   * may have local storage for it. The other handlers are only
   * needed once.
   */
  static int run_once = 0;

  build_rlx_tlb_refill_handler ();
  if (!run_once)
    {
      build_rlx_tlb_load_handler ();
      build_rlx_tlb_store_handler ();
      build_rlx_tlb_modify_handler ();
      run_once++;
    }
}

void __cpuinit
flush_tlb_handlers (void)
{
  local_flush_icache_range ((unsigned long) handle_tlbl,
                            (unsigned long) handle_tlbl + sizeof (handle_tlbl));
  local_flush_icache_range ((unsigned long) handle_tlbs,
                            (unsigned long) handle_tlbs + sizeof (handle_tlbs));
  local_flush_icache_range ((unsigned long) handle_tlbm,
                            (unsigned long) handle_tlbm + sizeof (handle_tlbm));
}
