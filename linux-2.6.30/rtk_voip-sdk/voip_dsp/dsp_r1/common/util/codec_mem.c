#include <linux/interrupt.h>
#include <linux/config.h>
#include <linux/types.h>
#include "typedef.h"
#include "rtk_voip.h"
#include "codec_mem.h"
#include "voip_addrspace.h"

#include "../../../include/lec.h"
#include "codec_descriptor.h"

#ifdef CONFIG_RTK_VOIP_MODULE
	#define DMEN_G726 0		/* DMEN for G.726, 1: enable  0: disable */
#else
	#define DMEN_G726 1		/* DMEN for G.726, 1: enable  0: disable */
#endif /* CONFIG_RTK_VOIP_MODULE */

/* declare codec stacks in bss section */
/*unsigned long g729_dummy_topstack[G729_DUMMY_SSIZE] MEM_SECTION(".g729_dummy_topstack") ALIGN(8);*/
/*unsigned long g729_dmem_stack[G729_DMEM_SSIZE] MEM_SECTION(".g729_dmem_stack1K") ALIGN(8);*/
unsigned long g729_orig_sp;
unsigned long g729_dmem_sp;
int16_t* g729_TmpVct = (int16_t*) (&codec_dmem_area[G729_DMEM_SSIZE]);
#if defined( SUPPORT_COMFORT_NOISE ) && !defined( SIMPLIFIED_COMFORT_NOISE )
#error "G.729 and g.726 dmem data is overlay, will over write data EACH OTHER!!!!!DANGEROUS"
#endif

#if ((G729_BSS_SIZE + G729_DMEM_SSIZE) > 1024)
#error "stack size and DATA size over 4kbyte"
#endif


//unsigned long g7231_dummy_topstack[G7231_DUMMY_SSIZE] MEM_SECTION(".g7231_dummy_topstack") ALIGN(8);
unsigned long g7231_dmem_stack[G7231_DMEM_SSIZE] MEM_SECTION(".g7231_dmem_stack1K") ALIGN(8);
unsigned long g7231_orig_sp;
unsigned long g7231_dmem_sp;

/*unsigned long g726_dummy_topstack[G726_DUMMY_SSIZE] MEM_SECTION(".g726_dummy_topstack") ALIGN(8);*/
/*unsigned long g726_dmem_stack[G726_DMEM_SSIZE] MEM_SECTION(".g726_dmem_stack1K") ALIGN(8);*/
int16_t* g726_TmpVct = (int16_t*) (&codec_dmem_area[G726_DMEM_SSIZE]);
unsigned long g726_orig_sp;
unsigned long g726_dmem_sp;

#if ((G726_DATA_SIZE + G726_DMEM_SSIZE) > 1024)
#error "stack size and DATA size over 4kbyte"
#endif

/*unsigned long gsmfr_dummy_topstack[GSMFR_DUMMY_SSIZE] MEM_SECTION(".gsmfr_dummy_topstack") ALIGN(8);*/
/*unsigned long gsmfr_dmem_stack[GSMFR_DMEM_SSIZE] MEM_SECTION(".gsmfr_dmem_stack1K") ALIGN(8);*/
int16_t* gsmfr_TmpVct = (int16_t*) (&codec_dmem_area[GSMFR_DMEM_SSIZE]);
unsigned long gsmfr_orig_sp;
unsigned long gsmfr_dmem_sp;
#if ((GSMFR_BSS_SIZE + GSMFR_DMEM_SSIZE) > 1024)
#error "stack size and BSS size over 4kbyte"
#endif

//unsigned long iLBC_dummy_topstack[ILBC_DUMMY_SSIZE] MEM_SECTION(".iLBC_dummy_topstack") ALIGN(8);
//unsigned long iLBC_dmem_stack[ILBC_DMEM_SSIZE] MEM_SECTION(".iLBC_dmem_stack1K") ALIGN(8);
unsigned long iLBC_orig_sp;
unsigned long iLBC_dmem_sp;

#if DMEN_STACK_LEC
/*unsigned long lec_dummy_topstack[LEC_DUMMY_SSIZE] MEM_SECTION(".lec_dummy_topstack") ALIGN(8);*/
unsigned long lec_dmem_stack[LEC_DMEM_SSIZE] MEM_SECTION(".lec_dmem_stack1K") ALIGN(8);
unsigned long lec_orig_sp;
unsigned long lec_dmem_sp;
#endif

/*unsigned long sys_dummy_topstack[SYS_DUMMY_SSIZE] MEM_SECTION(".sys_dummy_topstack") ALIGN(8);*/
unsigned long sys_dmem_stack[SYS_DMEM_SSIZE] MEM_SECTION(".sys_dmem_stack") ALIGN(8);
unsigned long sys_orig_sp;
unsigned long sys_dmem_sp;

