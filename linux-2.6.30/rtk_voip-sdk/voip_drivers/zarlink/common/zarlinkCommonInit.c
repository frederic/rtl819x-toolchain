
/** \file zarlinkCommonInit.c
 * 
 *
 * This file contains the major process of zarlink slic
 * 
 *
 * Copyright (c) 2010, Realtek Semiconductor, Inc.
 *
 */
#include <linux/sched.h>
#include <linux/delay.h>
#include "zarlinkCommonSlic.h"

#undef DEBUG_INT

/*
** local definition 
*/
#define SLIC_INIT_TIMEOUT 1200

/*
** local variable 
*/

/*
** local function 
*/
static BOOL RtkInitFxsLine(RTKLineObj *pLine);
static BOOL RtkInitFxoLine(RTKLineObj *pLine);
static BOOL zarlinkWaitForEvent( VpDevCtxType* pDevCtx, VpEventCategoryType category, uint16 event);
static void zarlinkAlarmHandle(unsigned long args);

/*
** external function
*/

#ifdef CONFIG_RTL865X_WTDOG
extern void plat_enable_watchdog( void );
extern void plat_disable_watchdog( void );
#elif defined( CONFIG_RTL_WTDOG )
extern void bsp_enable_watchdog( void );
extern void bsp_disable_watchdog( void );
#endif

#define MAX_DEV_OBJ 16
static int gDevNum = 0;

/* handle different types of dev obj */
RTKDevObj * RTKDevList[MAX_DEV_OBJ];

static void zarlinkAlarmHandle(unsigned long args);

/* Regiser Dev for event handler and VpApiTick() */
int zarlinkRegDevForEvHandle(RTKDevObj * pDev)
{
	if (gDevNum > MAX_DEV_OBJ)
		return -1;

	if (gDevNum == 0) {
		/* init data structure and VpApiTick() timer in the first time */
		memset(RTKDevList,0,sizeof(RTKDevList));

		register_timer_10ms( ( fn_timer_t )zarlinkAlarmHandle, NULL );
	}

	RTKDevList[gDevNum] = pDev;

	return gDevNum++;
}

/**
 * Function:  zarlinkAlarmHandle()
 *
 * Description: This function is called at a periodic rate to perform all
 * required API operations.  It implements the functional requirements of the
 * application mentioned in the header.
 */
static void zarlinkAlarmHandle(unsigned long args)
{
    int deviceNum;
    bool deviceEventStatus = FALSE;
    VpEventType pEvent;

	/*
 	 * This loop will query the FXS device for events, and when an event is
	 * found (deviceEventStatus = TRUE), it will parse the event and perform
	 * further operations.
	 */
    for (deviceNum = 0; deviceNum < gDevNum; deviceNum++) {

        VpApiTick(RTKDevList[deviceNum]->pDevCtx, &deviceEventStatus);

		RTKDevList[deviceNum]->UpdIOState(RTKDevList[deviceNum]->pLine[0]);

        if(deviceEventStatus == TRUE) {
            while(VpGetEvent(RTKDevList[deviceNum]->pDevObj, &pEvent)) {
            	/*
 				 * If the event was a Response:Device Init Complete event,
 				 * we need to initialize the device and associated lines, so
				 * send the event structure (pointer) to the Init function for
				 * handling.
				 */
                if (pEvent.eventCategory == VP_EVCAT_RESPONSE) {
                    if(pEvent.eventId == VP_DEV_EVID_DEV_INIT_CMP) {
                        //InitQuickStartSettings(&pEvent);
                    } else if (pEvent.eventId == VP_LINE_EVID_RD_OPTION) {
                        //InitReadOptions(&pEvent);
                    } else {
                        /* Do nothing */
                    }
                    /* We're initializing if a response event occurred. */
                    return;
                }
                //ProcessFXSEvent(&pEvent);
            }
        }
    }

    return;
}

