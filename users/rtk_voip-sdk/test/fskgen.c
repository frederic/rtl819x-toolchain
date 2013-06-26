#include "voip_manager.h"

unsigned int Gain2Val(int gain);
int Val2Gain(unsigned int val);

int fskgen_main(int argc, char *argv[])
{
	if (argc == 7)
	{
		if (atoi(argv[3]) == 1)//type I
		{
			//DSP default is soft gen, no need to set soft gen mode
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[2])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			// API to gen FSK caller ID and auto ring by DSP
			rtk_Gen_FSK_CID(atoi(argv[1]), argv[4]/*cid*/, argv[6], argv[5]/*name*/, 0);
#elif 0
			// API to gen caller ID and auto ring by DSP
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], argv[6], argv[5]/*name*/, 0/*type1*/, 0);
#else
			//FSK_PARAM_NULL = 0,
			//FSK_PARAM_DATEnTIME = 0x01,	// Date and Time
			//FSK_PARAM_CLI = 0x02,		// Calling Line Identify (CLI)
			//FSK_PARAM_CLI_ABS = 0x04,	// Reason for absence of CLI
			//FSK_PARAM_CLI_NAME = 0x07,	// Calling Line Identify (CLI) Name
			//FSK_PARAM_CLI_NAME_ABS = 0x08,	// Reason for absence of (CLI) Name
			//FSK_PARAM_MW = 0x0b,		// Message Waiting
	
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
        	clid.service_type = 0; //service type 1
        
        	clid.cid_data[0].type = 0x01;
        	strcpy(clid.cid_data[0].data, argv[6]);//DATE

			if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') ) //Private or Out of area
				clid.cid_data[1].type = 0x04;
        	else
        		clid.cid_data[1].type = 0x02;

        	strcpy(clid.cid_data[1].data, argv[4]);	//CLI
        		
			if ( (argv[5][0] == 'P') || (argv[5][0] == 'O') ) //Private or Out of area
				clid.cid_data[2].type = 0x08;
			else
				clid.cid_data[2].type = 0x07;

			strcpy(clid.cid_data[2].data, argv[5]);	//CLI_NAME

			//Only 3 elements for Caller ID data. Set other element to 0 (MUST)
        	clid.cid_data[3].type = 0;
        	clid.cid_data[4].type = 0;
			
			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 3);
#endif
		}
		else if (atoi(argv[3]) == 2)//type II
		{
			//DSP default is soft gen, no need to set soft gen mode
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[2])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			rtk_Gen_FSK_CID(atoi(argv[1])/*chid*/, argv[4]/*cid*/, argv[6], argv[5]/*name*/, 1);
#elif 0
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], argv[6], argv[5]/*name*/, 1/*type1*/, 0);
#else
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
			clid.service_type = 1;  //service type 2
	
			clid.cid_data[0].type = 0x01;
			strcpy(clid.cid_data[0].data, argv[6]);	//DATE

			if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') ) //Private or Out of area
				clid.cid_data[1].type = 0x04;
			else
				clid.cid_data[1].type = 0x02;

			strcpy(clid.cid_data[1].data, argv[4]);	//CLI
			
			if ( (argv[5][0] == 'P') || (argv[5][0] == 'O') ) //Private or Out of area
				clid.cid_data[2].type = 0x08;
			else
				clid.cid_data[2].type = 0x07;

			strcpy(clid.cid_data[2].data, argv[5]);	//CLI_NAME
	
			//Only 3 elements for Caller ID data. Set other element to 0 (MUST)
			clid.cid_data[3].type = 0;
			clid.cid_data[4].type = 0;
			
			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 3);
#endif
		}
		else
			printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
	}
	else if (argc == 5)// no name, date, time
	{
		if (atoi(argv[3]) == 1)//type I
		{
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[2])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			rtk_Gen_FSK_CID(atoi(argv[1]), argv[4]/*cid*/, 0/*date*/, 0/*name*/, 0);
#elif 0
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], 0/*date*/, 0/*name*/, 0/*type1*/, 0);
#else
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
       		clid.service_type = 0;  //service type 1
        
			if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') )
				clid.cid_data[0].type = 0x04;
			else
				clid.cid_data[0].type = 0x02;

       		strcpy(clid.cid_data[0].data, argv[4]);	//CLI

			clid.cid_data[1].type = 0;        		
			clid.cid_data[2].type = 0;
			clid.cid_data[3].type = 0;
        	clid.cid_data[4].type = 0;

			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 1);
#endif
		}
		else if (atoi(argv[3]) == 2)//type II
		{
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[2])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			rtk_Gen_FSK_CID(atoi(argv[1])/*chid*/, argv[4]/*cid*/, 0/*date*/, 0/*name*/, 1);
#elif 0
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], 0/*date*/, 0/*name*/, 1/*type1*/, 0);
#else
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
        	clid.service_type = 1;	 //service type 2
        
        	if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') )
				clid.cid_data[0].type = 0x04;
        	else
        		clid.cid_data[0].type = 0x02;

        	strcpy(clid.cid_data[0].data, argv[4]);
        		
        	clid.cid_data[1].type = 0;        		
			clid.cid_data[2].type = 0;
			clid.cid_data[3].type = 0;
        	clid.cid_data[4].type = 0;

			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 1);