int16_t* g711wb_TmpVct = (int16_t*) (&codec_dmem_area[G711WB_DMEM_SSIZE]);

//unsigned long codec_dummy_topstack[CODEC_DUMMY_SSIZE] MEM_SECTION(".codec_dummy_topstack") ALIGN(8);
unsigned long codec_dmem_area[DMEM_SIZE/4] MEM_SECTION(".codec_dmem_area") ALIGN(8);
			//DMEM_SIZE is in byte, convert to long.

/* local functions */
int set_g7231enc_imem(void);
int set_g7231dec_imem(void);
int set_g729enc_imem(void);
int set_g729dec_imem(void);
static int set_common_imem(void);

extern void set_g726_dmem(unsigned char rate);
extern short g726_rate[];

unsigned long* __imem_type;
int imem_size = 0x1000;
int dmem_size = 0x1000;
/* record the imem map codec. */
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672)  && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
#ifndef CODEC_IMEM_SWAP_NEW
static unsigned int imem_data =0;
#endif
#endif

/* record the imem internal data */
static unsigned int imem_data_internal =0;

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
    #if defined( CONFIG_RTL8192SE ) && defined( CONFIG_DEFAULTS_KERNEL_2_6 )
	#define COMMON_INSIDE	(1<<2)	/* for imem_data_internal only*/
	#define ILBC_DEC_INSIDE	(1<<3)	/* for imem_data_internal only*/
	#define ILBC_ENC_INSIDE	(1<<4)	/* for imem_data_internal only*/
	#define SYSTEM_INSIDE	(1<<8)	/* for imem_data_internal only*/
	/* for 92se G.729 G.7231 common (no system )is put together in 16Kbyte */
    #else
	#define ILBC_DEC_INSIDE	(1<<3)	/* for imem_data_internal only*/
	#define ILBC_ENC_INSIDE	(1<<4)	/* for imem_data_internal only*/
	#define SYSTEM_INSIDE	(1<<8)	/* for imem_data_internal only*/
	/* for 8952 G.729 G.7231 common system is put together in 16Kbyte */
    #endif
#else

#define G729_INSIDE	(1<<0)	/* both for imem_data, imem_data_internal */
#define G7231_INSIDE	(1<<1)	/* both for imem_data, imem_data_internal */

#define COMMON_INSIDE	(1<<2)	/* for imem_data_internal only*/
#define ILBC_DEC_INSIDE	(1<<3)	/* for imem_data_internal only*/
#define ILBC_ENC_INSIDE	(1<<4)	/* for imem_data_internal only*/
#define SYSTEM_INSIDE	(1<<8)	/* for imem_data_internal only*/

#endif

/* define this improve codec imem swap useage, better performance */
#define CODEC_IMEM_SWAP_NEW

#ifdef VOIP_CPU_CACHE_WRITE_BACK
/* cctl A transition from 0 to 1 on DWB initiates a hardware sequence to
 * write-back all dirty lines
 */
static void DCACHE_WRITE_BACK_DIRTY( void )
{
__asm__ volatile(
    "mfc0 $8,$20\n\t"
    "nop\n\t"
    "or $8, $8, 0x100\n\t"
    "xori $9, $8, 0x100\n\t"
    "nop\n\t"
    "nop\n\t"
    "mtc0 $9,$20\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "mtc0 $8,$20\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    );
}

/* cctl A transition from 0 to 1 on Iinval initiates a hardware sequence to
 * invalidation I cache
 */
static void ICACHE_INV( void )
{
__asm__ volatile(
    "mfc0 $8,$20\n\t"
    "nop\n\t"
    "or $8, $8, 0x2\n\t"
    "xori $9, $8, 0x2\n\t"
    "nop\n\t"
    "nop\n\t"
    "mtc0 $9,$20\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "mtc0 $8,$20\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    );
}
#endif

#ifndef CONFIG_RTK_VOIP_MODULE