/* 
** API	  : zarlinkCaculateDevObj()
** Desp	  :	Caculate dev num by device type
** input  : RTKDevType 
** return : number of device
*/
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK_ON_NEW_ARCH
int zarlinkCaculateDevObj(RTKDevType dev_type)
{
	int fxs_dev;
	int fxo_dev;
	int fxsfxo_dev;
	int fxsfxs_dev=0;
	int total_dev;

	if (DEV_FXS==dev_type) //display once
		PRINT_MSG("Total %d lines. %d fxs and %d fxo from Global configuration\n",
			ZARLINK_SLIC_CH_NUM, ZARLINK_FXS_LINE_NUM, ZARLINK_FXO_LINE_NUM);

	/* Use minimum number of device	to caculate possible combination */
	fxsfxo_dev = MIN(ZARLINK_FXS_LINE_NUM, ZARLINK_FXO_LINE_NUM);/* FXS+FXO dev */
#if defined (CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)
	fxsfxs_dev = (ZARLINK_FXS_LINE_NUM - fxsfxo_dev) >> 1;	 	 /* FXS+FXS dev */ 
#endif
	fxs_dev = ZARLINK_FXS_LINE_NUM - fxsfxo_dev - 2*fxsfxs_dev;  /* FXS device  */
	fxo_dev	= ZARLINK_FXO_LINE_NUM - fxsfxo_dev; 				 /* FXO device  */

	total_dev = fxs_dev+fxsfxs_dev+fxo_dev+fxsfxo_dev;

	if (DEV_FXS==dev_type) //display once
		PRINT_MSG("Total %d devices\tFXS:%d, FXS+FXS:%d, FXS+FXO:%d, FXO:%d\n",
			total_dev, fxs_dev, fxsfxs_dev, fxsfxo_dev, fxo_dev );

	if (ZARLINK_SLIC_DEV_NUM < total_dev) {
		PRINT_R("error : %s() Not enough space for chObj\n",__FUNCTION__);
		return FAILED;
	}

	switch (dev_type) {
		case DEV_FXS:
			return fxs_dev;			
			break;
		case DEV_FXO:
			return fxo_dev;			
			break;
		case DEV_FXSFXS:
			return fxsfxs_dev;			
			break;
		case DEV_FXSFXO:
			return fxsfxo_dev;			
			break;
		default:
			return 0;
			break;
	}
	return 0;
}
#endif

