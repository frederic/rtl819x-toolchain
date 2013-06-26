/*
 * jwsyu@realtek.com
 * 2011/05/11 AM 09:53:51
 * version 01
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "voip_manager.h"

/**
 * @ingroup VOIP_DSP
 * @brief The purpose of this sample code is designed to allow the programmer easily development when using AGC feature
 * @param cmd: gain_test <test_mode>
 * @param test_mode: 1-5
 * @param 1: Mute the SPK and Mic 10 seconds
 * @param 2: Set the SPK and Mic gain to -6dB(cut 6dB)respectively 
 * @param 3: Set the SPK and Mic gain to +6dB(boost 6dB)respectively 
 * @param 4: Set the SPK and Mic target level to level 0 and max gain-up = 5dB and max gain-down = -5dB 
 * @param 5: Set the SPK and Mic target level to level 4 and max gain-up = 5dB and max gain-down = -5dB
 * @retval 0 Success
 */
void ShowUsage(char *cmd)
{
	printf("usage: %s <test_mode> \n" \
	       " - test_mode => 1: mute spk & mic 10sec \n" \
	       "               2: spk & mic gain = -6db 10sec \n" \
	       "              3: spk & mic gain = +6db 10sec \n" \
         "             4: agc lvl=0, max_gun=+5db, max_gdown=-5db 20sec\n" \
	       "            5: agc lvl=4, max_gun=+5db, max_gdown=-5db 20sec\n", cmd); 
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int test_mode;

	if (argc < 2)
		ShowUsage(argv[0]);

	test_mode = atoi(argv[1]);

	switch (test_mode)
	{
	case 1:
		rtk_Set_Voice_Gain(0, -32, -32);	// channel[0], Mute the speaker and the microphone	
		sleep(10);												// duration: 10 seconds
		rtk_Set_Voice_Gain(0, 0, 0);			// channel[0], restore the default volume level of the speaker and Mic
		break;
	case 2:
		rtk_Set_Voice_Gain(0, -6, -6);	// set the SPK and mic gain to -6dB (cut 6dB) in the channel 0		
		sleep(10);											
		rtk_Set_Voice_Gain(0, 0, 0);		// restore the SPK and mic gain to 0dB in the channel 0		
		break;
	case 3:
		rtk_Set_Voice_Gain(0, +6, +6);	// set the SPK and mic gain to 6dB (boost 6dB) in the channel 0		
		sleep(10);
		rtk_Set_Voice_Gain(0, 0, 0);		// restore the SPK and mic gain to 0dB in the channel 0		
		break;
	case 4:
		rtk_Set_SPK_AGC_LVL(0, 0); 		// set the AGC target level of the SPK to level 0  
		rtk_Set_SPK_AGC_GUP(0, 4);  	// set the AGC gain-up of the SPK to level 4  
		rtk_Set_SPK_AGC_GDOWN(0, 4);	// set the AGC gain-down of the SPK to level 4  
		rtk_Set_SPK_AGC(0, 1, 55);				// Enable the speaker AGC in channel number 0
		
		rtk_Set_MIC_AGC_LVL(0, 0); 		// set the AGC target level of the mic to level 0
		rtk_Set_MIC_AGC_GUP(0, 4); 		// set the AGC gain-up of the mic to level 4     
		rtk_Set_MIC_AGC_GDOWN(0, 4);	// set the AGC gain-down of the mic to level 4   	 
		rtk_Set_MIC_AGC(0, 1, 55); 				// Enable the mic AGC in channel number 0    		
		sleep(20);
		rtk_Set_SPK_AGC(0, 0, 55); 				// Disable the speaker AGC
		rtk_Set_MIC_AGC(0, 0, 55);				// Disable the mic AGC
		break;
	case 5:
		rtk_Set_SPK_AGC_LVL(0, 4); 		// set the AGC target level of the SPK to level 4   
		rtk_Set_SPK_AGC_GUP(0, 4); 		// set the AGC gain-up of the SPK to level 4        
		rtk_Set_SPK_AGC_GDOWN(0, 4); 	// set the AGC gain-down of the SPK to level 4      	
		rtk_Set_SPK_AGC(0, 1, 55);      	// Enable the speaker AGC in channel number 0       	
		                                                                                  
		rtk_Set_MIC_AGC_LVL(0, 4); 		// set the AGC target level of the mic to level 4   	
		rtk_Set_MIC_AGC_GUP(0, 4); 		// set the AGC gain-up of the mic to level 4        
		rtk_Set_MIC_AGC_GDOWN(0, 4); 	// set the AGC gain-down of the mic to level 4   	  	
		rtk_Set_MIC_AGC(0, 1, 55); 				// Enable the mic AGC in channel number 0    		    
		sleep(20);										                                                    	
		rtk_Set_SPK_AGC(0, 0, 55);     		// Disable the speaker AGC                          	
		rtk_Set_MIC_AGC(0, 0, 55);				// Disable the mic AGC                              	

	}

		printf("test_finish\n");
	return 0;
}