int set_system_imem(void)
{
	static unsigned long flags;

	if(imem_data_internal!=SYSTEM_INSIDE)
	{
    #if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
		unsigned long size_in_word; /* word size of IMEM code*/
		extern unsigned long __iram, __iram_end;
		size_in_word = ((((unsigned long)&__iram_end - (unsigned long)&__iram ) + 3) & (~3))>>2;
		if (size_in_word >=0x1000)
			size_in_word = 0x1000;
		else
			size_in_word = size_in_word & 0xfff;
		save_flags(flags);
		cli();
		set_IMEM((unsigned long)&__iram,(unsigned long)&__iram+(imem_size-1));
		*(volatile unsigned long *) IM_CTL = IM_CTL_RSET;
		*(volatile unsigned long *) IM_CTL = 0x0;
		*(volatile unsigned long *) IM_IMEM_SA = 0;
		//*(volatile unsigned long *) IM_EXTM_SA = (unsigned long)&__iram & 0xfffffff; /* convert to physical address */
		*(volatile unsigned long *) IM_EXTM_SA = Virtual2Physical( &__iram ); /* convert to physical address */
		*(volatile unsigned long *) IM_CTL = IM_CTL_START | IM_CTL_SDRAM2IMEM | size_in_word;
		*(volatile unsigned long *) IM_CTL;
		while(*(volatile unsigned long *) IM_CTL & IM_CTL_START);
		restore_flags(flags);
    #else
	#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
		save_flags(flags);
		cli();
		set_and_fill_IMEM((unsigned long)__imem_type,(unsigned long)__imem_type+(imem_size-1));
		restore_flags(flags);
    #endif
    #endif
		imem_data_internal = SYSTEM_INSIDE;
	}

	return 0;
}
#endif /* !CONFIG_RTK_VOIP_MODULE */



void G711_set_codec_mem(int state, int g726_rate)
{
	/* case CODEC_TYPE_G711: */
  #if defined( SUPPORT_COMFORT_NOISE ) && !defined( SIMPLIFIED_COMFORT_NOISE )
   #ifdef CONFIG_RTK_VOIP_G729AB
	set_DMEM((unsigned long)&__g729_dmem_start, (unsigned long)dmem_size-1);

	if(state & DECODE)
	{
		set_g729dec_imem();

	}
	else
	{
		set_g729enc_imem();
	}
   #else
    #error "Out of consider !!??"
   #endif
  #else
	set_common_imem();
  #endif
}

#ifdef CONFIG_RTK_VOIP_G722
void G722_set_codec_mem(int state, int g726_rate)
{
  #if defined( SUPPORT_COMFORT_NOISE ) && !defined( SIMPLIFIED_COMFORT_NOISE )
   #ifdef CONFIG_RTK_VOIP_G729AB
	set_DMEM(&__g729_dmem_start, (unsigned long)dmem_size-1);

	if(state & DECODE)
	{
		set_g729dec_imem();

	}
	else
	{
		set_g729enc_imem();
	}
   #else
    #error "Out of consider !!??"
   #endif
  #else
	set_common_imem();
  #endif
}
#endif /* CONFIG_RTK_VOIP_G722 */

#ifdef CONFIG_RTK_VOIP_G7111
void G7111_set_codec_mem(int state, int g726_rate)
{
	set_DMEM((unsigned long)&__codec_dmem_4k_start, (unsigned long)dmem_size-1);
	set_common_imem();
}
#endif /* CONFIG_RTK_VOIP_G7111 */

#ifdef CONFIG_RTK_VOIP_G7231
void G723_set_codec_mem(int state, int g726_rate)
{
	/* case CODEC_TYPE_G7231: */

	//set_DMEM(&g7231_dmem_stack[1024], dmem_size-1);
	//set_DMEM(&__g7231_dmem_start, dmem_size-1);
	//set_DMEM((unsigned long)&__codec_dmem_start, (unsigned long)dmem_size-1);
	set_DMEM((unsigned long)&__codec_dmem_4k_start, (unsigned long)dmem_size-1);
	
	if(state & DECODE)
	{
		set_g7231dec_imem();
	}
	else
	{
		set_g7231enc_imem();
	}
}
#endif /* CONFIG_RTK_VOIP_G7231 */

#ifdef CONFIG_RTK_VOIP_G729AB
void G729_set_codec_mem(int state, int g726_rate)
{
	/* case CODEC_TYPE_G729: */

	//set_DMEM(&__g729_dmem_start, dmem_size-1);
	//set_DMEM((unsigned long)&__codec_dmem_start, (unsigned long)dmem_size-1);
	set_DMEM((unsigned long)&__codec_dmem_4k_start, (unsigned long)dmem_size-1);

	if(state & DECODE)
	{
		set_g729dec_imem();
	}
	else
	{
		set_g729enc_imem();
	}
}
#endif /* CONFIG_RTK_VOIP_G729AB */

#ifdef CONFIG_RTK_VOIP_G726
void G726_set_codec_mem(int state, int g726_rate)
{
	/* case CODEC_TYPE_G726: */

   #if DMEN_G726
	//set_DMEM(&__g726_dmem_start, dmem_size-1);
	//set_DMEM((unsigned long)&__codec_dmem_start, (unsigned long)dmem_size-1);
	set_DMEM((unsigned long)&__codec_dmem_4k_start, (unsigned long)dmem_size-1);

	if(state & DECODE)
	{
		set_g726_dmem(g726_rate);
	}
	else
	{
		set_g726_dmem(g726_rate);
	}
   #endif


   #if defined( SUPPORT_COMFORT_NOISE ) && !defined( SIMPLIFIED_COMFORT_NOISE )

	#if !defined (DMEN_G726)
	set_DMEM((unsigned long)&__g729_dmem_start, (unsigned long)dmem_size-1);
	#endif

	if(state & DECODE)
	{
		set_g729dec_imem();

	}
	else
	{
		set_g729enc_imem();
	}

   #else
	set_common_imem();
   #endif
}
#endif /* CONFIG_RTK_VOIP_G726 */

