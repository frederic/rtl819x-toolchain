#include <linux/kernel.h>

#if 1
#include "voip_types.h"
#else
#define int8 char
#define uint32 unsigned long
#define int32 long
/*
typedef struct {
	uint32	lo;
	uint32	hi;
}uint64;
*/
#define uint64	unsigned long long
#define TRUE 1
#define FALSE 0
#endif

//#define printk	printf

void startCOP3Counters(int32 countInst){
	/* counter control modes:
		0x10	cycles
		0x11	new inst fetches
		0x12	inst fetch cache misses
		0x13	inst fetch miss busy cycles
		0x14	data store inst
		0x15	data load inst
		0x16	load or store inst
		0x1a	load or store cache misses
		0x1b	load or store miss busy cycles
		*/
	uint32 cntmode;

	if(countInst == TRUE)
		cntmode = 0x131b1110;//0x13121110;
	else
		cntmode = 0x1b1a1610;
	
	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9,$12\n\t"
		"nop\n\t"
		"la $10,0x80000000\n\t"
		"or $9,$10\n\t"
		"mtc0 $9,$12\n\t"
		"nop\n\t"
		"nop\n\t"
		/* stop counters */
		"ctc3 $0,$0\n\t"
		/* clear counters */
		"mtc3 $0,$8\n\t"
		"mtc3 $0,$9\n\t"
		"mtc3 $0,$10\n\t"
		"mtc3 $0,$11\n\t"
		"mtc3 $0,$12\n\t"
		"mtc3 $0,$13\n\t"
		"mtc3 $0,$14\n\t"
		"mtc3 $0,$15\n\t"
		/* set counter controls */
		"ctc3 %0,$0"
		: /* no output */
		: "r" (cntmode)
		);
}


uint32 stopCOP3Counters(void){
	uint32 cntmode;
	uint32 cnt0h, cnt0l, cnt1h, cnt1l, cnt2h, cnt2l, cnt3h, cnt3l;
	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9,$12\n\t"
		"nop\n\t"
		"la $10,0x80000000\n\t"
		"or $9,$10\n\t"
		"mtc0 $9,$12\n\t"
		"nop\n\t"
		"nop\n\t"
		/* get counter controls */
		"cfc3 %0,$0\n\t"
		/* stop counters */
		"ctc3 $0,$0\n\t"
		/* save counter contents */
		"mfc3 %1,$9\n\t"
		"mfc3 %2,$8\n\t"
		"mfc3 %3,$11\n\t"
		"mfc3 %4,$10\n\t"
		"mfc3 %5,$13\n\t"
		"mfc3 %6,$12\n\t"
		"mfc3 %7,$15\n\t"
		"mfc3 %8,$14\n\t"
		"nop\n\t"
		"nop\n\t"
		: "=r" (cntmode), "=r" (cnt0h), "=r" (cnt0l), "=r" (cnt1h), "=r" (cnt1l), "=r" (cnt2h), "=r" (cnt2l), "=r" (cnt3h), "=r" (cnt3l)
		: /* no input */
		);
		/*
	if(cntmode == 0x13121110){
		printk("COP3 counter for instruction access\n");
		printk("%10d cycles\n", cnt0l);
		printk("%10d new inst fetches\n", cnt1l);
		printk("%10d inst fetch cache misses\n", cnt2l);
		printk("%10d inst fetch miss busy cycles\n", cnt3l);
		}
		*/
	if(cntmode == 0x131b1110){
		printk("COP3 counter access\n");
		printk("%10d cycles\n", cnt0l);
		printk("%10d new inst fetches\n", cnt1l);
		printk("%10d load or store miss busy cycles\n", cnt2l);
		printk("%10d inst fetch miss busy cycles\n", cnt3l);
		}
	else{
		printk("COP3 counter for data access\n");
		printk("%10d cycles\n", cnt0l);
		printk("%10d load or store inst\n", cnt1l);
		printk("%10d load or store cache misses\n", cnt2l);
		printk("%10d load or store miss busy cycles\n", cnt3l);
		}
	return cnt0l;
}

