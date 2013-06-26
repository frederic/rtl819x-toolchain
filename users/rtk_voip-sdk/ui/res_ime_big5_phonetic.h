#ifndef __RES_IME_BIG5_PHONETIC_H__
#define __RES_IME_BIG5_PHONETIC_H__

#define PHONETIC_SUPRA_INDEX_ENTRY			37		/* One level */
#define PHONETIC_SUPRA_INDEX_BOUNDARY		13
extern const unsigned char phoneticSupraIndex[];

#define PHONETIC_CODES_SPELL_KINDS			424
extern const unsigned char phoneticCodes2to3[];
extern const unsigned char phoneticWordOffset[];
#define PHONETIC_WORD_OFFSET_BOUNDARY_NUM	3
extern const unsigned short phoneticWordOffsetBoundary[];

#define PHONETIC_WORDS_LIST_NUM				14144
extern const unsigned char phoneticWordsList[];

#endif /* __RES_IME_BIG5_PHONETIC_H__ */
