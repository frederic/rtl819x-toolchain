#ifndef __RES_IMAGE_H__
#define __RES_IMAGE_H__

#define IMAGE_FLAGS_ORIENTED_MASK	0x0003	/* bit 0-1 */
#define IMAGE_FLAGS_ORIENTED_ROW	0x0001	/* bit 0-1=1 */
#define IMAGE_FLAGS_ORIENTED_COL	0x0002	/* bit 0-1=2 */
#define IMAGE_FLAGS_ORIENTED_COL2	0x0003	/* bit 0-1=3 */
#define IAMGE_FLAGS_BPP_MASK		0x00F0	/* bit 4-7 */
#define IAMGE_FLAGS_1BPP			0x0000	/* bit 4-7=0 */

#ifdef _TEST_MODE
#define M_IMAGE_FLAGS_LIST( x )			( ( x ) & 0x00FF ), ( ( ( x ) & 0xFF00 ) >> 8 )
#else
#define M_IMAGE_FLAGS_LIST( x )			( ( ( x ) & 0xFF00 ) >> 8 ), ( ( x ) & 0x00FF )
#endif

#pragma pack( 1 )

typedef struct image_s {
	unsigned char width;
	unsigned char height;
	unsigned short flags;
	unsigned char data[ 1 ];
} image_t;

#pragma pack()

/* image for IME */
extern const unsigned char Image_ime_indicator_English[];
extern const unsigned char Image_ime_indicator_PinYin[];
extern const unsigned char Image_ime_indicator_Phonetic[];
extern const unsigned char Image_ime_indicator_WuBiHua[];
extern const unsigned char Image_ime_indicator_WuBiHua_root1[];
extern const unsigned char Image_ime_indicator_WuBiHua_root2[];
extern const unsigned char Image_ime_indicator_WuBiHua_root3[];
extern const unsigned char Image_ime_indicator_WuBiHua_root4[];
extern const unsigned char Image_ime_indicator_WuBiHua_root5[];
extern const unsigned char Image_ime_indicator_WuBiHua_root6[];

#endif /* __RES_IMAGE_H__ */
