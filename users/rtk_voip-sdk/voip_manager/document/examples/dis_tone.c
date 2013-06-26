#include "voip_manager.h"

/**
 * @brief This sample codes show how you to set dis-connect tone detection parameters.<br>
 *        In this codes, it sets two kind of tones as dis-connect tone parameters.<br>
 *        If these two tone are detected at FXO port, DSP will report dis-connect tone event.
 * @author thlin@realtek.com, version 0.1 @ 2011/06/01
 * @see TstVoipdistonedet_parm do_mgr_VOIP_MGR_SET_DIS_TONE_DET
 */
int main(void)
{
	voipCfgParam_t voip_ptr;

	voip_ptr.distone_num = 2;
	
	// For 1st Set, to detect USA Stutter Dial Tone (350Hz+440Hz, 0.1s on, 0.1s off)
	voip_ptr.d1Freq1 = 350;
	voip_ptr.d1Freq2 = 440;
	voip_ptr.d1Accur = 13;
	voip_ptr.d1Level = 700;
	voip_ptr.d1ONup = 15;
	voip_ptr.d1ONlow = 5;
	voip_ptr.d1OFFup = 15;
	voip_ptr.d1OFFlow = 5;
	
	// For 2nd Set, to detect USA Busy Tone (480Hz+620Hz, 0.5s on, 0.5s off)
	voip_ptr.d2Freq1 = 480;
	voip_ptr.d2Freq2 = 620;
	voip_ptr.d2Accur = 16;
	voip_ptr.d2Level = 800;
	voip_ptr.d2ONup = 54;
	voip_ptr.d2ONlow = 46;
	voip_ptr.d2OFFup = 54;
	voip_ptr.d2OFFlow = 46;

	rtk_Set_Dis_Tone_Para(&voip_ptr);
	return 0;
}

