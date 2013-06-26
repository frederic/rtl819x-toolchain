/*
 * @file fxo_callerid_det.c
 * @brief example code for fxo callerid detect. run when fxo pcm enable
 *        rtk_Get_DAA_CallerID rtk_Set_CID_Det_Mode api example also in fxo.c
 * jwsyu@realtek.com
 * 2011/05/12 PM 07:11:41
 * version 01
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "voip_manager.h"

void ShowUsage(char *cmd)
{
	printf("usage: %s <fxo_chid> <test_mode> \n" \
	       " - fxo_chid => the fxo channel id \n" \
	       " - test_mode => 1: GET fxo caller id \n" \
	       "               2: SET Disable Caller ID Auto Detection + ETSI FSK detection mode \n" \
	       "              3: SET Disable Caller ID Auto Detection + DTMF detection mode \n" \
               "             4: Enable Caller ID Auto Detection (NTT Not Support) \n" \
	       "            5: Enable Caller ID Auto Detection (NTT Support)\n", cmd); 
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int test_mode;
	unsigned int fxo_chid;
	char str_cid[128], str_date[128], str_name[128];

	/* init the caller id buffer */
	str_cid[0]=str_date[0]=str_name[0]=0;

	if (argc < 3)
		ShowUsage(argv[0]);

	fxo_chid = atoi(argv[1]);
	test_mode = atoi(argv[2]);

	switch (test_mode)
	{
	case 1:
		rtk_Get_DAA_CallerID(fxo_chid, str_cid, str_date, str_name);
		if (str_cid[0])
			printf("caller id number:%s, date:%s, name:%s", str_cid, str_date, str_name);
		else
			printf("no caller id detect");
		break;
	case 2:
		rtk_Set_CID_Det_Mode(fxo_chid, 0, 1);
		break;
	case 3:
		rtk_Set_CID_Det_Mode(fxo_chid, 0, 4);
		break;
	case 4:
		rtk_Set_CID_Det_Mode(fxo_chid, 2, 0);
		break;
	case 5:
		rtk_Set_CID_Det_Mode(fxo_chid, 1, 1);
	}
	return 0;

}

