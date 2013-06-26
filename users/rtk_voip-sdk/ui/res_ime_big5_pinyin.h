#ifndef __RES_IME_BIG5_PINYIN_H__
#define __RES_IME_BIG5_PINYIN_H__

#define PY_SUPRA_INDEX_ENTRY		( 26 * 10 )		/* Level 1: (25) a~z; Level 2: 10 */
#define PY_SUPRA_INDEX_BOUNDARY		155
extern const unsigned char pySupraIndex[];

#define PY_6CODES_ENTRIES			3
typedef struct {
	unsigned char code1;
	unsigned char code2;
	unsigned char code3to5;
	unsigned char code6;
	unsigned short woStart;
	unsigned short woEnd;
} py6CodesEntry_t;
extern const py6CodesEntry_t py6CodesEntries[];

#define PY_CODES_SPELL_KINDS		428				/* 431 - 3 (6 codes) */
extern const unsigned char pyCodes3to5[];
extern const unsigned char pyWordOffset[];
#define PY_WORD_OFFSET_BOUNDARY_NUM	3
extern const unsigned short pyWordOffsetBoundary[];

#define PY_WORDS_LIST_NUM			14083
extern const unsigned char pyWordsList[];


#endif /* __RES_IME_BIG5_PINYIN_H__ */
