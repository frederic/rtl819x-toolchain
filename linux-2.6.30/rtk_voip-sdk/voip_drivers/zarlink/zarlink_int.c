/** \file zarlink_int.c
 * 
 *
 * This file contains the major process of zarlink slic
 * 
 *
 * Copyright (c) 2010, Realtek Semiconductor, Inc.
 *
 */
#if defined( CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES )
#include "ve880_int.h"
#endif

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES )
#include "ve890_int.h"
#endif

extern void RST_Slic(void);

/* 
** API	  : ZarlinkInit()
** Desp	  :	Init Realtek dev and line obj 
**          Init Zarlink dev and line obj
** input  : pcm_mode
** return : SUCCESS/FAILED
*/
int ZarlinkInit(int pcm_mode)
{
	/* 
 	 * All SLICs are sharing the same reset pin. 
 	 * Only RESET one time! 
 	 * */
	RST_Slic();

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES
	Ve880Init(pcm_mode);
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES
	Ve890Init(pcm_mode);
#endif

	return SUCCESS;
}

RTKLineObj * RtkGetLine(int chid)
{
	return(rtkGetLine(chid));
}

RTKDevObj * RtkGetDev(int devid)
{
	return rtkGetDev(devid);
}
/*********** End of File ***************/

