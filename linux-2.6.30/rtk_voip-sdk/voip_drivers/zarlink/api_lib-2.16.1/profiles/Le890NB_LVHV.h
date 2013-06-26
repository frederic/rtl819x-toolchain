/*
 * Le890NB_LVHV.h --
 *
 * This header file exports the Profile data types
 *
 * Project Info --
 *   File:   V:\work\VoIP\branches\VoIP-jungle-2.4-1.5\rlxlinux-v2.2\linux-2.6.30\rtk_voip\voip_drivers\zarlink\api_lib-2.16.1\profiles\Le890NB_LVHV.vpw
 *   Type:   VE890 Configuration for 150V Tracking Inverting-Boost Power Supply, Lite Narrowband FXS Coefficients, and 8.192MHz PCLK
 *   Date:   Monday, April 18, 2011 09:45:27
 *   Device: VE890 Le89336
 *
 *   This file was generated with Profile Wizard Version: P2.0.0
 *
 * Project Comments --
 *  --------------------------------------------
 *  Profile Wizard Coefficient Release 2.7 Notes:
 *  --------------------------------------------
 *  Replaced the FXS AC Coefficients for GR57, China, Finland, and Japan.
 *  
 *  --------------------------------------------
 *  Profile Wizard Coefficient Release 2.6 Notes:
 *  --------------------------------------------
 *  I. General:
 *  1. This release adds support for Mexico, Turkey, Thailand, Malaysia, Indonesia, and Ecuador, bringing
 *  the total number of supported countries to 44.  They are:
 *  Argentina (AR), Austria (AT), Australia (AU), Belgium (BE), Bulgaria (BG), Brazil (BR), Canada (CA), Switzerland (CH),
 *  Chile (CL), China (CN), Czech Republic (CZ), Germany (DE), Denmark (DK), Ecuador (EC), Spain (ES), Finland (FI),
 *  France (FR), UK (GB), Greece (GR), Hong Kong SAR China (HK), Hungary (HU), Indonesia (ID), Ireland (IE), Israel (IL),
 *  India (IN), Iceland (IS), Italy (IT), Japan (JP), S. Korea (KR), Mexico (MX), Malaysia (MY), Netherlands (NL),
 *  Norway (NO), New Zealand (NZ), Poland (PL), Portugal (PT), Russian Federation (RU), Sweden (SE), Singapore (SG),
 *  Thailand (TH), Turkey (TK), Taiwan (TW), USA (US), and South Africa (ZA).
 *  2. This release also corrects some Caller ID implementations and signal levels that were incorrect in release 2.3.
 *  3. The coefficients in this and all releases are provided for use only with the Zarlink VoicePath API. Please refer to the terms 
 *  and conditions for licensing the software regarding terms and conditions of usage.
 *  4. This release is for the VE8911-HV chipset and includes coefficients required for the FXS and FXO operation supported by the chipset.
 *  
 *  II. Device Profile:
 *  1. The default settings for the Device Profile are:
 *         PCLK = 8192 kHz
 *         PCM Transmit Edge = Positive
 *         Transmit Time Slot = 0
 *         Receive Time Slot = 0
 *         Interrupt Mode = Open Drain
 *         Switcher = Inverting-Boost (150V)
 *         Driver Tick Rate = 5 ms
 *         Maximum Events / Tick = 2
 *  2. The settings may be changed by the user as necessary.  Please refer to the VE890 and API documentation for information about
 *  the supported settings.
 *  
 *  II. AC Profiles:
 *  1. FXS Coefficients assume -6dBr RX (Output from chipset) and 0dB TX relative gain levels.
 *  2. FXO Coefficients 0dB RX (Output from chipset) and -6dB TX (which means +6dB gain) relative gain levels.
 *  3. Supported countries not individually listed should use the default 600R coefficients, such as LE890_AC_FXS_RF50_600R_DEF and
 *  AC_FXO_LC_600R_DEF.
 *  4. AC FXS Coefficients assume the use of two 25 ohm series resistors or PTCs.
 *  5. This release includes Normal (or narrowband) coefficients for the FXS port.  Wideband coefficients are available upon request.
 *  6. AC FXO Coefficients assume the use of the LC Filter on VE890 DAA Circuit consisting of CIMM and LIMM as shown
 *  in the datasheet.
 *  7. AC FXO Coefficients include the coefficient sets for the VE890 echo free adaptive balance.
 *  
 *  III. DC Profile:
 *  1. The LE890_DC_FXS_DEF Profile is the default used for all countries.  Additional profiles may be created by the user if necessary.
 *  
 *  IV. Ring Profiles:
 *  1. LE890_RING_25HZ_DEF is the default ringing profile and should be used for all countries which do not have a listed ringing profile.  The default
 *  ringing profile is set for a sine wave ringing with an amplitude of 75 Vrms and a frequency of 25 Hz.
 *  2. All ringing profiles on the list have a 75 Vrms ringing level.  
 *  3. DC biasing is 0 for all the sample ringing profiles.
 *  
 *  V. Tone Profiles:
 *  1. These profiles are available only in the full version of the API.
 *  2. The shown levels assume a 6dB attenuation in the chipset before being outputed to line.
 *  3. Call progress tone levels may be arbitrary as they are not always specified in national standards, or the standards may not be available to Zarlink.
 *  4. ITU-T Recommendation E.180 (03/1998) revised on 02/2003 and ETSI TR 101 041-2 V.1.1.1 (05/1997) were used if national standards were not
 *  available.
 *  5. Recommended ETSI ES 201 970 call progress tones are provided for reference.
 *  6. Modulated tones f1 x f2 are approximated as the sum of f1 + (f1+f2)/2 + (f1-f2)/2.
 *  7. The data in these profiles may be changed by the user as necessary.
 *   8. T_CAS_DEF is not a country-specific tone and is used by several national Caller ID profiles.
 *  
 *  V. Cadence Profiles:
 *  1. These profiles are available only in the full version of the API.
 *  2.  ITU-T Recommendation E.180 (03/1998) revised on 02/2003 and ETSI TR 101 041-2 V.1.1.1 (05/1997) were used if national standards were not
 *  available.
 *  3. Recommended ETSI ES 201 970 call progress cadences are provided for reference.
 *  4. Some countries support multiple call progress tone cadences.  The ones used are believed to be representative and most common.  The user may
 *  wish to edit some of the cadence definitions or add additional cadences.
 *  5. Ringing signal cadences include the alerting signal(s) and necessary delays for Type 1 Caller ID, if it is supported below in the Caller ID Profiles.
 *  
 *  VI. Caller ID Profiles:
 *  1. These profiles are available only in the full version of the API.
 *  2. The option to calculate the checksum in the API is selected for all countries except Japan, which requires that the CRC checksum be calculated by
 *  host application.
 *  
 *  VII. FXO/Dialing Profiles:
 *  1. 44 country-specific profiles are provided.  They take into account such variations in national standards as pulse dialing, DTMF dialing, and line
 *  event detection parameters.
 *  2. They may be edited as necessary to meet regulatory requirements.
 *  3. ETSI 203 021 defaults are also provided for reference.
 *  
 *  ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef LE890NB_LVHV_H
#define LE890NB_LVHV_H

#ifdef VP_API_TYPES_H
#include "vp_api_types.h"
#else
typedef unsigned char VpProfileDataType;
#endif


/************** Device_Parameters **************/
extern const VpProfileDataType LE890_DEV_PROFILE_Buck_Boost_HV[];/* Device Configuration Data */
extern const VpProfileDataType LE890_DEV_PROFILE_Buck_Boost_LV[];/* Device Configuration Data */