/* 
** API	  : zarlinkInitDevice()
** Desp	  :	Init Zarlink device
** input  : Realtek device obj pointer
** return : SUCCESS/FAILED
*/
BOOL zarlinkInitDevice( RTKDevObj *pDev )
{
   	int ch;
	BOOL rtn;
	VpProfilePtrType dev_profile;
	VpStatusType status;
	VpTermType term_type;

   	PRINT_MSG("Initializing device 0x%x\n", pDev->dev_id);

	/* Create the API-2 device */
	status = VpMakeDeviceObject( pDev->VpDevType, 
								 (VpDeviceIdType) pDev->dev_id, 
								 pDev->pDevCtx,	
								 pDev->pDevObj	
							  );

	if ( status != VP_STATUS_SUCCESS ) {
	  	PRINT_R("Error: VpMakeDeviceObject (status %d) \n", status);
	  	return FAILED;
	}

	for (ch=0; ch < pDev->max_line; ch++) {

		if (pDev->pLine[ch]->line_type == LINE_FXS ) 
			 term_type = VP_TERM_FXS_GENERIC;
		else term_type = VP_TERM_FXO_GENERIC;

		/* Create line objects */
		status = VpMakeLineObject( term_type, 
							   pDev->pLine[ch]->channelId,	
							   pDev->pLine[ch]->pLineCtx,
							   pDev->pLine[ch]->pLineObj,
							   pDev->pDevCtx		
							);
		if ( status != VP_STATUS_SUCCESS ) {
	  		PRINT_R("Error: VpMakeLineObject ch %d term=%d(status %d) \n", 
				ch, term_type, status);
	  		return FAILED;
		}
	}

	dev_profile = pDev->pDev_profile;

	PRINT_MSG("tick rate %d ms\n", dev_profile[DEVICE_PROFILE_TICK_RATE_IDX]);

	/* Avoid taking to long to init device */
#ifdef CONFIG_RTL865X_WTDOG
	plat_disable_watchdog();
	PRINT_MSG("Disable watchdog\n" );
#elif defined( CONFIG_RTL_WTDOG )
	bsp_disable_watchdog();
	PRINT_MSG("Disable watchdog\n" );
#endif	

	/* Initialize the device */
	status = VpInitDevice(  pDev->pDevCtx,		
						   	pDev->pDev_profile,
						   	pDev->pAC_profile,
						   	pDev->pDC_profile,
						   	pDev->pRing_profile,
						   	pDev->pACFxoLC_profile,
						   	pDev->pFxoDial_profile
						);

	if ( status != VP_STATUS_SUCCESS ) {
		PRINT_R("Error: VpInitDevice (status %d)\n", status );
		goto DONE;
	}

	/* Check if VP_DEV_EVID_DEV_INIT_CMP occurred */
	if( TRUE != zarlinkWaitForEvent( pDev->pDevCtx, 
		VP_EVCAT_RESPONSE, VP_DEV_EVID_DEV_INIT_CMP ) ) {
		PRINT_R("Error: Device %d initialization was not complete\n",pDev->dev_id);
		status = VP_STATUS_FAILURE;
		goto DONE;
	}

	PRINT_MSG("DEV%x initialized OK\n",pDev->dev_id);
	pDev->dev_st = DEV_S_READY;

	/* Initialize API-2 line settings */
	for (ch=0; ch < pDev->max_line; ch++) {
		if (pDev->pLine[ch]->line_type == LINE_FXS)	{
			rtn = RtkInitFxsLine(pDev->pLine[ch]);

		} else if (pDev->pLine[ch]->line_type == LINE_FXO) {
			rtn = RtkInitFxoLine(pDev->pLine[ch]);

		} else {
			PRINT_R("Error: unknow line_type %d\n",pDev->pLine[ch]->line_type);
			goto DONE;
		}
	}

DONE:
#ifdef CONFIG_RTL865X_WTDOG
	plat_enable_watchdog();
	PRINT_MSG("Enable watchdog\n");
#elif defined( CONFIG_RTL_WTDOG )
	bsp_enable_watchdog();
	PRINT_MSG("Enable watchdog\n");
#endif
	if ( status != VP_STATUS_SUCCESS )	return FAILED;

	return SUCCESS;
}

