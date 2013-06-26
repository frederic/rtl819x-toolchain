/** \file Ve890_api.c
 * 
 *
 * This file contains the major api for upper application
 * 
 *
 * Copyright (c) 2010, Realtek Semiconductor, Inc.
 *
 */
#include "ve890_api.h"
#include "Ve_profile.h"
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK_ON_NEW_ARCH
#include "snd_define.h"
#else
#include "Slic_api.h" 	/* for COUNTRY_USA ... */
#endif

#undef DEBUG_API 

#if defined(DEBUG_API)
#define DEBUG_API_PRINT() printk("%s(%d) line #%d\n",__FUNCTION__,__LINE__,pLine->ch_id);
#else
#define DEBUG_API_PRINT()
#endif

VpStatusType Ve890SetRingCadenceProfile(RTKLineObj *pLine, uint8 ring_cad)
{
	VpStatusType status;
	VpProfilePtrType ring_cadence = DEF_LE890_RING_CAD_PROFILE;

	DEBUG_API_PRINT();

	switch (ring_cad)
	{
		case 0:
			ring_cadence = LE890_RING_CAD_STD;
			PRINT_MSG("set LE890_RING_CAD_STD\n");
			break;
		case 1:
			ring_cadence = LE890_RING_CAD_SHORT;
			PRINT_MSG("set LE890_RING_CAD_SHORT\n");
			break;
		default:
			ring_cadence = LE890_RING_CAD_STD;
			PRINT_MSG("set LE890_RING_CAD_STD\n");
			break;
	}

	status = VpInitRing( pLine->pLineCtx, 
						 ring_cadence,
						 VP_PTABLE_NULL);

	if ( status == VP_STATUS_SUCCESS )
		pLine->pRing_cad_profile = ring_cadence;

	return status;
}

VpStatusType Ve890SetImpedenceCountry(RTKLineObj *pLine, uint8 country)
{
	VpStatusType status;
	VpProfilePtrType AC_profile;

	DEBUG_API_PRINT();

	PRINT_MSG(" <<<<<<<<< %s Country %d >>>>>>>>>\n",__FUNCTION__, country);

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
	if (pLine->codec_type == VP_OPTION_WIDEBAND) 
		PRINT_MSG("line#%d VP_OPTION_WIDEBAND\n",pLine->ch_id);
	else
#endif
		PRINT_MSG("line#%d VP_OPTION_NARROWBAND\n",pLine->ch_id);

	switch(country)
	{
		case COUNTRY_AUSTRALIA:	
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND) 
				AC_profile = LE890_AC_FXS_RF50_WB_AU;
			else	
#endif
				AC_profile = LE890_AC_FXS_RF50_AU;
			break;

		case COUNTRY_BE:	/* Belgium*/
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_BE;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_BE;
			break;
			
		case COUNTRY_CN:	/* China  */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_CN;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_CN;
			break;
		
		case COUNTRY_GR:	/* German */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_DE;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_DE;
			break;

		case COUNTRY_FL:	/* Finland*/
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_FI;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_FI;
			break;
			
		case COUNTRY_FR:	/* France */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_FR;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_FR;
			break;
			
		case COUNTRY_IT:	/* Italy  */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_IT;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_IT;
			break;
			
		case COUNTRY_JP:	/* Japan  */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_JP;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_JP;
			break;
			
		case COUNTRY_SE:	/* Sweden */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_SE;
			else
#endif
				AC_profile = LE890_AC_FXS_RF50_SE;
			break;

		case COUNTRY_HK:
		case COUNTRY_TW:	
		case COUNTRY_UK:
		case COUNTRY_USA:
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_600R_DEF;
			else
#endif
				AC_profile = DEF_LE890_AC_PROFILE;
			PRINT_MSG("Set SLIC impedance to 600 ohm.\n");
			break;

		default:
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = LE890_AC_FXS_RF50_WB_600R_DEF;
			else
#endif
				AC_profile = DEF_LE890_AC_PROFILE;
			PRINT_Y("country wasn't defined. \
						Set to default SLIC impedance 600 ohm.\n");
			break;
	}

	status = VpConfigLine( pLine->pLineCtx, 
						   AC_profile,
						   VP_PTABLE_NULL,
						   VP_PTABLE_NULL );

	if ( status == VP_STATUS_SUCCESS ) {
		pLine->pAC_profile = AC_profile;
		pLine->AC_country  = country;
	}
	
	return status;
}

VpStatusType Ve890SetFxsAcProfileByBand(RTKLineObj *pLine, int pcm_mode)
{
    VpStatusType status;

	DEBUG_API_PRINT();
    /* pcm_mode ??*/

	status = Ve890SetImpedenceCountry(pLine, pLine->AC_country);
	return status;
    
}

