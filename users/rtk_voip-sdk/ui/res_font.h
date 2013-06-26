#ifndef __RES_FONT_H__
#define __RES_FONT_H__

typedef unsigned char font_flag;
#define FONT_FLAG_WIDTH_MASK		0x01	// bit 0: font width is fixed or variable
#define FONT_FLAG_WIDTH_FIXED		0x00	// bit 0 = 0: fixed width 
#define FONT_FLAG_WIDTH_VARIABLE	0x01	// bit 0 = 1: variable width 
#define FONT_FLAG_ORIENTED_MASK		0x06	// bit 1-2: font orientation maps to LCD ORIENTED
#define FONT_FLAG_ORIENTED_ROW		0x02	// bit 1-2 = 1: row oriented 
#define FONT_FLAG_ORIENTED_COL		0x04	// bit 1-2 = 2: column oriented type 1 
#define FONT_FLAG_ORIENTED_COL2		0x06	// bit 1-2 = 3: column oriented type 2 
#define FONT_FLAG_MSB_FIRST			0x08	// bit 3 = 1, MSB first; bit 3 = 0, LSB first 
#define FONT_FLAG_LANG_MASK			0xF0	// bit 4-7: font language  
#define FONT_FLAG_LANG_ENG			0x00	// bit 4-7 = 0: English 
#define FONT_FLAG_LANG_BIG5			0x10	// bit 4-7 = 1: Traditional Chinese (Big5)
#define FONT_FLAG_LANG_GB2312		0x20	// bit 4-7 = 2: Simplified Chinese (GB)

typedef unsigned char font_flag2;
#define FONT_FLAG2_xxx				0x01

typedef unsigned char big5_segment;
#define BIG5_SEG0_USER1				0x01	/* 0x8140-0xA0FE (5024) 保留給使用者自定義字元（造字區） */
#define BIG5_SEG1_SYMBOL			0x02	/* 0xA140-0xA3BF (408)  標點符號、希臘字母及特殊符號，包括在0xA259-0xA261，安放了雙音節度量衡單位用字：兙兛兞兝兡兣嗧瓩糎。 */
#define BIG5_SEG2_RESERVED			0x04	/* 0xA3C0-0xA3FE (63)   保留。此區沒有開放作造字區用。 */
#define BIG5_SEG3_COMMON1			0x08	/* 0xA440-0xC67E (5401) 常用漢字，先按筆劃再按部首排序。 */
#define BIG5_SEG4_USER2				0x10	/* 0xC6A1-0xC8FE (408)  保留給使用者自定義字元（造字區） */
#define BIG5_SEG5_COMMON2			0x20	/* 0xC940-0xF9D5 (7652) 次常用漢字，亦是先按筆劃再按部首排序。 */
#define BIG5_SEG6_USER3				0x40	/* 0xF9D6-0xFEFE (826)  保留給使用者自定義字元（造字區） */

typedef unsigned char gb2312_segment;
#define GB2312_SEG0_SYMBOL			0x01	/* 0xA1A1-0xA9FE 01-09區為特殊符號 */
#define GB2312_SEG1_UNUSED1			0x02	/* 0xAAA1-0xAFFE 10-15區未有編碼 */
#define GB2312_SEG2_COMMON1			0x04	/* 0xB0A1-0xD7FE 16-55區為一級漢字，按拼音排序 */
#define GB2312_SEG3_COMMON2			0x08	/* 0xD8A1-0xF7FE 56-87區為二級漢字，按部首/筆畫排序 */
#define GB2312_SEG4_UNUSED2			0x10	/* 0xF8A1-0xFEFE 88-94區則未有編碼 */


typedef unsigned short nFontOffset_t;

#pragma pack( 1 )

typedef struct {
	unsigned char width;
	unsigned char height;
	unsigned char bytes;		// bytes per word 
	unsigned char reserved1;
	unsigned short words;		// number of words 
	font_flag flag;
	font_flag2 flag2;
	union {
		struct {	// English 
			unsigned char start_ascii;	// English only 
			unsigned char bitmap[ 1 ];	// font bitmap data (words x bytes)
		} eng;
		struct {	// English 
			unsigned char start_ascii;	// English only 
			unsigned char padding_byte;
			nFontOffset_t offset[ 1 ];	// font offset data (words x 2)
			// unsigned char bitmap[ 1 ];	// font bitmap data (words x bytes)
		} eng_var;
		struct {	// Big5 
			big5_segment segment;
			unsigned char padding_byte;
			unsigned char bitmap[ 1 ];
		} big5;
		struct {	// GB2312
			gb2312_segment segment;
			unsigned char padding_byte;
			unsigned char bitmap[ 1 ];
		} gb2312;
	};
} font_t;

#pragma pack()

#ifdef _TEST_MODE
#define FONT_WORDS( n )		( unsigned char )( ( n ) & 0xFF ), ( unsigned char )( ( n ) / 256 ) 
#define FONT_OFFSET( n )	( unsigned char )( ( n ) & 0xFF ), ( unsigned char )( ( n ) / 256 ) 
#else
#define FONT_WORDS( n )		( unsigned char )( ( n ) / 256 ), ( unsigned char )( ( n ) & 0xFF )
#define FONT_OFFSET( n )	( unsigned char )( ( n ) / 256 ), ( unsigned char )( ( n ) & 0xFF )
#endif

/* Resource of font */
extern const unsigned char ASCII_TAB_Eng_5x8_col[];
extern const unsigned char ASCII_TAB_Eng_8x16_col2[];
extern const unsigned char ASCII_TAB_Eng_Nx16_col2[];
extern const unsigned char ASCII_TAB_Eng_8x16_row[];
extern const unsigned char Chinese_big5_16x16_col2[];
extern const unsigned char Chinese_big5_14x14_col2[];
extern const unsigned char Chinese_gb2312_16x16_col2[];
extern const unsigned char Chinese_gb2312_14x14_col2[];


#endif /* __RES_FONT_H__ */
