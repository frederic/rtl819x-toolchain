
//
// cp3.h
//

#ifndef __CP3_H__
#define __CP3_H__

// control for cp3 counters
//
#define CP3_COUNT_STOP					0
#define CP3_COUNT_CYCLE					0x10
#define CP3_COUNT_NEW_INST_FETCH		0x11
#define CP3_COUNT_INST_CACHE_MISS		0x12
#define CP3_COUNT_INST_CACHE_MISS_BUSY	0x13
#define CP3_COUNT_DATA_STORE			0x14
#define CP3_COUNT_DATA_LOAD				0x15
#define CP3_COUNT_LOAD_STORE			0x16
#define CP3_COUNT_LOAD_STORE_MISS		0x1a
#define CP3_COUNT_LOAD_STORE_MISS_BUSY	0x1b

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CP3_COUNTER	10
typedef struct {
	int Mask;
	int SysCycle;
	int InstCount;
	int ICacheCycle;
	int DCacheCycle;
}cp3_counter_type;

extern cp3_counter_type cp3_counter[MAX_CP3_COUNTER];
extern void cp3_init(void);
extern void cp3_start(void);
extern void cp3_stop(void);
extern void cp3_set_counter(int counter, unsigned int control_bits);
extern unsigned int cp3_get_counter_high(int counter);
extern unsigned int cp3_get_counter_low(int counter);
extern void cp3_set_iram_addr(void* base_addr, void* top_addr);
extern void cp3_set_dram_addr_8601(void* base_addr, void* top_addr);
extern void cp3_set_dram_addr_8181(void* base_addr, void* top_addr);

#ifdef __cplusplus
}
#endif

#endif	// _CP3_H_
