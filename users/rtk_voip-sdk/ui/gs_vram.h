#ifndef __GS_VRAM_H__
#define __GS_VRAM_H__

#include "gs_lib.h"
#include "ioctl_lcd.h"

/* Initialize VRam */
extern void GS_InitializeVRam( void );

/* Terminate VRam */
extern void GS_TerminateVRam( void );

/* Write VRam data to device */
extern void GS_VRamToLcdDevice( const rect_t *pInvalidRect );

#ifdef _TEXT_MODE
	/* ========================================================================== */
	/* Text mode vRam */
	typedef struct vRam_s {
		unsigned char ch[ VRAM_WIDTH * VRAM_HEIGHT ];
	} vRam_t;
#else
	/* ========================================================================== */
	/* Graphic mode vRam */
  #if LCD_BPP == 1
    #ifdef LCD_COL_ORIENTED
		/* ========================================================================== */
		/* Column oriented LCD */
		#define VRAM_HEIGHT_BYTES	( VRAM_HEIGHT / 8 )
		#define VRAM_TOTAL_BYTES	( VRAM_WIDTH * VRAM_HEIGHT_BYTES )
      #if VRAM_HEIGHT_BYTES == 2
		typedef unsigned short cols_t;
      #elif VRAM_HEIGHT_BYTES == 4
		typedef unsigned long cols_t;
      #else
        #error "VRAM_HEIGHT_BYTES??"
      #endif

		typedef union vRam_s {
			unsigned char pixels[ VRAM_TOTAL_BYTES ];
			cols_t cols[ VRAM_WIDTH ];
		} vRam_t;
    #elif defined( LCD_COL2_ORIENTED )
		/* ========================================================================== */
		/* Column oriented LCD (type 2) */
		#define VRAM_HEIGHT_BYTES	( ( VRAM_HEIGHT + 7 ) / 8 )
		#define VRAM_TOTAL_BYTES	( VRAM_WIDTH * VRAM_HEIGHT_BYTES )

		typedef union vRam_s {
			unsigned char pixels[ VRAM_TOTAL_BYTES ];
			unsigned char cols[ VRAM_WIDTH * VRAM_HEIGHT_BYTES ];
		} vRam_t;
    #elif defined( LCD_ROW_ORIENTED )
      /* ========================================================================== */
	  /* Row oriented LCD */
		#define VRAM_WIDTH_BYTES	( VRAM_WIDTH / 8 )
		#define VRAM_TOTAL_BYTES	( VRAM_WIDTH_BYTES * VRAM_HEIGHT )

      #if CHAR_WIDTH % 8 == 0 && VRAM_WIDTH_IN_TEXT_UNIT % 4 == 0	/* 8 * 4 = 32 bits */
		#define VRAM_ROW_ATOM_BYTE		4
		#define VRAM_ROW_ATOM_BITS_MASK	0xFFFFFFFF
		#define VRAM_ROW_ATOM_BIT_LOG	5		/* 2 ^ 5 = 32 */
		#define VRAM_ROW_ATOM_REMANDER	0x1F	/* remainder / 32 */
		#define VRAM_ROW_ATOM_NUM		( VRAM_WIDTH_BYTES / VRAM_ROW_ATOM_BYTE )
		typedef unsigned long rows_atom_t;
      #endif

		typedef struct rows_s {
			rows_atom_t atom[ VRAM_ROW_ATOM_NUM ];
		} rows_t;

		typedef union vRam_s {
			unsigned char pixels[ VRAM_TOTAL_BYTES ];
			rows_t rows[ VRAM_HEIGHT ];
		} vRam_t;
      /* ========================================================================== */
    #endif
  #else
	???
  #endif
#endif

#ifdef VRAM_MMAP
extern vRam_t *pvRam;
#define vRam	( *pvRam )
#else
extern vRam_t vRam;
#endif

/*
 * There 3 types of LCD orientations, which are row-oriented, column-oriented type 1 and 2. 
 *
 * Row oriented: 
 * +---------------------------------------+
 * | 0 1 2 3 4 5 6 7 ..... 0 1 2 3 4 5 6 7 |
 * :                                       :
 * +---------------------------------------+
 *
 * Column oriented: 
 * +---------------------+      Type 1: bytes order is column-oriented. 
 * | 0 0 0 0 ......... 0 |      Type 2: bytes order is row-oriented. 
 * | 1 1 1 1 ......... 1 |
 * | : : : : ......... : |
 * | 7 7 7 7 ......... 7 |
 * | : : : : ......... : |
 * | 0 0 0 0 ......... 0 |
 * | 1 1 1 1 ......... 1 |
 * | : : : : ......... : |
 * | 7 7 7 7 ......... 7 |
 * +---------------------+
 *
 */


#endif /* __GS_VRAM_H__ */