#ifdef CONFIG_RTK_VOIP_GSMFR
void GSMfr_set_codec_mem(int state, int g726_rate)
{
	/* case CODEC_TYPE_GSMFR: */

	//set_DMEM(&__gsmfr_dmem_start, dmem_size-1);
	//set_DMEM((unsigned long)&__codec_dmem_start, (unsigned long)dmem_size-1);
	set_DMEM((unsigned long)&__codec_dmem_4k_start, (unsigned long)dmem_size-1);
	set_common_imem();
}
#endif /* CONFIG_RTK_VOIP_GSMFR */

#ifdef CONFIG_RTK_VOIP_ILBC
void iLBC_set_codec_mem(int state, int g726_rate)
{
	/* case CODEC_TYPE_ILBC: */

	//set_DMEM((unsigned long)&__codec_dmem_start, (unsigned long)dmem_size-1);
	set_DMEM((unsigned long)&__codec_dmem_4k_start, (unsigned long)dmem_size-1);
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
	if(state & DECODE)
	{
    #if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
		//if (imem_data_internal!=ILBC_DEC_INSIDE) {
			//printk("DECODE\n");
			unsigned long flags;
			unsigned long size_in_word; /* word size of IMEM code*/
			extern unsigned long __ILBCDEC_IMEM_end;
			size_in_word = ((((unsigned long)&__ILBCDEC_IMEM_end - (unsigned long)&__ILBCDEC_IMEM_start ) + 3) & (~3))>>2;
			if (size_in_word >=0x1000)
				size_in_word = 0x1000;
			else
				size_in_word = size_in_word & 0xfff;
			//printk("size%x", size_in_word);
			save_flags(flags);
			cli();
			set_IMEM((unsigned long)&__ILBCDEC_IMEM_start,(unsigned long)&__ILBCDEC_IMEM_start+(imem_size-1));
			*(volatile unsigned long *) IM_CTL = IM_CTL_RSET;
			*(volatile unsigned long *) IM_CTL = 0x0;
			*(volatile unsigned long *) IM_IMEM_SA = 0;
			//*(volatile unsigned long *) IM_EXTM_SA = (unsigned long)&__ILBCDEC_IMEM_start & 0xfffffff; /* convert to physical address */
			*(volatile unsigned long *) IM_EXTM_SA = Virtual2Physical( &__ILBCDEC_IMEM_start ); /* convert to physical address */
			*(volatile unsigned long *) IM_CTL = IM_CTL_START | IM_CTL_SDRAM2IMEM | size_in_word;
			*(volatile unsigned long *) IM_CTL;
			restore_flags(flags);
			//printk("dma%x", *(volatile unsigned long *) IM_CTL);
			while(*(volatile unsigned long *) IM_CTL & IM_CTL_START);
			//printk("dmaa0x%x,", *(volatile unsigned long *) IM_CTL);
		//}
    #else
		set_and_fill_IMEM(&__ILBCDEC_IMEM_start,(unsigned long)&__ILBCDEC_IMEM_start+(imem_size-1));
    #endif
		imem_data_internal = ILBC_DEC_INSIDE;
	}
	else
	{
    #if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
		//if (imem_data_internal!=ILBC_ENC_INSIDE) {
			//printk("ENCODE\n");
			unsigned long flags;
			unsigned long size_in_word; /* word size of IMEM code*/
			extern unsigned long __ILBCENC_IMEM_end;
			size_in_word = ((((unsigned long)&__ILBCENC_IMEM_end - (unsigned long)&__ILBCENC_IMEM_start ) + 3) & (~3))>>2;
			if (size_in_word >=0x1000)
				size_in_word = 0x1000;
			else
				size_in_word = size_in_word & 0xfff;
			save_flags(flags);
			cli();
			set_IMEM((unsigned long)&__ILBCENC_IMEM_start,(unsigned long)&__ILBCENC_IMEM_start+(imem_size-1));
			*(volatile unsigned long *) IM_CTL = IM_CTL_RSET;
			*(volatile unsigned long *) IM_CTL = 0x0;
			*(volatile unsigned long *) IM_IMEM_SA = 0;
			//*(volatile unsigned long *) IM_EXTM_SA = (unsigned long)&__ILBCENC_IMEM_start & 0xfffffff; /* convert to physical address */
			*(volatile unsigned long *) IM_EXTM_SA = Virtual2Physical( &__ILBCENC_IMEM_start ); /* convert to physical address */
			*(volatile unsigned long *) IM_CTL = IM_CTL_START | IM_CTL_SDRAM2IMEM | size_in_word;
			*(volatile unsigned long *) IM_CTL;
			restore_flags(flags);
			while(*(volatile unsigned long *) IM_CTL & IM_CTL_START);
		//}
    #else
		set_and_fill_IMEM(&__ILBCENC_IMEM_start,(unsigned long)&__ILBCENC_IMEM_start+(imem_size-1));
    #endif
		imem_data_internal = ILBC_ENC_INSIDE;
	}
#endif
	//set_common_imem();
}
#endif /* CONFIG_RTK_VOIP_ILBC */