/* 
** API	  : RtkInitFxsLine()
** Desp	  :	Init Zarlink FXS line obj/ctx
** input  : Realtek Line obj pointer
** return : SUCCESS/FAILED 
*/
static BOOL RtkInitFxsLine(RTKLineObj *pLine)
{
	VpStatusType status;
	VpOptionTimeslotType timeslot;

	PRINT_MSG("Initializing line %d\n",pLine->ch_id);

	/* Initialize the line with proper profile settings */
	status = VpInitLine( pLine->pLineCtx,
						 pLine->pAC_profile,
						 pLine->pDCfxo_profile,	
						 pLine->pRing_profile);
						 
	if ( status != VP_STATUS_SUCCESS ) {
		PRINT_R("VpInitLine #%d failed (%d) \n",pLine->ch_id, status);
		return FAILED;
	}

   status = VpMapLineId (pLine->pLineCtx, pLine->ch_id);

   /* Set the initial line state */
   status = VpSetLineState( pLine->pLineCtx, VP_LINE_STANDBY );

   if ( status != VP_STATUS_SUCCESS ) {
      	PRINT_R("VpSetLineState failed (%d) \n", status);
      	return FAILED;
   }
   
   /* Check if VP_LINE_EVID_LINE_INIT_CMP occurred */
   if( TRUE != zarlinkWaitForEvent( pLine->pDev->pDevCtx, 
		VP_EVCAT_RESPONSE, VP_LINE_EVID_LINE_INIT_CMP ) )
   {
      PRINT_R("SLIC: ERROR: Line initialization was not completed\n");
      return FAILED;
   }
   
   // Zarkink FAE said: cal of line may skip for 89 series
   /* Calibrate the line */
   status = VpCalLine(pLine->pLineCtx);
   if ( (status != VP_STATUS_SUCCESS) && 
		(status != VP_STATUS_FUNC_NOT_SUPPORTED) ) {
      PRINT_R("SLIC ERROR: %s line%d, status = %d \n", 
			__FUNCTION__, __LINE__, status);
      return FAILED;
   }
#if 1 //TODO vincent
   else if ( status == VP_STATUS_SUCCESS )
   {
      /* Check if VP_EVID_CAL_CMP occurred */
      if( TRUE != zarlinkWaitForEvent( pLine->pDev->pDevCtx, 
			VP_EVCAT_RESPONSE, VP_EVID_CAL_CMP ) ) {
         PRINT_R("SLIC: ERROR: Line calibration was not completed\n");
         return FAILED;
      }
   }
#endif

   timeslot.tx = (uint8)pLine->slot_tx;
   timeslot.rx = (uint8)pLine->slot_rx;
   PRINT_MSG("FXS line TX/RX timeslot = %d/%d\n", pLine->slot_tx, pLine->slot_rx);

   /* Don't give device context (just give NULL) 
 	* if you are configuring any parameters for a specific line 
 	*/
   status = VpSetOption( pLine->pLineCtx, VP_NULL, 
		VP_OPTION_ID_TIMESLOT, (void*)&timeslot );

   if ( status != VP_STATUS_SUCCESS ) {
      PRINT_R("VpSetOption for timeslot failed (%d) \n", status);
   }   

   /* First time always try whatever codec type is set. 
   ** If we get an invalid argument error we try linear */
   status = VpSetOption( pLine->pLineCtx, VP_NULL, 
		VP_OPTION_ID_CODEC, (void*)&pLine->codec_type );   
   
	if ( status != VP_STATUS_SUCCESS ) {
    	PRINT_R("VpSetOption for codec type failed. status = %d\n", status);
		return FAILED;
   	}

   	/* Thlin add for init ring cadence */
   	//PRINT_G("update ring cad profile = 0x%p, dev=%d\n", 
   	//			ring_cad_profile, deviceId);
   	status = VpInitRing( pLine->pLineCtx, pLine->pRing_cad_profile, VP_PTABLE_NULL);

   	if ( status != VP_STATUS_SUCCESS ) {
	  	PRINT_R("VpInitRing failed (%d) \n", status);
	  	return FAILED;
   	}
   
   	/* Thlin add for talk */
   	status = VpSetLineState( pLine->pLineCtx, VP_LINE_OHT );
   
	if ( status != VP_STATUS_SUCCESS ) {
    	PRINT_R("VpSetLineState failed (%d) \n", status);
      	return FAILED;
   	}
	pLine->line_st = LINE_S_READY;
   	PRINT_MSG("FXS #%d line initialized OK\n",pLine->ch_id);

   	return SUCCESS;
}

