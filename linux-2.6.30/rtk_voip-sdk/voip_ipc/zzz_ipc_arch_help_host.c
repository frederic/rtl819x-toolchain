#include <linux/init.h>
#include <asm/uaccess.h>
#include "voip_types.h"
//#include "type.h"
#include "voip_control.h"
#include "voip_init.h"
#include "rtk_voip.h"
#include "../voip_manager/voip_mgr_define.h"

#include "ipc_arch_tx.h"

#if 0

extern int voip_ch_num;
extern int sess_num;

/********* Help Host to transfer Host chid to DSP ID and DSP chid ******/

/* Note: 
 * 	Host Channel can be FXS or FXO.
 *	If Host Channel Numbers are 4, there are different combination as below:
 *	4FXS, or 3FXS+1FXO, or 2FXS+2FXO, or 1FXS+3FXO.
 */


/*======== For 8 Host Channel =========*/
#if (HOST_CH_NUM == 8)
#if (DSP_DEVICE_NUM == 1)
/* DSP0(8S) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {8};
#elif (DSP_DEVICE_NUM == 2)
/* DSP0(4S) + DSP1(4S) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 0, 0, 1, 1, 1, 1};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {4, 4};
#elif (DSP_DEVICE_NUM == 4)
/* DSP0(2S) + DSP1(2S) + DSP2(2S) + DSP3(2S) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 1, 1, 2, 2, 3, 3};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {2, 2, 2, 2};
#endif
#endif

/*======== For 5 Host Channel =========*/
#if (HOST_CH_NUM == 5)
#if (DSP_DEVICE_NUM == 1)
/* DSP0(4S) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 0, 0, 0};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {5};
#endif
#endif

/*======== For 4 Host Channel =========*/
#if (HOST_CH_NUM == 4)
#if (DSP_DEVICE_NUM == 1)
/* DSP0(4S) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 0, 0};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {4};
#elif (DSP_DEVICE_NUM == 2)
/* DSP0(2S) + DSP1(2S) configuration */
/* DSP0(2S) + DSP1(1S1O) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 1, 1};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {2, 2};
#endif
#endif

/*======== For 2 Host Channel =========*/
#if (HOST_CH_NUM == 2)
#if (DSP_DEVICE_NUM == 1)
/* DSP0(2S) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {2};
#elif (DSP_DEVICE_NUM == 2)
/* DSP0(1S) + DSP1(1S) configuration */
unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 1};
unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {1};
#endif
#endif


/*======== For 6 Host Channel =========*/
#if (HOST_CH_NUM == 6)
/* For 6 FXS: DSP0(2S) + DSP1(4S) configuration */
//#define DSP_DEVICE_NUM 2
//unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 1, 1, 1, 1};
//unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {2, 4};

/* For 6 FXS: DSP0(4S) + DSP1(2S) configuration */
//#define DSP_DEVICE_NUM 2
//unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 0, 0, 1, 1};
//unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {4, 2};

/* For 6 FXS: DSP0(2S) + DSP1(2S) + DSP2(2S) configuration */
//#define DSP_DEVICE_NUM 3
//unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 1, 1, 2, 2};
//unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {2, 2, 2};
#endif

/* For 3 FXS 1 FXO: DSP0(2S) + DSP1(1S1O) configuration */
//#define DSP_DEVICE_NUM 2
//unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 1, 1};
//unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {2, 2};

/* For 3 FXS: DSP0(2S) + DSP1(1S) configuration */
//#define DSP_DEVICE_NUM 2
//unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 1};
//unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {2, 1};

/* For 3 FXS: DSP0(1S) + DSP1(2S) configuration */
//#define DSP_DEVICE_NUM 2
//unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 1, 1};
//unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {1, 2};

/* For 2 FXS 1 FXO: DSP0(2S1O) configuration */
//#define DSP_DEVICE_NUM 1
//unsigned char HostChannelToDspIdTable[HOST_CH_NUM] = {0, 0, 0};
//unsigned char DspIdToDspChannelNum[DSP_DEVICE_NUM] = {3};

#endif

/*************************************************************************/

#if 0
unsigned short API_get_DSP_ID(int cmd, int host_chid)
{
	if ((host_chid < 0) || (host_chid > voip_ch_num))
		PRINT_R("1: Error host chid%d, cmd=%d\n", host_chid, cmd);

	//PRINT_R("h%d->id%d\n", host_chid, HostChannelToDspIdTable[host_chid]);
	return HostChannelToDspIdTable[host_chid];
}

unsigned int API_get_DSP_CH(int cmd, int host_chid)
{
	int id = 0, dsp_id, tmp = 0;
	if ((host_chid < 0) || (host_chid > voip_ch_num))
		PRINT_R("2: Error host chid%d, cmd=%d\n", host_chid, cmd);

	dsp_id = HostChannelToDspIdTable[host_chid];

	for (id=0; id < dsp_id; id++)
		tmp += DspIdToDspChannelNum[id];
	
	//PRINT_R("h%d->ch%d\n", host_chid, host_chid - tmp);
	return (host_chid - tmp);
}

unsigned int API_get_Host_CH(unsigned short dsp_id, int dsp_chid)
{
	int id = 0, tmp = 0;
	
	for (id=0; id < dsp_id; id++)
		tmp += DspIdToDspChannelNum[id];
	
	//PRINT_R("(%d,%d)->h%d\n", dsp_id, dsp_chid, tmp + dsp_chid);
	return (tmp + dsp_chid);
}

unsigned int API_get_DSP_NUM(void)
{
	return DSP_DEVICE_NUM;
}

#endif

/****** sid API: The same API to the dsp_main.c *******/
// for chid and mid transfer to sid


/* For DSP */

#if 0
unsigned int ChNumPerDsp[DSP_DEVICE_NUM] = {0};
unsigned int SessNumPerDsp[DSP_DEVICE_NUM] = {0};
#endif

//#define SESS_NUM_DSP	2*CHNUM_PER_DSP

/************************************************************************/


/**********************************************/

#if 0
void __init ipc_arch_is_host_init(void)
{
	//int i = 0, j = 0;
	//int j;
	
	//for(i=0; i<sess_num; i++)
	//{
	//	nSidTbl[i] = SESSION_NULL;
	//}
	

#if 0
	for (j=0; j<DSP_DEVICE_NUM; j++)
	{
		//for(i=0; i<MAX_SESS_NUM; i++)
		//{
		//	nSidTbl_dsp[j][i] = SESSION_NULL;
		//}
	
		PRINT_Y("DSP%d: \n", j);
		ChNumPerDsp[j] = DspIdToDspChannelNum[j];
		SessNumPerDsp[j] = 2*ChNumPerDsp[j];
		PRINT_Y("ChNum= %d, SesNum= %d\n", ChNumPerDsp[j], SessNumPerDsp[j]);
	}
#endif

#if 0	
	PRINT_Y("--- nSidTbl_dsp ---\n");
	
	for( j = 0; j < DSP_DEVICE_NUM; j ++ ) {
		for( i = 0; i < MAX_DSP_RTK_SS_NUM; i ++ ) {
			PRINT_Y("[%d][%d]=%d\n", j, i, nSidTbl_dsp[j][i] );
		}
	}
#endif
	
}
#endif

