#include <zarlinkCommonDaa.h>

#if defined( CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES )
#include "ve880_api.h"
#endif

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES )
#include "ve890_api.h"
#endif

#undef DEBUG_API

#if defined(DEBUG_API)
#define DEBUG_API_PRINT() printk("%s(%d) line #%d\n",__FUNCTION__,__LINE__,pLine->ch_id);
#else
#define DEBUG_API_PRINT()
#endif

/*
** local function 
*/
static void zarlinkFxoLineStateControl( VpLineCtxType *pLineCtx, VpLineStateType lineState );
static BOOL zarlinkFxoLineIsOffhook(VpLineCtxType *pLineCtx, VpDevCtxType *pDevCtx);
static BOOL zarlinkFxoGetLineStatus(VpLineCtxType *pLineCtx, uint8 input );

static void zarlinkFxoLineStateControl( VpLineCtxType *pLineCtx, VpLineStateType lineState )
{
    switch( lineState )
    {
		/* Put device into on-hook - release line */
		/* Disables ring validation mode */
		case VP_LINE_FXO_LOOP_OPEN:
		{
		   printk("DAA: Going OnHook\n");
		   printk("DAA: Disables ring validation\n");
		}
		break;
	  
		/*  Caller ID receive mode, enable onhook data reception  */
		case VP_LINE_FXO_OHT:
		{
		   printk("DAA: Enable on-hook Caller ID receive. \n");
		}
		break;

		/* Put device into off-hook - seize line */
		case VP_LINE_FXO_TALK:
		{
		   printk("DAA: Going OffHook\n");
		}
		break;

		/* Enable ring validation mode */
		case VP_LINE_FXO_LOOP_CLOSE:
		{
		   printk("DAA: Enable ring validation\n");
		}
		break;
		
		default:
		{
		   printk("DAA: unrecognized DAA mode\n");
		}
		break;
    }

   VpSetLineState( pLineCtx, lineState );
}

static BOOL zarlinkFxoLineIsOffhook(VpLineCtxType *pLineCtx, VpDevCtxType *pDevCtx)
{
   bool liuStatus = FALSE;
   bool vpApiEventPending = FALSE;
   static bool liuStatusOld = FALSE;
   
    if ( VpApiTick( pDevCtx, &vpApiEventPending ) == VP_STATUS_SUCCESS)
    {
       ;
    }
    else
    {
       printk(("SLIC ERROR %s %d\n", __FUNCTION__, __LINE__));
    }
   
    /* LIU status does not return success at this point due to Zarlink bug */
    if ( VpGetLineStatus( pLineCtx, VP_INPUT_LIU, &liuStatus ) != VP_STATUS_SUCCESS )
    {
       printk(("DAA ERROR %s %d\n", __FUNCTION__, __LINE__));
    } 

    liuStatusOld = liuStatus;

	return liuStatus;
}

static BOOL zarlinkFxoGetLineStatus(VpLineCtxType *pLineCtx, uint8 input )
{
	bool ringStatus = FALSE;
	bool dis_connectStatus = FALSE;
	bool polStatus = FALSE;
	bool feedStatus = FALSE;
	bool liuStatus = FALSE;
	
	switch (input)
	{
		case 0: // Ringing status (On/Off)
			if ( VpGetLineStatus( pLineCtx, VP_INPUT_RINGING, &ringStatus ) != VP_STATUS_SUCCESS )
			{
				printk(("DAA ERROR %s %d\n", __FUNCTION__, __LINE__));
			}
			else
				return ringStatus;
			break;
		case 1: // Line status (connect/dis-connect)
			if ( VpGetLineStatus( pLineCtx, VP_INPUT_DISCONNECT, &dis_connectStatus ) != VP_STATUS_SUCCESS )
			{
				printk(("DAA ERROR %s %d\n", __FUNCTION__, __LINE__));
			}
			else
				return dis_connectStatus;
			break;
		case 2: // Polarity status (normal/reverse)
			if ( VpGetLineStatus( pLineCtx, VP_INPUT_POLREV, &polStatus ) != VP_STATUS_SUCCESS )
			{
				printk(("DAA ERROR %s %d\n", __FUNCTION__, __LINE__));

			}
			else
				return polStatus;
			break;
		case 3: // Feed status
			if ( VpGetLineStatus( pLineCtx, VP_INPUT_FEED_DIS, &feedStatus ) != VP_STATUS_SUCCESS )
			{
				printk(("DAA ERROR %s %d\n", __FUNCTION__, __LINE__));
			}
			else
				return feedStatus;
			break;
		case 4: // Line in use status
			if ( VpGetLineStatus( pLineCtx, VP_INPUT_LIU, &liuStatus ) != VP_STATUS_SUCCESS )
			{
				printk(("DAA ERROR %s %d\n", __FUNCTION__, __LINE__));
			}
			else
			{
				printk("liuStatus = %d\n", liuStatus);
				return liuStatus;
			}
			break;
		default:
			printk("Error in function %s, line%d\n", __FUNCTION__, __LINE__);
			break;
	}
	
	return 0;
}

/* Move from ve890_api.c *****************************************************/

BOOL zarlinkGetFxoLineStatus(RTKLineObj *pLine, uint8 category)
{

	#ifdef DEBUG_POLLING_API
	printk("%s(%d)\n",__FUNCTION__,__LINE__);
	#endif

	return zarlinkFxoGetLineStatus( pLine->pLineCtx, category );
}

VpStatusType zarlinkSetFxoLineState(RTKLineObj *pLine, VpLineStateType lineState)
{

	DEBUG_API_PRINT(); 

	zarlinkFxoLineStateControl( pLine->pLineCtx, lineState );
	return VP_STATUS_SUCCESS;
}

VpStatusType zarlinkSetFxoLineOnHook(RTKLineObj *pLine)
{

	DEBUG_API_PRINT(); 

	zarlinkFxoLineStateControl( pLine->pLineCtx, VP_LINE_FXO_OHT );
	pLine->hook_st = 0;
	return VP_STATUS_SUCCESS;
}

VpStatusType zarlinkSetFxoLineOffHook(RTKLineObj *pLine)
{

	DEBUG_API_PRINT(); 

	zarlinkFxoLineStateControl( pLine->pLineCtx, VP_LINE_FXO_TALK );
	pLine->hook_st = 1;
	return VP_STATUS_SUCCESS;
}

VpStatusType zarlinkSetFxoLineOHT(RTKLineObj *pLine)
{

	DEBUG_API_PRINT(); 

	zarlinkFxoLineStateControl( pLine->pLineCtx, VP_LINE_FXO_OHT );
	return VP_STATUS_SUCCESS;
}

VpStatusType zarlinkSetFxoRingValidation(RTKLineObj *pLine)
{

	DEBUG_API_PRINT(); 

	zarlinkFxoLineStateControl( pLine->pLineCtx, VP_LINE_FXO_LOOP_CLOSE );
	return VP_STATUS_SUCCESS;
}

unsigned char zarlinkGetFxoHookStatus(RTKLineObj *pLine)
{
	uint8 hook=0;

	#ifdef DEBUG_POLLING_API
	printk("%s(%d)\n",__FUNCTION__,__LINE__);
	#endif

	hook = pLine->hook_st;

	#ifdef DEBUG_API
	//Ve890_dump_hook_map(pLine->ch_id,hook);
	#endif

	return (hook);
}