/* 
** API	  : RtkInitFxoLine()
** Desp	  :	Init Zarlink FXO line obj/ctx
** input  : Realtek Line obj pointer
** return : SUCCESS/FAILED 
*/
static BOOL RtkInitFxoLine(RTKLineObj *pLine)
{
	VpStatusType status;
	VpOptionTimeslotType timeslot;

	PRINT_MSG("Initializing line %d\n",pLine->ch_id);

	/* Initialize the line with proper profile settings */
	status = VpInitLine( pLine->pLineCtx,
						 pLine->pAC_profile,
						 pLine->pDCfxo_profile,	
						 pLine->pRing_profile);
						 
	if ( status != VP_STATUS_SUCCESS ) {
		PRINT_R("VpInitLine failed (%d) \n", status);
		return FAILED;
	}

   /* Set the initial line state */
   status = VpSetLineState( pLine->pLineCtx, VP_LINE_FXO_OHT );

   if ( status != VP_STATUS_SUCCESS ) {
      	PRINT_R("VpSetLineState failed (%d) \n", status);
      	return FAILED;
   }
   
   /* Check if VP_LINE_EVID_LINE_INIT_CMP occurred */
   if( TRUE != zarlinkWaitForEvent( pLine->pDev->pDevCtx, 
		VP_EVCAT_RESPONSE, VP_LINE_EVID_LINE_INIT_CMP ) )
   {
      PRINT_R("SLIC: ERROR: Line initialization was not completed\n");
      return FAILED;
   }
   
   /* TODO Calibrate the line if needing */

   timeslot.tx = (uint8)pLine->slot_tx;
   timeslot.rx = (uint8)pLine->slot_rx;
   PRINT_MSG("FXO line TX/RX timeslot = %d/%d\n", pLine->slot_tx, pLine->slot_rx);

   /* Don't give device context (just give NULL) 
 	* if you are configuring any parameters for a specific line 
 	*/
   status = VpSetOption( pLine->pLineCtx, VP_NULL, 
		VP_OPTION_ID_TIMESLOT, (void*)&timeslot );

   if ( status != VP_STATUS_SUCCESS ) {
      PRINT_R("VpSetOption for timeslot failed (%d) \n", status);
   }   

   /* First time always try whatever codec type is set. 
   ** If we get an invalid argument error we try linear */
   status = VpSetOption( pLine->pLineCtx, VP_NULL, 
		VP_OPTION_ID_CODEC, (void*)&pLine->codec_type );   
   
	if ( status != VP_STATUS_SUCCESS ) {
    	PRINT_R("VpSetOption for codec type failed. status = %d\n", status);
		return FAILED;
   	}

   	/* Thlin add for talk */
   	status = VpSetLineState( pLine->pLineCtx, VP_LINE_FXO_OHT );
   
	if ( status != VP_STATUS_SUCCESS ) {
    	PRINT_R("VpSetLineState failed (%d) \n", status);
      	return FAILED;
   	}
	pLine->line_st = LINE_S_READY;
   	PRINT_MSG("FXO #%d line initialized OK\n",pLine->ch_id);

   	return SUCCESS;
}

/*
*****************************************************************************
** FUNCTION:   zarlinkWaitForEvent
**
** PURPOSE:    Waiting for specific event category and event Id on device.
**
** PARAMETERS: deviceNum - Device number of SLIC
**             category  - Event Category to wait for
**             event     - Event Id to wait for
**
** RETURNS:    TRUE  - Event occurred.
**             FALSE - Event did not occur. 
**
*****************************************************************************
*/
static BOOL zarlinkWaitForEvent( VpDevCtxType* pDevCtx, VpEventCategoryType category, uint16 event)
{
   int i;
   VpStatusType status;
   bool vpApiEventPending = FALSE;
   
   for (i = 0; i<SLIC_INIT_TIMEOUT; i++)
   {
      status = VpApiTick( pDevCtx, &vpApiEventPending );
      if (  (status == VP_STATUS_SUCCESS) && 
			(TRUE == vpApiEventPending) )
      {
         VpEventType pEvent;
         while(VpGetEvent(pDevCtx, &pEvent)) 
         {
            switch (pEvent.eventCategory) 
            {
               /* Add more categories as needed */
               case VP_EVCAT_RESPONSE:
                  switch(pEvent.eventId)
                  {
                     /* Add more events as needed */
                     case VP_DEV_EVID_DEV_INIT_CMP:
                        PRINT_MSG("SLIC: Received VP_DEV_EVID_DEV_INIT_CMP event (i = %d)\n", i);
						//InitQuickStartSettings(&pEvent);
                        break;
                     case VP_LINE_EVID_LINE_INIT_CMP:
                        PRINT_MSG("SLIC: Received VP_LINE_EVID_LINE_INIT_CMP event (i = %d)\n", i);
                        break;
                     case VP_EVID_CAL_CMP:
                        PRINT_MSG("SLIC: Received VP_EVID_CAL_CMP event (i= %d)\n", i);
                        break;
					 case VP_LINE_EVID_RD_OPTION:
                        PRINT_MSG("SLIC: Received VP_LINE_EVID_RD_OPTION event (i= %d)\n", i);
                    	//InitReadOptions(&pEvent);
						//EventMaskReady = 1;
						break;
                     default: 
                        /* Do nothing */
						#ifdef DEBUG_API
                        PRINT_MSG("SLIC: ERROR Unexpected Event %d came from the SLIC.\n", pEvent.eventId);
						#endif
                        break;
                  }
                  break;

               default:
                  break;
            }

            /* As soon as it finds the event we are looking for, return */
            if ( (pEvent.eventCategory == category) && (pEvent.eventId == event) )
            {
				#ifdef DEBUG_API
               	PRINT_MSG("SLIC: Found event: pEvent.eventCategory = %d pEvent.eventId = %d \n", 
							pEvent.eventCategory, pEvent.eventId);
				#endif
               return  TRUE;
            }
            else
            {
				#ifdef DEBUG_API
               	PRINT_MSG("SLIC: Unexpected event: pEvent.eventCategory = %d pEvent.eventId = %d \n", 
							pEvent.eventCategory, pEvent.eventId);
				#endif
            }
         }
      }

      mdelay(10);
   }
    
   /* Could not find any events if we reach here */
   return FALSE;
}

