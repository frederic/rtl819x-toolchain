#include "voip_manager.h"

/*
typedef struct
{
	unsigned long	toneType;	///< ADDITIVE, MODULATED, SUCC, SUCC_ADD
	unsigned short	cycle;		///< "<0": illegal value, "0": represent "continuous tone", ">0": cycle number

	unsigned short	cadNUM;		///< Cadence number (in SUCC and SUCC_ADD mode, it represent repeat number of sequence)

	unsigned long	CadOn0;		///< Cadence On0 time (ms)
	unsigned long	CadOn1;		///< Cadence On1 time (ms)
	unsigned long	CadOn2;		///< Cadence On2 time (ms)
	unsigned long	CadOn3;		///< Cadence On3 time (ms)
	unsigned long	CadOff0;	///< Cadence Off0 time (ms)
	unsigned long	CadOff1;	///< Cadence Off1 time (ms)
	unsigned long	CadOff2;	///< Cadence Off2 time (ms)
	unsigned long	CadOff3;	///< Cadence Off3 time (ms)

	unsigned long PatternOff;	///< pattern Off time (ms)
	unsigned long ToneNUM;		///< tone number (1..4)

	unsigned long	Freq1;		///< Freq1 (Hz)
	unsigned long	Freq2;		///< Freq2 (Hz)
	unsigned long	Freq3;		///< Freq3 (Hz)
	unsigned long	Freq4;		///< Freq4 (Hz)

	long Gain1;					///< Gain1 (db)
	long Gain2;					///< Gain2 (db)
	long Gain3;					///< Gain3 (db)
	long Gain4;					///< Gain4 (db)
	//int32	ret_val;
}
TstVoipToneCfg;
*/

TstVoipToneCfg customToneTable[] =
{
	{0, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, 13, 13, 0, 0},		// USA dial tone
	{0, 0, 1, 2000, 0, 0, 0, 4000, 0, 0, 0, 0, 2, 440, 480, 0, 0, 13, 13, 0, 0},		// USA ring back tone
	{2, 0, 3, 500, 500, 500, 0, 0, 0, 1000, 0, 0, 3, 950, 1400, 1750, 0, 7, 7, 7, 0},// USA sit-no circuit tone
	{2, 2, 3, 500, 500, 500, 0, 0, 0, 1000, 0, 0, 3, 950, 1400, 1750, 0, 7, 7, 7, 0},// USA sit-no circuit tone (2 cycle)
	{0, 0, 1, 500, 0, 0, 0, 500, 0, 0, 0, 0, 2, 480, 620, 0, 0, 7, 7, 0, 0},			// USA busy tone
	{0, 0, 2, 500, 1000, 0, 0, 500, 1000, 0, 0, 0, 2, 330, 880, 0, 0, 7, 7, 0, 0},	// tone with 2 cadence
	{0, 0, 1, 1000, 0, 0, 0, 1000, 0, 0, 0, 0, 2, 330, 660, 0, 0, 7, 7, 0, 0},		// tone with 1 cadence
	{1, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 425, 25, 0, 0, 7, 7, 0, 0}			// AUSTRALIA dial tone

};

void SetCustomTone(int test)
{
	int cust_idx;
	
	st_ToneCfgParam custom_tone[TONE_CUSTOMER_MAX];
	
	voipCfgParam_t VoIPCfg;
	
	
	memcpy(custom_tone, customToneTable, TONE_CUSTOMER_MAX*sizeof(st_ToneCfgParam));
	
	for (cust_idx=0; cust_idx < TONE_CUSTOMER_MAX; cust_idx++)
	{
		rtk_Set_Custom_Tone(cust_idx, &custom_tone[cust_idx]);
	}	
	
#if 0
	VoIPCfg.tone_of_country = 13; //Custom
	VoIPCfg.tone_of_custdial = 0;
	VoIPCfg.tone_of_custring = 1;
	VoIPCfg.tone_of_custbusy = 2;
	VoIPCfg.tone_of_custwaiting = 3;

	rtk_Set_Country(&VoIPCfg);
#else	
	rtk_Set_Country_Tone(COUNTRY_CUSTOMER);
	
	//int32 rtk_Use_Custom_Tone(uint8 dial, uint8 ringback, uint8 busy, uint8 waiting)

	if (test == 0)
		rtk_Use_Custom_Tone(0, 0, 0, 0);
	else if (test == 1)
		rtk_Use_Custom_Tone(1, 1, 1, 1);
	else if (test == 2)
		rtk_Use_Custom_Tone(2, 2, 2, 2);
	else if (test == 3)
		rtk_Use_Custom_Tone(3, 3, 3, 3);
	else if (test == 4)
		rtk_Use_Custom_Tone(4, 4, 4, 4);
	else if (test == 5)
		rtk_Use_Custom_Tone(5, 5, 5, 5);
	else if (test == 6)
		rtk_Use_Custom_Tone(6, 6, 6, 6);
	else if (test == 7)
		rtk_Use_Custom_Tone(7, 7, 7, 7);
#endif
}

int tone_main(int argc, char *argv[])
{
	uint32 val;
	
	if (argc == 3)
	{
		switch (atoi(argv[1]))
		{
			case 0:
				// argv[2]: 0 ~ 12: country, 13: customize
				printf("rtk_Set_Country_Tone...\n");
				rtk_Set_Country_Tone(atoi(argv[2]));
				break;
				
			case 1:
				// argv[2]: 0 ~ 12: country, 13: customize
				printf("rtk_Set_Country_Impedance...\n");
				rtk_Set_Country_Impedance(atoi(argv[2]));
				break;
			
			case 2:
				printf("rtk_Set_Impedance...\n");
				rtk_Set_Impedance(atoi(argv[2]));
				/* 
					0: 600
					1: 900
					2: 250+(750||150nf)
					3: 320+(1150||230nf)
					4: 350+(1000||210nf)
				*/
				break;
				
			case 3:
				printf("SetCustomTone...\n");
				SetCustomTone(atoi(argv[2]));
				break;
			
			default:
				printf("Error input\n");
				break;
		}
	}
	else
	{
		printf("tone test error, please see the rtk_voip/test/tone.c for detail!\n");
	}

	return 0;
}