/************** AC_Coefficients **************/
extern const VpProfileDataType LE890_AC_FXS_RF50_600R_DEF[];/* AC FXS RF50 600R Normal Coefficients (Default)  */
extern const VpProfileDataType LE890_AC_FXS_RF50_ETSI[];/* AC FXS RF50 ETSI ES201 970 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_GR57[];/* AC FXS RF50 Telcordia GR-57 900R+2.16uF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_AT[];/* AC FXS RF50 Austria 220R+820R//115nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_AU[];/* AC FXS RF50 Australia 220R+820R//120nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_BE[];/* AC FXS RF50 Belgium 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_BG[];/* AC FXS RF50 Bulgaria 220R+820R//115nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_CH[];/* AC FXS RF50 Switzerland 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_CN[];/* AC FXS RF50 China 200R+680R//100nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_DE[];/* AC FXS RF50 Germany 220R+820R//115nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_DK[];/* AC FXS RF50 Denmark 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_ES[];/* AC FXS RF50 Spain 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_FI[];/* AC FXS RF50 Finland 270R+910R//120nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_FR[];/* AC FXS RF50 France 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_GB[];/* AC FXS RF50 UK 370R+620R//310nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_HU[];/* AC FXS RF50 Hungary 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_IE[];/* AC FXS RF50 Ireland 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_IT[];/* AC FXS RF50 Italy 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_JP[];/* AC FXS RF50 Japan 600R+1uF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_NL[];/* AC FXS RF50 Netherlands 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_NO[];/* AC FXS RF50 Norway 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_NZ[];/* AC FXS RF50 New Zealand 370R+620R//310nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_PT[];/* AC FXS RF50 Portugal 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_SE[];/* AC FXS RF50 Sweden 270R+750R//150nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_ZA[];/* AC FXS RF50 South Africa 220R+820R//115nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_600R_DEF[];/* AC FXO LC Filter 600R Normal ABF Coefficients (Default) */
extern const VpProfileDataType LE890_AC_FXO_LC_ETSI[];/* AC FXO LC Filter ETSI ES203 021 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_TBR21_CR[];/* AC FXO LC Filter TBR21 with Current Limit 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_GR57[];/* AC FXO LC Filter Telcordia GR-57 600R Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_AT[]; /* AC FXO LC Filter Austria 220R+820R//115nF Normal Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_AU[]; /* AC FXO LC Filter Australia 220R+820R//120nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_BE[]; /* AC FXO LC Filter Belgium 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_BG[]; /* AC FXO LC Filter Bulgaria 220R+820R//115nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_CH[]; /* AC FXO LC Filter Switzerland 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_CN[]; /* AC FXO LC Filter China 200R+680R//100nF Coefficients (Voice Applications) */
extern const VpProfileDataType LE890_AC_FXO_LC_CN2[];/* AC FXO LC Filter China 600R Normal ABF Coefficients (Modem Applications) */
extern const VpProfileDataType LE890_AC_FXO_LC_DE[]; /* AC FXO LC Filter Germany 220R+820R//115nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_DK[]; /* AC FXO LC Filter Denmark 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_ES[]; /* AC FXO LC Filter Spain 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_FI[]; /* AC FXO LC Filter Finland 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_FR[]; /* AC FXO LC Filter France 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_GB[]; /* AC FXO LC Filter UK 370R+620R//310nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_HK[]; /* AC FXO LC Filter Hong Kong SAR 600R ABF Coefficients - 4 KHz Return Loss */
extern const VpProfileDataType LE890_AC_FXO_LC_HU[]; /* AC FXO LC Filter Hungary 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_IE[]; /* AC FXO LC Filter Ireland 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_IT[]; /* AC FXO LC Filter Italy 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_JP[]; /* AC FXO LC Filter Japan 600R Lowest ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_MY[]; /* AC FXO LC Filter Malaysia 600R Low ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_NL[]; /* AC FXO LC Filter Netherlands 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_NO[]; /* AC FXO LC Filter Norway 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_NZ[]; /* AC FXO LC Filter New Zealand 370R+620R//310nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_PT[]; /* AC FXO LC Filter Portugal 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_SE[]; /* AC FXO LC Filter Sweden 270R+750R//150nF Normal ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_TW[]; /* AC FXO LC Filter Taiwan 600R Low ABF Coefficients */
extern const VpProfileDataType LE890_AC_FXO_LC_ZA[]; /* AC FXO LC Filter South Africa 220R+820R//115nF Normal ABF Coefficients */

