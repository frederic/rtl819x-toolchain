/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003, 04, 05 Ralf Baechle (ralf@linux-mips.org)
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#include <asm/cacheops.h>
#include <asm/inst.h>
#include <asm/io.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/bootinfo.h>
#include <asm/rlxregs.h>
#include <asm/mmu_context.h>

#include <asm/cpu.h>
#include <asm/cpu-features.h>

/*
 * Maximum sizes:
 *
 * R4000 128 bytes S-cache:		0x58 bytes
 * R4600 v1.7:				0x5c bytes
 * R4600 v2.0:				0x60 bytes
 * With prefetching, 16 byte strides	0xa0 bytes
 */

static unsigned int clear_page_array[0x130 / 4];
void clear_page(void * page) __attribute__((alias("clear_page_array")));
EXPORT_SYMBOL(clear_page);

/*
 * Maximum sizes:
 *
 * R4000 128 bytes S-cache:		0x11c bytes
 * R4600 v1.7:				0x080 bytes
 * R4600 v2.0:				0x07c bytes
 * With prefetching, 16 byte strides	0x0b8 bytes
 */
static unsigned int copy_page_array[0x148 / 4];
void copy_page(void *to, void *from) __attribute__((alias("copy_page_array")));
EXPORT_SYMBOL(copy_page);

static int load_offset __cpuinitdata;
static int store_offset __cpuinitdata;

static unsigned int __cpuinitdata *dest, *epc;

static unsigned int instruction_pending;
static union mips_instruction delayed_mi;

static void __cpuinit emit_instruction(union mips_instruction mi)
{
  if (instruction_pending)
    *epc++ = delayed_mi.word;

  instruction_pending = 1;
  delayed_mi = mi;
}

static inline void flush_delay_slot_or_nop(void)
{
	if (instruction_pending) {
		*epc++ = delayed_mi.word;
		instruction_pending = 0;
		return;
	}

	*epc++ = 0;
}

static inline unsigned int *label(void)
{
	if (instruction_pending) {
		*epc++ = delayed_mi.word;
		instruction_pending = 0;
	}

	return epc;
}

static inline void build_insn_word(unsigned int word)
{
	union mips_instruction mi;

	mi.word		 = word;

	emit_instruction(mi);
}

static inline void build_nop(void)
{
	build_insn_word(0);			/* nop */
}

static inline void build_load_reg(int reg)
{
	union mips_instruction mi;
	unsigned int width;

	mi.i_format.opcode     = lw_op;
	width = 4;
	mi.i_format.rs         = 5;		/* $a1 */
	mi.i_format.rt         = reg;		/* $reg */
	mi.i_format.simmediate = load_offset;

	load_offset += width;
	emit_instruction(mi);
}

static void __cpuinit build_store_reg(int reg)
{
	union mips_instruction mi;
	unsigned int width;

	mi.i_format.opcode     = sw_op;
	width = 4;
	mi.i_format.rs         = 4;		/* $a0 */
	mi.i_format.rt         = reg;		/* $reg */
	mi.i_format.simmediate = store_offset;

	store_offset += width;
	emit_instruction(mi);
}

static inline void build_addiu_a2_a0(unsigned long offset)
{
	union mips_instruction mi;

	BUG_ON(offset > 0x7fff);

	mi.i_format.opcode     = addiu_op;
	mi.i_format.rs         = 4;		/* $a0 */
	mi.i_format.rt         = 6;		/* $a2 */
	mi.i_format.simmediate = offset;

	emit_instruction(mi);
}

static inline void build_addiu_a2(unsigned long offset)
{
	union mips_instruction mi;

	BUG_ON(offset > 0x7fff);

	mi.i_format.opcode     = addiu_op;
	mi.i_format.rs         = 6;		/* $a2 */
	mi.i_format.rt         = 6;		/* $a2 */
	mi.i_format.simmediate = offset;

	emit_instruction(mi);
}

static inline void build_addiu_a1(unsigned long offset)
{
	union mips_instruction mi;

	BUG_ON(offset > 0x7fff);

	mi.i_format.opcode     = addiu_op;
	mi.i_format.rs         = 5;		/* $a1 */
	mi.i_format.rt         = 5;		/* $a1 */
	mi.i_format.simmediate = offset;

	load_offset -= offset;

	emit_instruction(mi);
}

static inline void build_addiu_a0(unsigned long offset)
{
	union mips_instruction mi;

	BUG_ON(offset > 0x7fff);

	mi.i_format.opcode     = addiu_op;
	mi.i_format.rs         = 4;		/* $a0 */
	mi.i_format.rt         = 4;		/* $a0 */
	mi.i_format.simmediate = offset;

	store_offset -= offset;

	emit_instruction(mi);
}

static inline void build_bne(unsigned int *dest)
{
	union mips_instruction mi;

	mi.i_format.opcode = bne_op;
	mi.i_format.rs     = 6;			/* $a2 */
	mi.i_format.rt     = 4;			/* $a0 */
	mi.i_format.simmediate = dest - epc - 1;

	*epc++ = mi.word;
	flush_delay_slot_or_nop();
}

static inline void build_jr_ra(void)
{
	union mips_instruction mi;

	mi.r_format.opcode = spec_op;
	mi.r_format.rs     = 31;
	mi.r_format.rt     = 0;
	mi.r_format.rd     = 0;
	mi.r_format.re     = 0;
	mi.r_format.func   = jr_op;

	*epc++ = mi.word;
	flush_delay_slot_or_nop();
}

void __cpuinit build_clear_page(void)
{
	unsigned int loop_start;
	unsigned long off;

	epc = (unsigned int *) &clear_page_array;
	instruction_pending = 0;
	store_offset = 0;

	off = PAGE_SIZE;
	build_addiu_a2_a0(off);

    dest = label();
	//do {
		build_store_reg(0);
		build_store_reg(0);
		build_store_reg(0);
		build_store_reg(0);
	//} while (store_offset < half_scache_line_size());

	build_addiu_a0(2 * store_offset);
	loop_start = store_offset;
	//do {
		build_store_reg(0);
		build_store_reg(0);
		build_store_reg(0);
		build_store_reg(0);
	//} while ((store_offset - loop_start) < half_scache_line_size());
	build_bne(dest);

	build_jr_ra();

	BUG_ON(epc > clear_page_array + ARRAY_SIZE(clear_page_array));
}

void __cpuinit build_copy_page(void)
{
	unsigned int loop_start;

	epc = (unsigned int *) &copy_page_array;
	store_offset = load_offset = 0;
	instruction_pending = 0;

	build_addiu_a2_a0(PAGE_SIZE);

    dest = label();
	loop_start = store_offset;
	//do {
		build_load_reg( 8);
		build_load_reg( 9);
		build_load_reg(10);
		build_load_reg(11);
		build_store_reg( 8);
		build_store_reg( 9);
		build_store_reg(10);
		build_store_reg(11);
	//} while ((store_offset - loop_start) < half_scache_line_size());

	build_addiu_a0(2 * store_offset);
	build_addiu_a1(2 * load_offset);
	loop_start = store_offset;
	//do {
		build_load_reg( 8);
		build_load_reg( 9);
		build_load_reg(10);
		build_load_reg(11);
		build_store_reg( 8);
		build_store_reg( 9);
		build_store_reg(10);
		build_store_reg(11);
	//} while ((store_offset - loop_start) < half_scache_line_size());
	build_bne(dest);

	build_jr_ra();

	BUG_ON(epc > copy_page_array + ARRAY_SIZE(copy_page_array));
}