#ifdef CONFIG_RTK_VOIP_AMR_NB
void AMR_NB_set_codec_mem(int state, int g726_rate)
{

}
#endif /* CONFIG_RTK_VOIP_AMR_NB */

#ifdef CONFIG_RTK_VOIP_T38
void T38_set_codec_mem(int state, int g726_rate)
{
	/* case CODEC_TYPE_T38: */

	set_common_imem();
}
#endif /* CONFIG_RTK_VOIP_T38 */

#ifdef CONFIG_RTK_VOIP_SPEEX_NB
void SPEEX_NB_set_codec_mem(int state, int g726_rate)
{

}
#endif /* CONFIG_RTK_VOIP_SPEEX_NB */

void set_codec_mem_to_common(int state, int g726_rate)
{
	set_common_imem();
}

int set_codec_mem(int type, int state, int g726_rate)
{
	const codec_type_desc_t *pCodecTypeDesc;

	pCodecTypeDesc = GetCodecTypeDesc( type );

	if( pCodecTypeDesc == NULL )
		return -1;

	/* 0 ~ NUM_OF_CODEC_TYPE_DESC-1 are valid */
	( *pCodecTypeDesc ->fnSetCodecMem )( state, g726_rate );

	return 0;
}


#ifdef CONFIG_RTK_VOIP_G7231
int set_g7231enc_imem()
{
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672)  && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) 
#ifndef CODEC_IMEM_SWAP_NEW
	long size_g7231enc = 0;
	long size_g7231 = 0;

	size_g7231enc = ((unsigned long)&__load_stop_IMEM_G7231ENC)-((unsigned long)&__load_start_IMEM_G7231ENC)+8;
	if(size_g7231enc<0)
		return -1;

	size_g7231enc = 64*(1+size_g7231enc/64);
#if 0
	memcpy((void*)&__imem_start, (void*)&__load_start_IMEM_G7231ENC, size_g7231enc);
#else
	memcpy64s((Word32*)&__imem_start, (Word32*)&__load_start_IMEM_G7231ENC, size_g7231enc>>1);
#endif

	if(imem_data != G7231_INSIDE)
	{
		size_g7231 = ((unsigned long)&__load_stop_IMEM_G7231)-((unsigned long)&__load_start_IMEM_G7231)+8;
		if(size_g7231<0)
			return -1;
		size_g7231 = 64*(1+size_g7231/64);
#if 0
		memcpy((void*)&__IMEM_G7231_START, (void*)&__load_start_IMEM_G7231, size_g7231);
#else
		memcpy64s((Word32*)&__IMEM_G7231_START, (Word32*)&__load_start_IMEM_G7231, size_g7231>>1);
#endif
		imem_data = G7231_INSIDE;
	}
#ifdef VOIP_CPU_CACHE_WRITE_BACK
	ICACHE_INV();
	DCACHE_WRITE_BACK_DIRTY();
