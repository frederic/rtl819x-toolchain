#include <linux/types.h>
#include <asm/uaccess.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 ) 
#include "lcm_char16x2.h"
#endif
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
#include "lcm_ht1650.h"
#endif
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
#include "lcm_epl65132.h"
#endif
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
#include "lcm_splc780d.h"
#endif

void do_lcm_opt_ctl( void *pUser, unsigned int len )
{
	TstLcmCtl lcmCtl;
	
	if( len > sizeof( TstLcmCtl ) ) {
		len = sizeof( TstLcmCtl );
		printk( "LCM: Truncate user specified length\n" );
	}
	
	copy_from_user( &lcmCtl, pUser, len );

	switch( lcmCtl.General.cmd ) {
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 ) 
	case LCM_CMD_DISPLAY_ON_OFF:
		Display_on_off( lcmCtl.DisplayOnOff.bDisplayOnOff, 
						lcmCtl.DisplayOnOff.bCursorOnOff,
						lcmCtl.DisplayOnOff.bCursorBlink, 0 );
		break;
	
	case LCM_CMD_MOVE_CURSOR_POS:
		lcm_move_cursor_position( lcmCtl.MoveCursorPosition.x, 
								  lcmCtl.MoveCursorPosition.y );
		break;
		
	case LCM_CMD_DRAW_TEXT:
		lcm_draw_text( lcmCtl.DrawText.x, lcmCtl.DrawText.y,
					   lcmCtl.DrawText.szText, lcmCtl.DrawText.len );
		break;
#endif /* CONFIG_RTK_VOIP_GPIO_IPP_100 || CONFIG_RTK_VOIP_GPIO_IPP_101 */ 

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
	case LCM_CMD_DISPLAY_ON_OFF:
		ht1650_DisplayOnOff( lcmCtl.DisplayOnOff.bDisplayOnOff );
		break;

	case LCM_CMD_WRITE_DATA:
		//printk( "W(%d,%X,%d)\n", lcmCtl.WriteData.start, lcmCtl.WriteData.pixels[ 0 ], lcmCtl.WriteData.len );
		ht1650_WriteData( lcmCtl.WriteData.start, lcmCtl.WriteData.pixels, 
						  lcmCtl.WriteData.len );
		break;

	//case LCM_CMD_MOVE_CURSOR_POS:
	//case LCM_CMD_DRAW_TEXT:
#endif /* CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 || CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 */

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
	case LCM_CMD_DISPLAY_ON_OFF:
		epl65132_DisplayOnOff( lcmCtl.DisplayOnOff.bDisplayOnOff );
		break;

	//case LCM_CMD_WRITE_DATA2:
	//	epl65132_WriteData( lcmCtl.WriteData2.page, lcmCtl.WriteData2.col,
	//						lcmCtl.WriteData2.pixels, lcmCtl.WriteData2.len );
	//	break;
		
	case LCM_CMD_DIRTY_MMAP2:
		epl65132_DirtyMmap( lcmCtl.DirtyMmap2.page, lcmCtl.DirtyMmap2.col, 
							lcmCtl.DirtyMmap2.len, lcmCtl.DirtyMmap2.rows );
		break;

	//case LCM_CMD_MOVE_CURSOR_POS:
	//case LCM_CMD_DRAW_TEXT:
	//case LCM_CMD_WRITE_DATA:
#endif /* CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 */

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	case LCM_CMD_DISPLAY_ON_OFF:
		splc780d_DisplayOnOff(	lcmCtl.DisplayOnOff.bDisplayOnOff, 
								lcmCtl.DisplayOnOff.bCursorOnOff,
								lcmCtl.DisplayOnOff.bCursorBlink );
		break;
	
	case LCM_CMD_MOVE_CURSOR_POS:
		splc780d_MoveCursorPosition(	lcmCtl.MoveCursorPosition.x, 
										lcmCtl.MoveCursorPosition.y );
		break;
		
	case LCM_CMD_DRAW_TEXT:
		splc780d_DrawText(	lcmCtl.DrawText.x, lcmCtl.DrawText.y,
							lcmCtl.DrawText.szText, lcmCtl.DrawText.len );
		break;
#endif /* CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 */
	
	default:
		printk( "LCD not support\n" );
		break;
	}
}

/* ==================================================================== */
/* CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 || CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 */
/* ==================================================================== */
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
#endif /* CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 || CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 */

/* ==================================================================== */
/* CONFIG_RTK_VOIP_GPIO_IPP_100 || CONFIG_RTK_VOIP_GPIO_IPP_101 */
/* CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 */
/* ==================================================================== */
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
#endif /* CONFIG_RTK_VOIP_GPIO_IPP_100 || CONFIG_RTK_VOIP_GPIO_IPP_101 */ 