/************** DC_Parameters **************/
extern const VpProfileDataType LE890_DC_FXS_DEF[];   /* DC FXS Default -- Use for for all countries unless country file exists */

/************** Ring_Parameters **************/
extern const VpProfileDataType LE890_RING_25HZ_DEF[];/* Default Ringing 25 Hz 75 Vrms Tracking - Use for all countries unless country file exists */
extern const VpProfileDataType LE890_RING_AT[];      /* Austria Ringing 50 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_CA[];      /* Canada Ringing 20 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_FI[];      /* Finland Ringing 50 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_FR[];      /* France Ringing 50 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_HK[];      /* Hong Kong SAR Ringing 20 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_JP[];      /* Japan Ringing 16 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_KR[];      /* Korea Ringing 20 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_SG[];      /* Singapore Ringing 24 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_TW[];      /* Taiwan Ringing 20 Hz 75 Vrms Tracking */
extern const VpProfileDataType LE890_RING_US[];      /* USA Ringing 20 Hz 75 Vrms Tracking */

/************** Call_Progress_Tones **************/

/************** Cadence_Definitions **************/
extern const VpProfileDataType LE890_RING_CAD_STD[]; /* Standard Ringing Cadence (2 sec on 4 sec off) */
extern const VpProfileDataType LE890_RING_CAD_SHORT[];/* Short Ringing Cadence (0.5 sec on 0.5 sec off) */

