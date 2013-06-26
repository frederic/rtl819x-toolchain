#include "vp_api_common.h"

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES
#include "Le880NB_HV.h" /* for 88221/241/266/286 */
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES
//#include "Le890NB_HV.h"
#include "Le890NB_LVHV.h"
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES_WIDEBAND
#ifdef CONFIG_BOARD_011
#include "Le89156_100V_IB_Profile.h"
#endif
#include "Le890WB_HV.h"
#endif
#endif

/* ******************************************** */ 
/*  LE 880 series profile                       */
/* ******************************************** */ 
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES
/* DEV */
#define DEF_LE880_DEV_PROFILE 			LE880_ABS_VBL_FLYBACK

/* FXS */
#define DEF_LE880_AC_PROFILE 			LE880_AC_FXS_RF14_DEF
#define DEF_LE880_DC_PROFILE 			LE880_DC_FXS_DEF
#define DEF_LE880_RING_PROFILE 			LE880_RING_DEF
#define DEF_LE880_RING_CAD_PROFILE 		LE880_RING_CAD_STD

/* FXO */ /* VE880 Does NOT support FXO yet. */
#define DEF_LE880_AC_FXO_LC_PROFILE 	VP_PTABLE_NULL
#define DEF_LE880_FXO_DIALING_PROFILE 	VP_PTABLE_NULL
#endif

/* ******************************************** */ 
/*  LE 890 series profile                       */
/* ******************************************** */ 
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES
#ifndef CONFIG_BOARD_011
#define DEF_LE890_DEV_PROFILE_HV		LE890_DEV_PROFILE_Buck_Boost_HV
#define DEF_LE890_DEV_PROFILE_LV 		LE890_DEV_PROFILE_Buck_Boost_LV
/* FXS */
#define DEF_LE890_AC_PROFILE 			LE890_AC_FXS_RF50_600R_DEF
#define DEF_LE890_DC_PROFILE 			LE890_DC_FXS_DEF
#define DEF_LE890_RING_PROFILE 			LE890_RING_25HZ_DEF
#define DEF_LE890_RING_CAD_PROFILE 		LE890_RING_CAD_STD
#else
#define DEF_LE890_DEV_PROFILE_HV		DEV_PROFILE_Inverting_Boost_100V
#define DEF_LE890_DEV_PROFILE_LV 		DEV_PROFILE_Inverting_Boost_100V
/* FXS */
#define DEF_LE890_AC_PROFILE 			AC_FXS_RF50_600R_DEF
#define DEF_LE890_DC_PROFILE 			DC_FXS_DEF
#define DEF_LE890_RING_PROFILE 			RING_25HZ_DEF
#define DEF_LE890_RING_CAD_PROFILE 		LE890_RING_CAD_STD
#endif


/* FXO */
#define DEF_LE890_AC_FXO_LC_PROFILE 	LE890_AC_FXO_LC_600R_DEF
#define DEF_LE890_FXO_DIALING_PROFILE 	LE890_FXO_DIALING_DEF
#endif

/************** Cadence_Definitions **************/
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES
extern VpProfileDataType LE880_RING_CAD_USER_DEF[]; 	  /* User defined */
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES
extern VpProfileDataType LE890_RING_CAD_USER_DEF[]; 	  /* User defined */
#endif