VpStatusType Ve890SetIODir(RTKLineObj *pLine, VPIO IO, int bOut)
{
	unsigned char reg = 0x54;
	unsigned char regdir = 0;
	unsigned char len = 1;

	DEBUG_API_PRINT();

	#ifdef DEBUG_API
	printk("%s(%d)dir=0x%08X, IO=%d, bOut=%d\n",
		__FUNCTION__,__LINE__,pLine->pDev->gpio_dir,IO,bOut);
	#endif

	regdir = (unsigned char)pLine->pDev->gpio_dir;
	/* available bit: bit4, bit3, bit2, x, bit0 */
	/* bit 1 was reserved by Zarlink */
	if (bOut==1) {
		if (IO == VPIO_IO1)
			regdir |= IO;
		else
			regdir |= (IO<<1);

	} else {
		if (IO == VPIO_IO1)
			regdir &= ~IO;
		else
			regdir &= ~(IO<<1);
	}

	VpRegisterReadWrite(pLine->pLineCtx, reg, &len, &regdir);
	pLine->pDev->gpio_dir = regdir;

	#ifdef DEBUG_API
	printk("%s(%d)dir=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dir);
	#endif
}

VpStatusType Ve890SetIOState(RTKLineObj *pLine, VPIO IO, int bHigh )
{
	unsigned int gpio_new_dat;
    
	DEBUG_API_PRINT();

	#ifdef DEBUG_API
	printk("%s(%d)dat=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dat);
	#endif

	gpio_new_dat = pLine->pDev->gpio_dat & GPIO_NEW_MASK;

	if (bHigh==1)
		pLine->pDev->gpio_dat |= IO;
	else
		pLine->pDev->gpio_dat &= ~IO;

	#ifdef DEBUG_API
	printk("%s(%d)dat=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dat);
	#endif
}

VpStatusType Ve890GetIOState(RTKLineObj *pLine, VPIO IO, int *bHigh)
{
	unsigned char reg=0x53;
	unsigned char len=1;
	unsigned char regdat;
    
	DEBUG_API_PRINT();

	/* Read data from SLIC */
	VpRegisterReadWrite(pLine->pLineCtx, reg, &len, &regdat);

	*bHigh = (regdat & IO ? 1:0);

	#ifdef DEBUG_API
	printk("IO=0x%x, regdat=%d, bHigh=%d\n",IO, regdat, *bHigh);
	#endif
}

VpStatusType Ve890UpdIOState(RTKLineObj *pLine)
{
	unsigned char reg = 0x52;
	unsigned int gpio_new_dat;
	unsigned int gpio_cur_dat;
	unsigned char len=1;
	unsigned char regdat;
    
	if (pLine->line_st != LINE_S_READY)
		return;

	gpio_new_dat = pLine->pDev->gpio_dat & GPIO_NEW_MASK;
	gpio_cur_dat = pLine->pDev->gpio_dat & GPIO_CUR_MASK;
	gpio_cur_dat = gpio_cur_dat >> 16;

	#if 0
	/* noisy! should be called every 10ms */
	DEBUG_API_PRINT(); 

	printk("%s(%d)pLine=0x%08X, pLineCtx=0x%08X\n",
			__FUNCTION__,__LINE__,pLine, pLine->pLineCtx);
	printk("%s(%d)cur=0x%08X, new=0x%08X, channelId=%d\n",
			__FUNCTION__,__LINE__,gpio_cur_dat,gpio_new_dat,pLine->channelId);
	#endif

	/* update if dat was changed */
	if (gpio_cur_dat != gpio_new_dat) {

		regdat = (unsigned char)gpio_new_dat;

		/* write register */
		VpRegisterReadWrite(pLine->pLineCtx, reg, &len, &regdat);

		gpio_cur_dat = gpio_new_dat; 	/* update new to current */
		pLine->pDev->gpio_dat = ((gpio_cur_dat<<16) | gpio_new_dat);

		#ifdef DEBUG_API
		printk("%s(%d)dat=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dat);
		#endif
	}
}
/*****************************************************************/
#if 0
VpProfilePtrType Ve890RingProfile(uint8 profileId)
{
    VpProfilePtrType ring_profile = VP_PTABLE_NULL;                                                        
	PRINT_Y("%s(%d) Not support yet!\n",__FUNCTION__,profileId);
	return ring_profile;
}
#endif

#if 0
VpProfilePtrType Ve890AcProfile(uint8 profileId)
{
    VpProfilePtrType AC_profile = DEF_LE890_AC_PROFILE;

	PRINT_Y("%s(%d) Not support yet!\n",__FUNCTION__,profileId);
    return AC_profile;
}   
#endif

#ifdef DEBUG_API
int Ve890_ver(int deviceId)
{
	#if 1 // read revision
    unsigned char res[14]={0};
    int i;
    uint8 reg, len;
        
    reg = 0x73;
    len= 2;
    VpMpiCmd(deviceId, 0x3, reg, len, res);
    printk("Revision: ");
        
    for (i=0; i<len; i++)
        printk("\nbyte%d = 0x%x", i, res[i]);
    printk("\n");
	#endif

	return 0;
}
#endif