/* 
** API	  : rtkGetDev()
** Desp	  :	Get pointer of Realtek Device obj
**          This API defined for functions outside .c
** input  : device global id 
** return : pointer of Realtek Device obj
*/
RTKDevObj * rtkGetDev(int devid)
{
	RTKDevObj *pDev = NULL;

	if (devid < gDevNum)
		pDev = RTKDevList[devid];

	return pDev;
}

/* 
** API	  : rtkGetLine()
** Desp	  :	Get pointer of Realtek line obj
**          This API defined for functions outside .c
** input  : channel global id 
** return : pointer of Realtek line obj
*/
RTKLineObj * rtkGetLine(int chid)
{
    int i, j;
	RTKDevObj *pDev;
	RTKLineObj *pLine;

	for (i=0;i<gDevNum;i++) {
		pDev = RTKDevList[i];
		for (j=0;j<pDev->max_line;j++) {
			pLine = pDev->pLine[j];
			if (pLine->ch_id == chid)
				return pLine;
		}
	}
	
	return NULL;
}

#ifdef DEBUG_INT
int RtkDumpDevLine(int line)
{
	int i,j;
	RTKDevObj *pDev;
	RTKLineObj *pLine;

	printk("Dump Rtk Zarlink device\n");
	for (i=0;i<gDevNum;i++) {
		pDev = RTKDevList[i];
		printk("=======================================================\n");
		printk("dev_id     = 0x%x\n",(int)pDev->dev_id);
		printk("dev_st     = 0x%x\n",pDev->dev_st);
		printk("dev_type   = 0x%x\n",pDev->dev_type);
		printk("max_line   = 0x%x\n",pDev->max_line);
		printk("pDev       = 0x%x\n",(char*)pDev);
		printk("vpDevObj   = 0x%x\n",(char*)pDev->pDevObj);
		printk("vpDevCtx   = 0x%x\n",pDev->pDevCtx);

		for (j=0;j<pDev->max_line;j++) {
			
			pLine = pDev->pLine[j];
			printk("====================================\n");
			printk("\tch_id      = %d\n",(int)pLine->ch_id);
			printk("\tchannelId  = %d\n",pLine->channelId);
			printk("\tline_st    = %d\n",pLine->line_st);
			printk("\tslot_tx    = %d\n",pLine->slot_tx);
			printk("\tslot_rx    = %d\n",pLine->slot_rx);
			printk("\tpLine      = 0x%x\n",pLine);
			printk("\tvpLineObj  = 0x%x\n",pLine->pLineObj);
			printk("\tvpLineCtx  = 0x%x\n",pLine->pLineCtx);
			printk("\tcodec_type = 0x%x\n",pLine->codec_type);
			printk("\tAC_country = 0x%x\n",pLine->AC_country);
			printk("\tpAC_profile= 0x%x\n",pLine->pAC_profile);
			printk("\n");

		}
		printk("\n");
	}	
}
#endif
/*********** End of File ***************/

