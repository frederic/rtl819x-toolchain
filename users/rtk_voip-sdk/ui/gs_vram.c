#include <stdio.h>
#ifndef _TEST_MODE
#include <sys/mman.h>
#endif
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "ui_config.h"
#include "gs_lib.h"
#include "gs_vram.h"

#define VRAM_MMAP_DEVNAME	"/dev/lcm0"

#ifdef VRAM_MMAP
vRam_t *pvRam = NULL;
static int fdVRamMmap;
#else
vRam_t vRam;
#endif

void GS_InitializeVRam( void )
{
#if defined( VRAM_MMAP ) && defined( _TEST_MODE )
	/* ------------------------------------------------------- */
	extern unsigned char lcm_mmap[];
	pvRam = ( vRam_t * )lcm_mmap;
#elif defined( VRAM_MMAP ) && !defined( _TEST_MODE )
	/* ------------------------------------------------------- */
	if( ( fdVRamMmap = open( VRAM_MMAP_DEVNAME, O_RDWR ) ) == -1 ) {
		debug_out( "Open " VRAM_MMAP_DEVNAME "error(%d)\n", errno );
		return;
	}

	if( ( pvRam = mmap( ( void * )0, VRAM_MMAP_SIZE, PROT_WRITE | PROT_READ, 
					MAP_SHARED, fdVRamMmap, 0 ) ) == ( void * )-1 )
	{
		debug_out( "mmap " VRAM_MMAP_DEVNAME "error(%d)\n", errno );
		pvRam = NULL;
		return;
	}
#endif 
}

void GS_TerminateVRam( void )
{
#if defined( VRAM_MMAP ) && defined( _TEST_MODE )
	/* ------------------------------------------------------- */
	pvRam = NULL;
#elif defined( VRAM_MMAP ) && !defined( _TEST_MODE )
	/* ------------------------------------------------------- */
	munmap( pvRam, VRAM_MMAP_SIZE );

	close( fdVRamMmap );

	pvRam = NULL;
#endif 
}

#ifdef _TEXT_MODE
static inline void GS_VRamToLcdDevice_Text( const rect_t *rect )
{
	int y, len;
	unsigned char *pCh;

	len = rect ->right - rect ->left;
	pCh = &vRam.ch[ rect ->left + rect ->top * VRAM_WIDTH ];
	
	for( y = rect ->top; y < rect ->bottom; y ++, pCh += VRAM_WIDTH ) {
		LCD_DrawText( rect ->left, y, pCh, len );
	}
}
#endif /* _TEXT_MODE */

#if !defined( _TEXT_MODE ) && defined( LCD_COL_ORIENTED )
static inline void GS_VRamToLcdDevice_1bpp_Col( const rect_t *pRect )
{
	int start;
	//int bytes;
	//int x;
	rect_t rect;

	rect = *pRect;

	rect.top >>= 3;								/* rect.top = rect.top / 8 */ 
	rect.bottom = ( rect.bottom + 7 ) >> 3;		/* rect.bottom = ( rect.bottom + 7 ) / 8 */

  #if 1		/* less ioctl more efficient */ 
	start = rect.left * VRAM_HEIGHT_BYTES + 0;	/* rect.top == 0 */
	LCD_WriteData( start, &vRam.pixels[ start ], ( rect.right - rect.left ) * VRAM_HEIGHT_BYTES );
  #else
	if( rect.top == 0 && rect.bottom == VRAM_HEIGHT_BYTES ) {	/* continuous draw */
		start = rect.left * VRAM_HEIGHT_BYTES + 0;	/* rect.top == 0 */
		LCD_WriteData( start, &vRam.pixels[ start ], ( rect.right - rect.left ) * VRAM_HEIGHT_BYTES );
	} else {													/* divide into n-column draw */
		bytes = rect.bottom - rect.top;

		if( bytes > 0 ) {

			start = rect.left * VRAM_HEIGHT_BYTES + rect.top;

			for( x = rect.left; x < rect.right; x ++ ) {
				LCD_WriteData( start, &vRam.pixels[ start ], bytes );
				start += VRAM_HEIGHT_BYTES;
			}
		}
	}
  #endif
}
#endif /* !_TEXT_MODE && LCD_COL_ORIENTED */