#endif
		}
		else
			printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
	}
	else if (argc == 19)  /* set */
	{
		TstVoipFskPara para;
		int mark_gain;
		int space_gain;
		
		if (argv[1][0] == 's') //set para
		{
			/* check Gain range */
			mark_gain = atoi(argv[6]);
			space_gain = atoi(argv[7]);

			if (mark_gain<-16 || mark_gain>8) {
				printf("\ninvalid mark_gain  : %d\n",mark_gain);
				printf("Range of mark_gain : -16dB ~ +8dB\n\n");
				return -1;
			}

			if (space_gain<-16 || space_gain>8) {
				printf("\ninvalid space_gain  : %d\n",space_gain);
				printf("Range of space_gain : -16dB ~ +8dB\n\n");
				return -1;
			}

			para.ch_id = atoi(argv[2]);
			para.area = atoi(argv[3]);
			para.CS_cnt = atoi(argv[4]);
			para.mark_cnt = atoi(argv[5]);
			para.mark_gain = Gain2Val(atoi(argv[6]));
			para.space_gain = Gain2Val(atoi(argv[7]));
			para.type2_expected_ack_tone = argv[8][0];
			para.delay_after_1st_ring = atoi(argv[9]);
			para.delay_before_2nd_ring = atoi(argv[10]);
			para.silence_before_sas = atoi(argv[11]);
			para.sas_time = atoi(argv[12]);
			para.delay_after_sas = atoi(argv[13]);
			para.cas_time = atoi(argv[14]);
			para.type1_delay_after_cas = atoi(argv[15]);
			para.ack_waiting_time = atoi(argv[16]);
			para.delay_after_ack_recv = atoi(argv[17]);
			para.delay_after_type2_fsk = atoi(argv[18]);
			
			rtk_Set_FSK_CLID_Para(&para);
		}
	}
	else if (argc == 4) 
	{
		TstVoipFskPara para;
		
		if (argv[1][0] == 'g') //get para
		{
			para.ch_id = atoi(argv[2]);
			para.area = atoi(argv[3]);
			
			rtk_Get_FSK_CLID_Para(&para);
			
			printf("FSK parameters of chid %d\n",para.ch_id);
			printf("=======================================\n");

			switch(para.area) {
				case FSK_Bellcore: 
					printf(" - fsk_area = Bellcore/Telcordia FSK\n");
				break;
				case FSK_ETSI: 
					printf(" - fsk_area = ETSI FSK\n");
				break;
				case FSK_BT: 
					printf(" - fsk_area = BT FSK\n");
				break;
				case FSK_NTT: 
					printf(" - fsk_area = NTT FSK\n");
				break;
				default:
					printf(" - fsk_area = %d\n",para.area);
				break;
			}

			printf(" - ch seizure cnt           = %4d bits\n", para.CS_cnt);
			printf(" - mark cnt                 = %4d bits\n", para.mark_cnt);
			//printf(" - mark value               = %4d\n", para.mark_gain);
			printf(" - mark  (Logic 1) +/-gain  = %4d dB\n", Val2Gain(para.mark_gain));
			//printf(" - space value              = %4d\n", para.space_gain);
			printf(" - space (Logic 0) +/-gain  = %4d dB\n", Val2Gain(para.space_gain));
			printf(" - type2_expected_ack_tone  = %4C\n", para.type2_expected_ack_tone);
			printf(" - delay_after_1st_ring     = %4d ms\n", para.delay_after_1st_ring);
			printf(" - delay_before_2nd_ring    = %4d ms\n", para.delay_before_2nd_ring);
			printf(" - silence_before_sas       = %4d ms\n", para.silence_before_sas);
			printf(" - sas_time                 = %4d ms\n", para.sas_time);
			printf(" - delay_after_sas          = %4d ms\n", para.delay_after_sas);
			printf(" - cas_time                 = %4d ms\n", para.cas_time);
			printf(" - type1_delay_after_cas    = %4d ms\n", para.type1_delay_after_cas);
			printf(" - ack_waiting_time         = %4d ms\n", para.ack_waiting_time);
			printf(" - delay_after_ack_recv     = %4d ms\n", para.delay_after_ack_recv);
			printf(" - delay_after_type2_fsk    = %4d ms\n", para.delay_after_type2_fsk);
			printf("=======================================\n");

			printf("\nExample Command of FSK param set and gen:\n\n");
			printf("%s set %d %d %d %d %d %d %C %d %d %d %d %d %d %d %d %d %d\n",argv[0], para.ch_id, para.area, para.CS_cnt, 
                           para.mark_cnt, Val2Gain(para.mark_gain), Val2Gain(para.space_gain),
                           para.type2_expected_ack_tone, para.delay_after_1st_ring, 
                           para.delay_before_2nd_ring, para.silence_before_sas,
                           para.sas_time, para.delay_after_sas,
                           para.cas_time, para.type1_delay_after_cas, para.ack_waiting_time,
                           para.delay_after_ack_recv, para.delay_after_type2_fsk);

			printf("%s %d %d 1 1955-2011 Steve.Jobs 10060700\n\n",argv[0], para.ch_id, para.area);
		}
	}
	else
	{
		printf("Usage:\n");
		printf("To generate Caller ID:\n");
		printf("  fskgen <chid> <fsk_area> <type> <caller_id> <name> <date_time>\n\n");

		printf("To set parameters for a specific FSK standard:\n");
		printf("  fskgen set <chid> <fsk_area> <ch seizure cnt> <mark cnt>\n");
		printf("             <mark gain> <space gain> <type-2 expected ack tone>\n");
		printf("             <delay_after_1st_ring> <delay_before_2nd_ring>\n");
		printf("             <silence_before_sas> <sas_time>\n");
		printf("             <delay_after_sas> <cas_time> <type1_delay_after_cas>\n");
		printf("             <ack_waiting_time> <delay_after_ack_recv>\n");
		printf("             <delay_after_type2_fsk>\n\n");

		printf("To get parameters:\n  fskgen get <chid> <fsk_area>\n\n");
		
		
		printf("Parameters:\n");
		printf(" - fsk_area: 0 -> Bellcore, 1 -> ETSI, 2 -> BT, 3 -> NTT\n");
		printf(" - type: 1 -> type1, 2 -> type2\n");
		printf(" - gain: +/- gain from -12.5dB range: -16dB ~ +8dB\n");
		printf("   mark -> logic '1', space -> logic '0'\n");
		printf(" - type-2 expexted ack tone: character A or B or C or D\n");
		printf(" - date_time: MMDDhhmm\n\n\n");

		printf("Example of getting ch0 Bellcore parameters:\n");
		printf("  fskgen get 0 0\n");
		printf("Example of setting ch0 Bellcore FSK parameters:\n");
		printf("  fskgen set 0 0 300 180 0 0 D 150 1000 0 330 50 80 80 160 80 150\n");
		printf("Example of generating type 1 Bellcore FSK on ch0:\n");
		printf("  fskgen 0 0 1 1955-2011 Steve.Jobs 10060700\n");
		
		#if 1
		printf("Example of displaying number 035780211 and name, date, and time.(Bellcore)\n");
		printf("  fskgen 0 128 1 035780211 tester_B 01020304\n"); 
		printf("Example of displaying number 035780211 and name, date, and time.(ETSI)\n");
		printf("  fskgen 0 129 1 035780211 tester_E 04030201\n"); 
		printf("Example of displaying number 035780211 and name, date, and time.(NTT)\n");
		printf("  fskgen 0 131 1 035780211 tester_N 06020602\n");
		printf("Example of displaying number 035780211 without name, date, and time.(Bellcore)\n");
		printf("  fskgen 0 0 1 035780211\n"); 
		printf("Example of displaying number 035780211 without name, date, and time.(ETSI)\n");
		printf("  fskgen 0 1 1 035780211\n");
		printf("Example of displaying number 035780211 without name, date, and time.(NTT)\n");
		printf("  fskgen 0 3 1 035780211\n\n");
		#endif
	}
		

	return 0;
}