/************** Caller_ID **************/

/************** LE890_FXO_Dialing_Profile **************/
extern const VpProfileDataType LE890_FXO_DIALING_DEF[];/* Default FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_ETSI[];/* ETSI ES203 021 FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_AR[];/* Argentina FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_AT[];/* Austria FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_AU[];/* Australia FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_BE[];/* Belgium FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_BG[];/* Bulgaria FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_BR[];/* Brazil FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_CA[];/* Canada FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_CH[];/* Switzerland FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_CL[];/* Chile FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_CN[];/* China FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_CZ[];/* Czech Republic FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_DE[];/* Germany FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_DK[];/* Denmark FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_EC[];/* Ecuador FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_ES[];/* Spain FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_FI[];/* Finland FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_FR[];/* France FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_GB[];/* UK FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_GR[];/* Greece FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_HK[];/* Hong Kong SAR FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_HU[];/* Hungary FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_ID[];/* Indonesia FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_IE[];/* Ireland FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_IL[];/* Israel FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_IN[];/* India FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_IS[];/* Iceland FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_IT[];/* Italy FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_JP[];/* Japan FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_KR[];/* S. Korea FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_MX[];/* Mexico FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_MY[];/* Malaysia FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_NL[];/* Netherlands FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_NO[];/* Norway FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_NZ[];/* New Zealand FXO/Dialing  */
extern const VpProfileDataType LE890_FXO_DIALING_PL[];/* Poland FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_PT[];/* Portugal FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_RU[];/* Russia FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_SE[];/* Sweden FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_SG[];/* Singapore FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_TH[];/* Thailand FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_TK[];/* Turkey FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_TW[];/* Taiwan FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_US[];/* USA FXO/Dialing */
extern const VpProfileDataType LE890_FXO_DIALING_ZA[];/* South Africa FXO/Dialing */

#endif /* LE890NB_LVHV_H */

