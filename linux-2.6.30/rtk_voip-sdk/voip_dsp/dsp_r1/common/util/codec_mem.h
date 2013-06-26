#ifndef _CODEC_MEM_H_
#define _CODEC_MEM_H_

#include "dmem_stack.h"
#include <linux/types.h>

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define IMEM_SIZE	0x01000
#define DMEM_SIZE	0x01000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#define IMEM_SIZE	0x02000
#define DMEM_SIZE	0x01000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
#define IMEM_SIZE	0x01000
#define DMEM_SIZE	0x01000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
#define IMEM_SIZE       	0x01000
#define DMEM_SIZE		0x01000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676
#define IMEM_SIZE       	0x01000
#define DMEM_SIZE		0x01000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM865xC
#define IMEM_SIZE	0x04000
#define DMEM_SIZE	0x02000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
#define IMEM_SIZE	0x04000
#define DMEM_SIZE	0x02000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
#define IMEM_SIZE	0x01000
#define DMEM_SIZE	0x01000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxD
#define IMEM_SIZE	0x01000
#define DMEM_SIZE	0x01000
#endif

extern unsigned long __imem_start;
extern unsigned long __dmem_start;

/* new imem method */
extern unsigned long __imem_common_start;
extern unsigned long __imem_common_end;

/* g729 dmem */
extern unsigned long __g729_dmem_start;

/* g729 imem */
extern unsigned long __load_start_IMEM_G729ENC;
extern unsigned long __load_stop_IMEM_G729ENC;
extern unsigned long __load_start_IMEM_G729DEC;
extern unsigned long __load_stop_IMEM_G729DEC;
extern unsigned long __load_start_IMEM_G729;
extern unsigned long __load_stop_IMEM_G729;
extern unsigned long __IMEM_G729_START;


/* g723.1 imem */
extern unsigned long __load_start_IMEM_G7231ENC;
extern unsigned long __load_stop_IMEM_G7231ENC;
extern unsigned long __load_start_IMEM_G7231DEC;
extern unsigned long __load_stop_IMEM_G7231DEC;
extern unsigned long __load_start_IMEM_G7231;
extern unsigned long __load_stop_IMEM_G7231;
extern unsigned long __IMEM_G7231_START;
/* g723.1 dmem */
extern unsigned long __g7231_dmem_start;

/* iLBC imem */
extern unsigned long __ILBCENC_IMEM_start;
extern unsigned long __ILBCDEC_IMEM_start;

/* g726 dmem */
extern unsigned long __g726_dmem_start;

/* gsm-fr dmem */
extern const unsigned long __gsmfr_dmem_start;

/* lec dmem */
extern unsigned long __lec_dmem_start;

extern int set_codec_mem(int type, int state, int g726_rate);
extern void set_DMEM(unsigned long start, unsigned long size);
extern void set_IMEM(unsigned long start, unsigned long size);

extern void set_and_fill_IMEM(unsigned long start, unsigned long end);

/* g.729 stack size */
#define G729_DUMMY_SSIZE 1560
#if DMEM_SIZE == 0x2000 // use 8K DMEM ok. 4K for Stack + 4K for variables
#define G729_DMEM_SSIZE 64
#define G729ENC_SSIZE (64-64)
#define G729DEC_SSIZE (64-64)
#else // if codec can be interrupted, DMEM section must be at odd page. e.g. 0xXXX1000, 0xXXX3000
#define G729_DMEM_SSIZE 488//488
#define G729ENC_SSIZE 392 /* 456*4 bytes */
#define G729DEC_SSIZE 392 /* 224*4 bytes */
#endif
#define G729_BSS_SIZE 518	/* 518*4 bytes */
extern int16_t* g729_TmpVct;


extern unsigned long g729_dmem_stack[G729_DMEM_SSIZE];
extern unsigned long g729_orig_sp;
extern unsigned long g729_dmem_sp;