/*******************************************************
 * Convert Gain dB to value
 *
 * val  0 =   8 dB
 * val  1 =   7 dB
 * val  2 =   6 dB
 * val  3 =   5 dB
 * val  4 =   4 dB
 * val  5 =   3 dB
 * val  6 =   2 dB
 * val  7 =   1 dB
 * val  8 =   0 dB
 * val  9 =  -1 dB
 * val 10 =  -2 dB
 * val 11 =  -3 dB
 * val 12 =  -4 dB
 * val 13 =  -5 dB
 * val 14 =  -6 dB
 * val 15 =  -7 dB
 * val 16 =  -8 dB
 * val 17 =  -9 dB
 * val 18 = -10 dB
 * val 19 = -11 dB
 * val 20 = -12 dB
 * val 21 = -13 dB
 * val 22 = -14 dB
 * val 23 = -15 dB
 * val 24 = -16 dB
 ******************************************************/
unsigned int Gain2Val(int gain)
{
	int inGain = gain;

	/* valid gain: -16dB ~ 8dB */
    if (gain < -16) inGain = -16;
    if (gain > 8) 	inGain = 8;
    
    return (8-inGain);
}


int Val2Gain(unsigned int val)
{
	int inVal = val;

	/* valid val 0 ~ 24 */
    if (val>24) inVal = 24;

    return (8-inVal);
}

/*
		   - fsk_area[2:0] => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n" \
		"  - fsk_area[bit7]=> Auto SLIC Ringing\n" \
		"  - fsk_area[bit7]=> FSK date & time sync\n" \
		"  - fsk_area[bit6]=> reverse polarity before caller id (For FSK)\n" \
		"  - fsk_area[bit5]=> short ring before caller id (For FSK)\n" \
		"  - fsk_area[bit4]=> dual alert tone before caller id (For FSK)\n" \
		"  - fsk_area[bit3]=> caller id Prior Ring (FSK & DTMF)\n"
*/
