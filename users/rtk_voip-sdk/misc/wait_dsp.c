#include <stdio.h>
#include "voip_feature.h"
#include "voip_manager.h"

int main()
{
	int i;
	int dsp_nr;
	
	// TH: add for ethernet DSP booting
	if (RTK_VOIP_ETHERNET_DSP_HOST_CHECK(g_VoIP_Feature))
	{
		dsp_nr = RTK_VOIP_DSP_DEVICE_NUMBER( g_VoIP_Feature );
		
		for (i=0; i < dsp_nr; i++)
		{
			printf("Wait DSP %d/%d booting ......\n", i, dsp_nr);
			fflush(stdout);//force above message to display before while loop
			while( rtk_CheckDspAllSoftwareReady(i) == 0 );
			printf("DSP %d Software Ready!\n", i);
			if (i == 0)
				rtk_SetDspIdToDsp(15);  // force DSP to gen mido interrupt
			rtk_SetDspIdToDsp(i);
		}
		
		rtk_CompleteDeferInitialzation();
	}
	
	return 0;
}