#if !defined( _TEXT_MODE ) && defined( LCD_COL2_ORIENTED )
static inline void GS_VRamToLcdDevice_1bpp_Col_Type2( const rect_t *pRect )
{
	int bytes;
	rect_t rect;
  #if defined( VRAM_MMAP ) && VRAM_MMAP == 2
  #else
	int start;
	int y;
  #endif

	rect = *pRect;

	rect.top >>= 3;								/* rect.top = rect.top / 8 */ 
	rect.bottom = ( rect.bottom + 7 ) >> 3;		/* rect.bottom = ( rect.bottom + 7 ) / 8 */

	bytes = rect.right - rect.left;

  #if defined( VRAM_MMAP ) && VRAM_MMAP == 2
	LCD_DirtyMmap2( rect.top, rect.left, bytes, rect.bottom - rect.top );
  #else	
	start = rect.left + rect.top * VRAM_WIDTH;
#if 1
	for( y = rect.top; y < rect.bottom; y ++ ) {
		LCD_WriteData2( y, rect.left, &vRam.pixels[ start ], bytes );
		start += VRAM_WIDTH;
	}
#endif
  #endif /* VRAM_MMAP == 2 */

}
#endif /* !_TEXT_MODE && LCD_COL2_ORIENTED */

#if !defined( _TEXT_MODE ) && defined( LCD_ROW_ORIENTED )
static inline void GS_VRamToLcdDevice_1bpp_Row( const rect_t *pRect )
{
	int start;
	//int bytes;
	//int x;
	rect_t rect;

	rect = *pRect;

	rect.left >>= 3;							/* rect.left = rect.left / 8 */ 
	rect.right = ( rect.right + 7 ) >> 3;		/* rect.right = ( rect.right + 7 ) / 8 */

  #if 1		/* less ioctl more efficient */ 
	start = rect.top * VRAM_WIDTH_BYTES + 0;	/* rect.left == 0 */
	LCD_WriteData( start, &vRam.pixels[ start ], ( rect.bottom - rect.top ) * VRAM_WIDTH_BYTES );
  #else
	if( rect.left == 0 && rect.right == VRAM_WIDTH_BYTES ) {	/* continuous draw */
		start = rect.top * VRAM_WIDTH_BYTES + 0;	/* rect.left == 0 */
		LCD_WriteData( start, &vRam.pixels[ start ], ( rect.bottom - rect.top ) * VRAM_WIDTH_BYTES );
	} else {													/* divide into n-column draw */
		bytes = rect.right - rect.left;

		if( bytes > 0 ) {

			start = rect.top * VRAM_WIDTH_BYTES + rect.left;

			for( y = rect.top; y < rect.bottom; y ++ ) {
				LCD_WriteData( start, &vRam.pixels[ start ], bytes );
				start += VRAM_HEIGHT_BYTES;
			}
		}
	}
  #endif
}
#endif /* !_TEXT_MODE && LCD_ROW_ORIENTED */

void GS_VRamToLcdDevice( const rect_t *pInvalidRect )
{
	rect_t rect;

  #ifdef VRAM_MMAP
	if( !pvRam ) {	/* Not initiali */
		debug_out( "VRam MMap is not initialized\n" );
		return;
	}
  #endif

	/* Update full screen */
	if( pInvalidRect == NULL ) {
		rect.left = 0;
		rect.top = 0;
		rect.right = VRAM_WIDTH;
		rect.bottom = VRAM_HEIGHT;
		
		goto label_check_rect_done;
	}
	
	rect = *pInvalidRect;
	
	if( rect.top >= VRAM_HEIGHT || rect.left >= VRAM_WIDTH ||
		rect.bottom < 0 || rect.right < 0 )
	{
		/* Invisible rectangle */
		return;
	}
	
	if( rect.left >= rect.right || rect.top >= rect.bottom )
		return;	/* Not normalize */
	
	if( rect.left < 0 ) rect.left = 0;
	if( rect.top < 0 ) rect.top = 0;
	if( rect.right > VRAM_WIDTH ) rect.right = VRAM_WIDTH;
	if( rect.bottom > VRAM_HEIGHT ) rect.bottom = VRAM_HEIGHT;

	/*
	 * 0 <= rect.left < VRAM_WIDTH
	 * 0 <= rect.top < VRAM_HEIGHT
	 * rect.left < rect.right <= VRAM_WIDTH
	 * rect.top < rect.bottom <= VRAM_HEIGHT
	 */
label_check_rect_done:

#ifndef _TEXT_MODE
	/* graphic mode */	
  #ifdef LCD_COL_ORIENTED
	GS_VRamToLcdDevice_1bpp_Col( &rect );
  #elif defined( LCD_COL2_ORIENTED )
	GS_VRamToLcdDevice_1bpp_Col_Type2( &rect );
  #elif defined( LCD_ROW_ORIENTED )
	GS_VRamToLcdDevice_1bpp_Row( &rect );
  #endif	
#else
	/* text mode */
	GS_VRamToLcdDevice_Text( &rect );
#endif
}