static int8 cop3InstMode;
static uint64 cop3Cycles, cop3InstFetches, cop3InstCacheMisses, cop3InstcacheMissBusyCycles;
static uint64 cop3DataInst, cop3DataCacheMisses, cop3DataCacheMissCycles;
static uint64 max_cyc;

void clearCOP3Counters(void){
	cop3InstMode = 0;
	cop3Cycles = 0;
	cop3InstFetches = 0;
	cop3InstCacheMisses = 0;
	cop3InstcacheMissBusyCycles = 0;
	cop3DataInst = 0;
	cop3DataCacheMisses = 0;
	cop3DataCacheMissCycles = 0;
	max_cyc = 0;
}

void pauseCOP3Counters(void){
	uint32 cntmode;
	uint32 cnt0h, cnt0l, cnt1h, cnt1l, cnt2h, cnt2l, cnt3h, cnt3l;
	__asm__ __volatile__ (
		/* update status register CU[3] usable */
		"mfc0 $9,$12\n\t"
		"nop\n\t"
		"la $10,0x80000000\n\t"
		"or $9,$10\n\t"
		"mtc0 $9,$12\n\t"
		"nop\n\t"
		"nop\n\t"
		/* get counter controls */
		"cfc3 %0,$0\n\t"
		/* stop counters */
		"ctc3 $0,$0\n\t"
		/* save counter contents */
		"mfc3 %1,$9\n\t"
		"mfc3 %2,$8\n\t"
		"mfc3 %3,$11\n\t"
		"mfc3 %4,$10\n\t"
		"mfc3 %5,$13\n\t"
		"mfc3 %6,$12\n\t"
		"mfc3 %7,$15\n\t"
		"mfc3 %8,$14\n\t"
		"nop\n\t"
		"nop\n\t"
		: "=r" (cntmode), "=r" (cnt0h), "=r" (cnt0l), "=r" (cnt1h), "=r" (cnt1l), "=r" (cnt2h), "=r" (cnt2l), "=r" (cnt3h), "=r" (cnt3l)
		: /* no input */
		);
/*
	if(cntmode == 0x13121110){
		cop3InstMode = 1;
		cop3Cycles += cnt0l;
		cop3InstFetches += cnt1l;
		cop3InstCacheMisses = cnt2l;
		cop3InstcacheMissBusyCycles = cnt3l;
		}
*/
	if(cntmode == 0x131b1110){
		cop3InstMode = 1;
		cop3Cycles += cnt0l;
		cop3InstFetches += cnt1l;
		cop3InstCacheMisses = cnt2l;
		cop3InstcacheMissBusyCycles = cnt3l;
		if(cnt0l>max_cyc)
			max_cyc = cnt0l;
		}
	else{
		cop3InstMode = 0;
		cop3Cycles += cnt0l;
		cop3DataInst += cnt1l;
		cop3DataCacheMisses += cnt2l;
		cop3DataCacheMissCycles += cnt3l;
		}
}

void DisplayCOP3Counters(void){
#if 1
#define _displayy(val, str) if(val > 0xffffffffUL)\
	printk("0x%x%08x %s\n", (uint32) (val >> 32), (uint32) (val & 0xffffffffUL), str);\
else\
	printk("%d %s\n", (uint32) val, str);

	if(cop3InstMode == 1){
#if 0		
		printk("COP3 counter for instruction access\n");
		_displayy(cop3Cycles, "cycles");
		_displayy(cop3InstFetches, "new instruction fetches");
		//_displayy(cop3InstCacheMisses,"instruction fetch cache misses");
		_displayy(cop3InstCacheMisses,"load or store miss busy cycle");
		_displayy(cop3InstcacheMissBusyCycles, "instruction fetch miss busy cycles");
		_displayy(max_cyc, "max cycles");
#else
		_displayy(max_cyc, "mc");
#endif
		}
	else{
		printk("COP3 counter for data access\n");
		_displayy(cop3Cycles, "cycles");
		_displayy(cop3DataInst, "load or store instruction");
		_displayy(cop3DataCacheMisses,"load or store cache misses");
		_displayy(cop3DataCacheMissCycles, "load or store miss busy cycles");
		}
#endif
}