/* g.7231 stack size */
//#define G7231_DUMMY_SSIZE 1728
#if DMEM_SIZE == 0x2000 // use 8K DMEM ok.
#define G7231_DMEM_SSIZE 64
#define G7231ENC_SSIZE (64-64)
#define G7231DEC_SSIZE (64-64)
#else // if codec can be interrupted, DMEM section must be at odd page. e.g. 0xXXX1000, 0xXXX3000
#define G7231_DMEM_SSIZE 288
#define G7231ENC_SSIZE 256
#define G7231DEC_SSIZE 256
#endif

extern unsigned long g7231_dmem_stack[G7231_DMEM_SSIZE];
extern unsigned long g7231_orig_sp;
extern unsigned long g7231_dmem_sp;

/* g.726 stack size */
#define G726_DUMMY_SSIZE 1328
#define G726_DMEM_SSIZE 720//360
#define G726ENC_SSIZE 700//320 /* 320*4 bytes */
#define G726DEC_SSIZE 700//320 /* 320*4 bytes */
#define G726_DATA_SIZE	80	/* 80*4 bytes */
extern int16_t* g726_TmpVct;

extern unsigned long g726_dmem_stack[G726_DMEM_SSIZE];
extern unsigned long g726_orig_sp;
extern unsigned long g726_dmem_sp;

/* gsmfr stack size */
#define GSMFR_DUMMY_SSIZE 1328
#define GSMFR_DMEM_SSIZE 720//360	/* the size need 2 word align, or 8btye align */
#define GSMFRENC_SSIZE 700//320 /* 320*4 bytes */
#define GSMFRDEC_SSIZE 700//320 /* 320*4 bytes */
#define GSMFR_BSS_SIZE 298	/* 298*4 bytes */
extern int16_t* gsmfr_TmpVct;

extern unsigned long gsmfr_dmem_stack[GSMFR_DMEM_SSIZE];
extern unsigned long gsmfr_orig_sp;
extern unsigned long gsmfr_dmem_sp;

/* iLBC stack size */
#define ILBC_DUMMY_SSIZE 1024
#if DMEM_SIZE == 0x2000 // use 8K DMEM ok.
#define ILBC_DMEM_SSIZE 1024
#define ILBCENC_SSIZE (1024-64)
#define ILBCDEC_SSIZE (1024-64)
#else // if codec can be interrupted, DMEM section must be at odd page. e.g. 0xXXX1000, 0xXXX3000
#define ILBC_DMEM_SSIZE 1000
#define ILBCENC_SSIZE 1000-64
#define ILBCDEC_SSIZE 1000-64
#endif
//extern unsigned long iLBC_dmem_stack[ILBC_DMEM_SSIZE];
extern unsigned long iLBC_orig_sp;
extern unsigned long iLBC_dmem_sp;
extern unsigned long codec_dmem_area[DMEM_SIZE/4] ;
extern unsigned long __codec_dmem_start;
extern unsigned long __codec_dmem_4k_start;

/* lec stack size */
#define LEC_DUMMY_SSIZE 1948//1984
#define LEC_DMEM_SSIZE 100//64
#define LEC_SSIZE 920

/* G711WB */
#define G711WB_DMEM_SSIZE 480	/* the size need 2 word align, or 8btye align */
extern int16_t* g711wb_TmpVct;

extern unsigned long lec_dmem_stack[LEC_DMEM_SSIZE];
extern unsigned long lec_orig_sp;
extern unsigned long lec_dmem_sp;

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
#define IMEMDMA_BASE	0xb800b800

#define IM_IMEM_SA	(IMEMDMA_BASE + 0x00)	/*IMEM start address*/
#define IM_EXTM_SA	(IMEMDMA_BASE + 0x04)	/*External memory start address*/
#define IM_CTL		(IMEMDMA_BASE + 0x08)	/*IMEM DMA control*/

/* IM_CTL */
#define IM_CTL_RSET	(1<<15)
#define IM_CTL_START	(1<<14)
#define IM_CTL_IMEM2SDRAM	(0)
#define IM_CTL_SDRAM2IMEM	(1<<13)
#endif

#endif /* _CODEC_MEM_H_ */
