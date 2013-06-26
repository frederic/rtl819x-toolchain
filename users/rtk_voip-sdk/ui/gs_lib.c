#include <string.h>
#include "ui_config.h"
#include "gs_lib.h"
#include "gs_vram.h"
#include "gs_cursor.h"
#include "gs_font.h"
#include "gs_def.h"

fDrawOffScreen_t fDrawOffScreen;

CT_ASSERT( sizeof( fDrawOffScreen ) == sizeof( fDrawOffScreen.all ) );

void GS_Initialization( void )
{
	fDrawOffScreen.all = 0;

	GS_InitializeVRam();	/* must be initialized before using */

	GS_ClearScreen();	

	GS_InitializeCursor();
	GS_InitializeFont();
}

void GS_Termination( void )
{
	GS_TerminateVRam();		/* must be the last one */
}

void GS_DrawOffScreen( void )
{
	fDrawOffScreen.b.on = 1;
}

void GS_DrawOffScreenAndClearScreen( void )
{
	GS_DrawOffScreen();
	GS_ClearScreen();
}

void GS_DrawOnScreen( const rect_t *pInvalidRect )
{
	fDrawOffScreen.b.on = 0;
	
	if( !fDrawOffScreen.all )
		GS_VRamToLcdDevice( pInvalidRect );
}

void GS_SupervisorDrawOffScreen( void )
{
	fDrawOffScreen.b.supervisor = 1;
}

void GS_SupervisorDrawOnScreen( const rect_t *pInvalidRect )
{
	fDrawOffScreen.b.supervisor = 0;
	
	GS_DrawOnScreen( pInvalidRect );
}

void GS_ClearScreen( void )
{
#ifdef _TEXT_MODE
	memset( &vRam, 0x20, sizeof( vRam ) );	/* fill space instead of zero */
#else
	memset( &vRam, 0, sizeof( vRam ) );
#endif
	
	if( !fDrawOffScreen.all )
		GS_VRamToLcdDevice( NULL );
}

