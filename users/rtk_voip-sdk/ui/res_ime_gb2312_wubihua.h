#ifndef __RES_IME_GB2312_WUBIHUA_H__
#define __RES_IME_GB2312_WUBIHUA_H__

#define WBH_SUPRA_INDEX_ENTRY			( 5 * 6 * 6 )	/* Level 1: 5; Level 2: 6 (0~5), Level 3: 6 (0~5) */
#define WBH_SUPRA_INDEX_BOUNDARY_NUM	4
extern const unsigned char wbhSupraIndex[];
extern const unsigned short wbhSupraIndexBoundary[];

#define WBH_CODES_SPELL_KINDS			1233
extern const unsigned char wbhCodes4to5[];
extern const unsigned char wbhWordOffset[];
#define WBH_WORD_OFFSET_BOUNDARY_NUM	1
extern const unsigned short wbhWordOffsetBoundary[];

#define WBH_WORDS_LIST_NUM				6763
extern const unsigned char wbhWordsList[];


#endif /* __RES_IME_GB2312_WUBIHUA_H__ */