#endif
        set_and_fill_IMEM(&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
#else	/*CODEC_IMEM_SWAP_NEW*/

	if(imem_data_internal!=G7231_INSIDE)
	{
		set_and_fill_IMEM(&__load_start_IMEM_G7231ENC,(unsigned long)&__load_start_IMEM_G7231ENC+(imem_size-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = G7231_INSIDE;
	}
	else
	{
		set_and_fill_IMEM(&__load_start_IMEM_G7231ENC,(unsigned long)&__load_start_IMEM_G7231ENC+(0x800-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
	}
#endif	/*CODEC_IMEM_SWAP_NEW*/
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
    #if defined( CONFIG_RTL8192SE ) && defined( CONFIG_DEFAULTS_KERNEL_2_6 )
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
    #else
	if(imem_data_internal!=SYSTEM_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)__imem_type,(unsigned long)__imem_type+(imem_size-1));
		imem_data_internal = SYSTEM_INSIDE;
	}
    #endif
#endif

	return 0;
}
#endif

#ifdef CONFIG_RTK_VOIP_G7231
int set_g7231dec_imem()
{

#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
#ifndef CODEC_IMEM_SWAP_NEW
	long size_g7231dec = 0;
	long size_g7231 = 0;

	size_g7231dec = ((unsigned long)&__load_stop_IMEM_G7231DEC)-((unsigned long)&__load_start_IMEM_G7231DEC)+8;
	if(size_g7231dec<0)
		return -1;
	size_g7231dec = 64*(1+size_g7231dec/64);
#if 0
	memcpy((void*)&__imem_start, (void*)&__load_start_IMEM_G7231DEC, size_g7231dec);
#else
	memcpy64s((Word32*)&__imem_start, (Word32*)&__load_start_IMEM_G7231DEC, size_g7231dec>>1);
#endif

	if(imem_data != G7231_INSIDE)
	{
		size_g7231 = ((unsigned long)&__load_stop_IMEM_G7231)-((unsigned long)&__load_start_IMEM_G7231)+8;
		if(size_g7231<0)
			return -1;
		size_g7231 = 64*(1+size_g7231/64);
#if 0
		memcpy((void*)&__IMEM_G7231_START, (void*)&__load_start_IMEM_G7231, size_g7231);
#else
		memcpy64s((Word32*)&__IMEM_G7231_START, (Word32*)&__load_start_IMEM_G7231, size_g7231>>1);
#endif
		imem_data = G7231_INSIDE;
	}
#ifdef VOIP_CPU_CACHE_WRITE_BACK
	ICACHE_INV();
	DCACHE_WRITE_BACK_DIRTY();
#endif
        set_and_fill_IMEM(&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
#else	/*CODEC_IMEM_SWAP_NEW*/
	if(imem_data_internal!=G7231_INSIDE)
	{
		set_and_fill_IMEM(&__load_start_IMEM_G7231DEC,(unsigned long)&__load_start_IMEM_G7231DEC+(imem_size-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = G7231_INSIDE;
	}
	else
	{
		set_and_fill_IMEM(&__load_start_IMEM_G7231DEC,(unsigned long)&__load_start_IMEM_G7231DEC+(0x400-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
	}

#endif	/*CODEC_IMEM_SWAP_NEW*/
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
    #if defined( CONFIG_RTL8192SE ) && defined( CONFIG_DEFAULTS_KERNEL_2_6 )
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
    #else
	if(imem_data_internal!=SYSTEM_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)__imem_type,(unsigned long)__imem_type+(imem_size-1));
		imem_data_internal = SYSTEM_INSIDE;
	}
    #endif
#endif
	return 0;
}
#endif

#ifdef CONFIG_RTK_VOIP_G729AB
int set_g729enc_imem()
{

#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
#ifndef CODEC_IMEM_SWAP_NEW
	long size_g729enc = 0;
	long size_g729 = 0;

	size_g729enc = ((unsigned long)&__load_stop_IMEM_G729ENC)-((unsigned long)&__load_start_IMEM_G729ENC)+8;
	if(size_g729enc<0)
		return -1;
	size_g729enc = 64*(1+size_g729enc/64);
#if 0
	memcpy((void*)&__imem_start, (void*)&__load_start_IMEM_G729ENC, size_g729enc);
#else
	memcpy64s((Word32*)&__imem_start, (Word32*)&__load_start_IMEM_G729ENC, size_g729enc>>1);
#endif

	if(imem_data != G729_INSIDE)
	{
		size_g729 = ((unsigned long)&__load_stop_IMEM_G729)-((unsigned long)&__load_start_IMEM_G729)+8;
		if(size_g729<0)
		{
			printk("set_g729enc_imem:size_g729=%d\n",size_g729);
			return -1;
		}
		size_g729 = 64*(1+size_g729/64);
#if 0
		memcpy((void*)&__IMEM_G729_START, (void*)&__load_start_IMEM_G729, size_g729);
#else
		memcpy64s((Word32*)&__IMEM_G729_START, (Word32*)&__load_start_IMEM_G729, size_g729>>1);
#endif
		imem_data = G729_INSIDE;
	}
#ifdef VOIP_CPU_CACHE_WRITE_BACK
	ICACHE_INV();
	DCACHE_WRITE_BACK_DIRTY();
#endif
        set_and_fill_IMEM(&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
#else	/*CODEC_IMEM_SWAP_NEW*/

	if(imem_data_internal!=G729_INSIDE)
	{
		set_and_fill_IMEM(&__load_start_IMEM_G729ENC,(unsigned long)&__load_start_IMEM_G729ENC+(imem_size-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = G729_INSIDE;
	}
	else
	{
		set_and_fill_IMEM(&__load_start_IMEM_G729ENC,(unsigned long)&__load_start_IMEM_G729ENC+(0x400-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
	}

#endif	/*CODEC_IMEM_SWAP_NEW*/
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
    #if defined( CONFIG_RTL8192SE ) && defined( CONFIG_DEFAULTS_KERNEL_2_6 )
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
    #else
	if(imem_data_internal!=SYSTEM_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)__imem_type,(unsigned long)__imem_type+(imem_size-1));
		imem_data_internal = SYSTEM_INSIDE;
	}
    #endif
#endif

	return 0;
}
#endif

#ifdef CONFIG_RTK_VOIP_G729AB
int set_g729dec_imem()
{

#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
#ifndef CODEC_IMEM_SWAP_NEW
	long size_g729dec = 0;
	long size_g729 = 0;

	size_g729dec = ((unsigned long)&__load_stop_IMEM_G729DEC)-((unsigned long)&__load_start_IMEM_G729DEC)+8;
	if(size_g729dec<0)
		return -1;
	size_g729dec = 64*(1+size_g729dec/64);
#if 0
	memcpy((void*)&__imem_start, (void*)&__load_start_IMEM_G729DEC, size_g729dec);
#else
	memcpy64s((Word32*)&__imem_start, (Word32*)&__load_start_IMEM_G729DEC, size_g729dec>>1);
#endif

	if(imem_data != G729_INSIDE)
	{
		size_g729 = ((unsigned long)&__load_stop_IMEM_G729)-((unsigned long)&__load_start_IMEM_G729)+8;
		if(size_g729<0)
		{
			printk("set_g729dec_imem:size_g729=%d\n",size_g729);
			return -1;
		}
		size_g729 = 64*(1+size_g729/64);
#if 0
		memcpy((void*)&__IMEM_G729_START, (void*)&__load_start_IMEM_G729, size_g729);
#else
		memcpy64s((Word32*)&__IMEM_G729_START, (Word32*)&__load_start_IMEM_G729, size_g729>>1);
#endif
		imem_data = G729_INSIDE;
	}
#ifdef VOIP_CPU_CACHE_WRITE_BACK
	ICACHE_INV();
	DCACHE_WRITE_BACK_DIRTY();
#endif
        set_and_fill_IMEM(&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
#else	/*CODEC_IMEM_SWAP_NEW*/

	if(imem_data_internal!=G729_INSIDE)
	{
		set_and_fill_IMEM(&__load_start_IMEM_G729DEC,(unsigned long)&__load_start_IMEM_G729DEC+(imem_size-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = G729_INSIDE;
	}
	else
	{
		set_and_fill_IMEM(&__load_start_IMEM_G729DEC,(unsigned long)&__load_start_IMEM_G729DEC+(0x400-1));
		set_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
	}

#endif	/*CODEC_IMEM_SWAP_NEW*/
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
    #if defined( CONFIG_RTL8192SE ) && defined( CONFIG_DEFAULTS_KERNEL_2_6 )
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
    #else
	if(imem_data_internal!=SYSTEM_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)__imem_type,(unsigned long)__imem_type+(imem_size-1));
		imem_data_internal = SYSTEM_INSIDE;
	}
    #endif
#endif
	return 0;
}
#endif


static int set_common_imem()
{
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
        set_and_fill_IMEM(&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
        imem_data_internal = COMMON_INSIDE;
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
	if(imem_data_internal!=COMMON_INSIDE)
	{
		set_and_fill_IMEM((unsigned long)&__imem_start,(unsigned long)&__imem_start+(imem_size-1));
		imem_data_internal = COMMON_INSIDE;
	}
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
    #if defined( CONFIG_RTL8192SE ) && defined( CONFIG_DEFAULTS_KERNEL_2_6 )
	/* for non g.729/g.723 codec don't change IMEM content */
    #else
	if(imem_data_internal!=SYSTEM_INSIDE)
	{
	#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
		unsigned long flags;
		unsigned long size_in_word; /* word size of IMEM code*/
		extern unsigned long __iram, __iram_end;
		size_in_word = ((((unsigned long)&__iram_end - (unsigned long)&__iram ) + 3) & (~3))>>2;
		if (size_in_word >=0x1000)
			size_in_word = 0x1000;
		else
			size_in_word = size_in_word & 0xfff;
		save_flags(flags);
		cli();
		set_IMEM((unsigned long)&__iram,(unsigned long)&__iram+(imem_size-1));
		*(volatile unsigned long *) IM_CTL = IM_CTL_RSET;
		*(volatile unsigned long *) IM_CTL = 0x0;
		*(volatile unsigned long *) IM_IMEM_SA = 0;
		//*(volatile unsigned long *) IM_EXTM_SA = (unsigned long)&__iram & 0xfffffff; /* convert to physical address */
		*(volatile unsigned long *) IM_EXTM_SA = Virtual2Physical( &__iram ); /* convert to physical address */
		*(volatile unsigned long *) IM_CTL = IM_CTL_START | IM_CTL_SDRAM2IMEM | size_in_word;
		*(volatile unsigned long *) IM_CTL;
		while(*(volatile unsigned long *) IM_CTL & IM_CTL_START);
		restore_flags(flags);
	#else
		set_and_fill_IMEM((unsigned long)__imem_type,(unsigned long)__imem_type+(imem_size-1));
	#endif
		imem_data_internal = SYSTEM_INSIDE;
	}
    #endif
#endif

	return 0;
}

int init_codec_imem(void)
{
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
	long size_g729 = 0;
	long size_g7231 = 0;
	long size_common = 0;
	long size_common_and_g729 =0;

	unsigned long G729enc_729common_load_start =0;
	unsigned long G729enc_common_load_start =0;
	unsigned long G729dec_729common_load_start =0;
	unsigned long G729dec_common_load_start =0;

	unsigned long G7231enc_7231common_load_start =0;
	unsigned long G7231enc_common_load_start =0;
	unsigned long G7231dec_7231common_load_start =0;
	unsigned long G7231dec_common_load_start =0;

	size_common = ((unsigned long)&__imem_common_end)-((unsigned long)&__imem_common_start)+8;
	size_common = 64*(1+size_common/64);

	if (size_common > (0x1000 -  ((unsigned long)&__imem_common_start & 0xfff) ))
	{
		printk("++size_common=%x,&__imem_common_start=%x\n",size_common,&__imem_common_start);
		size_common = 0x1000 -  ((unsigned long)&__imem_common_start & 0xfff) ;
		printk("size_common new=%x\n",size_common);
	}
	else
	{
		printk("--size_common=%x,&__imem_common_start=%x\n",size_common,&__imem_common_start);
		size_common = 0x1000 -  ((unsigned long)&__imem_common_start & 0xfff) ;
		printk("size_common new=%x\n",size_common);	
	}

	size_g729 = ((unsigned long)&__load_stop_IMEM_G729)-((unsigned long)&__load_start_IMEM_G729)+8;

	size_g729 = 64*(1+size_g729/64);

	//G729enc_729common_load_start = (unsigned long)&__IMEM_G729_START + 0x4000 ;
	G729dec_729common_load_start = (unsigned long)&__IMEM_G729_START + 0x8000 ;

	G729enc_common_load_start = (unsigned long)&__imem_common_start + 0x4000;
	G729dec_common_load_start = (unsigned long)&__imem_common_start + 0x8000;


	//memcpy64s((Word32*)G729enc_729common_load_start, (Word32*)&__load_start_IMEM_G729, size_g729>>1);
	memcpy64s((Word32*)G729dec_729common_load_start, (Word32*)&__load_start_IMEM_G729, size_g729>>1);

	memcpy64s((Word32*)G729enc_common_load_start, (Word32*)&__imem_common_start, size_common>>1);
	memcpy64s((Word32*)G729dec_common_load_start, (Word32*)&__imem_common_start, size_common>>1);

	size_g7231 = ((unsigned long)&__load_stop_IMEM_G7231)-((unsigned long)&__load_start_IMEM_G7231)+8;
	if(size_g7231<0)
		return -1;
	size_g7231 = 64*(1+size_g7231/64);

	//G7231enc_7231common_load_start = (unsigned long)&__IMEM_G7231_START + 0xC000 ;
	G7231dec_7231common_load_start = (unsigned long)&__IMEM_G7231_START + 0x10000 ;

	G7231enc_common_load_start = (unsigned long)&__imem_common_start + 0xC000 ;
	G7231dec_common_load_start = (unsigned long)&__imem_common_start + 0x10000 ;


	//memcpy64s((Word32*)G7231enc_7231common_load_start, (Word32*)&__load_start_IMEM_G7231, size_g7231>>1);
	memcpy64s((Word32*)G7231dec_7231common_load_start, (Word32*)&__load_start_IMEM_G7231, size_g7231>>1);

	memcpy64s((Word32*)G7231enc_common_load_start, (Word32*)&__imem_common_start, size_common>>1);
	memcpy64s((Word32*)G7231dec_common_load_start, (Word32*)&__imem_common_start, size_common>>1);
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
	unsigned long flags;
	extern unsigned long __iram;
	save_flags(flags);
	cli();
	set_and_fill_IMEM((unsigned long)&__iram,(unsigned long)&__iram+(0x4000-1));
	restore_flags(flags);
	imem_data_internal = SYSTEM_INSIDE;
#endif
	printk("IMEM_init_OK\n");
	return 0;

}

