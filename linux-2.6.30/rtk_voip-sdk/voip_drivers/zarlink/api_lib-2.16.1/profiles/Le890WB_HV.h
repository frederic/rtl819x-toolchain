/*
 * Le890WB_HV.h --
 *
 * This header file exports the Profile data types
 *
 * Project Info --
 *   File:   \\172.21.69.20\vincent\work\VoIP\trunk\VoIP-jungle-2.4\rlxlinux-v2.2\linux-2.6.30\rtk_voip\voip_drivers\zarlink\api_lib-2.16.1\profiles\Le890WB_HV.vpw
 *   Type:   VE890 Configuration for 100V Buck-Boost Power Supply, Full Wideband Coefficients, and 8.192MHz PCLK
 *   Date:   Wednesday, March 23, 2011 16:33:56
 *   Device: VE890 Le89116
 *
 *   This file was generated with Profile Wizard Version: P2.0.0
 *
 * Project Comments --
 *  --------------------------------------------
 *  Profile Wizard Coefficient Release 2.7 Notes:
 *  --------------------------------------------
 *  Corrected all AC FXS Profiles.
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
 *  4. This release is for the VE8910 chipset and includes coefficients required for FXS operation.
 *  
 *  II. Device Profile:
 *  1. The default settings for the Device Profile are:
 *         PCLK = 8192 kHz
 *         PCM Transmit Edge = Positive
 *         Transmit Time Slot = 0
 *         Receive Time Slot = 0
 *         Interrupt Mode = Open Drain
 *         Switcher = Buck-Boost
 *         Driver Tick Rate = 5 ms
 *         Maximum Events / Tick = 2
 *  2. The settings may be changed by the user as necessary.  Please refer to the VE890 and API documentation for information about
 *  the supported settings.
 *  
 *  II. AC Profiles:
 *  1. FXS Coefficients assume -6dBr RX (Output from chipset) and 0dB TX relative gain levels.
 *  2. Supported countries not individually listed should use the default 600R profile AC_FXS_RF50_WB_600R_DEF.
 *  4. AC FXS Coefficients assume the use of two 25 ohm series resistors or PTCs.
 *  5. This release includes wideband coefficients for the FXS port.  Narrowband (or Normal) settings are available upon request.
 *  
 *  III. DC Profile:
 *  1. The LE890_DC_FXS_DEF Profile is the default used for all countries.  Additional profiles may be created by the user if necessary.
 *  
 *  IV. Ring Profiles:
 *  1. LE890_RING_25HZ_DEF is the default ringing profile and should be used for all countries which do not have a listed ringing profile.  The default
 *  ringing profile is set for a sine wave ringing with an amplitude of 50 Vrms and a frequency of 25 Hz.
 *  2. All ringing profiles on the list have a 50 Vrms ringing level.  
 *  3. DC biasing is set to 0 in the sample ringing profiles.
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
 *  1. Not included.
 *  
 *  ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef LE890WB_HV_H
#define LE890WB_HV_H

#ifdef VP_API_TYPES_H
#include "vp_api_types.h"
#else
typedef unsigned char VpProfileDataType;
#endif


/************** Device_Parameters **************/
extern const VpProfileDataType LE890_DEV_PROFILE_Buck_Boost[];/* Device Configuration Data */

/************** AC_Coefficients **************/
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_600R_DEF[];/* AC FXS RF50 600R Wideband Coefficients (Default) */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_ETSI[];/* AC FXS RF50 ETSI ES201 970 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_GR57[];/* AC FXS RF50 Telcordia GR-57 900R+2.16uF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_AT[];/* AC FXS RF50 Austria 220R+820R//115nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_AU[];/* AC FXS RF50 Australia 220R+820R//120nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_BE[];/* AC FXS RF50 Belgium 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_BG[];/* AC FXS RF50 Bulgaria 220R+820R//115nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_CH[];/* AC FXS RF50 Switzerland 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_CN[];/* AC FXS RF50 China 200R+680R//100nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_DE[];/* AC FXS RF50 Germany 220R+820R//115nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_DK[];/* AC FXS RF50 Denmark 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_ES[];/* AC FXS RF50 Spain 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_FI[];/* AC FXS RF50 Finland 270R+910R//120nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_FR[];/* AC FXS RF50 France 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_GB[];/* AC FXS RF50 UK 370R+620R//310nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_HU[];/* AC FXS RF50 Hungary 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_IE[];/* AC FXS RF50 Ireland 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_IT[];/* AC FXS RF50 Italy 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_JP[];/* AC FXS RF50 Japan 600R+1uF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_NL[];/* AC FXS RF50 Netherlands 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_NO[];/* AC FXS RF50 Norway 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_NZ[];/* AC FXS RF50 New Zealand 370R+620R//310nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_PT[];/* AC FXS RF50 Portugal 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_SE[];/* AC FXS RF50 Sweden 270R+750R//150nF Wideband Coefficients */
extern const VpProfileDataType LE890_AC_FXS_RF50_WB_ZA[];/* AC FXS RF50 South Africa 220R+820R//115nF Wideband Coefficients */

/************** DC_Parameters **************/

/************** Ring_Parameters **************/

/************** Call_Progress_Tones **************/

/************** Cadence_Definitions **************/

/************** Caller_ID **************/

#endif /* LE890WB_HV_H */

