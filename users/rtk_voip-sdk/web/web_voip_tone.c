#include <stdio.h>
#include "web_voip.h"

char tone_country[TONE_MAX][20] = {"USA", "UK", "AUSTRALIA", "HONG KONG", "JAPAN", 
				   "SWEDEN", "GERMANY", "FRANCE", "TAIWAN", "BELGIUM", 
#ifdef COUNTRY_TONE_RESERVED
				   "FINLAND", "ITALY", "CHINA", "EXT1", "EXT2", "EXT3", "EXT4", "RESERVE", "CUSTOMER"};
#else
				   "FINLAND", "ITALY", "CHINA", "EXT1", "EXT2", "EXT3", "EXT4", "CUSTOMER"};
#endif


char cust_tone[TONE_CUSTOMER_MAX][20] = {"Custom1", "Custom2", "Custom3", "Custom4",
					 "Custom5", "Custom6", "Custom7", "Custom8"};

char tone_type[TONE_TYPE_MAX][20] = {"ADDITIVE", "MODULATED", "SUCC"};

#if 1
char tone_cycle[TONE_CYCLE_MAX][20] = {"CONTINUOUS", "BURST", "CADENCE"};
#else
char tone_cycle[TONE_CYCLE_MAX][20] = {"CONTINUOUS", "BURST"};
#endif

char number_of_distone[DIS_CONNECT_TONE_MAX][5] = {"0", "1", "2"};

void asp_voip_ToneSet(webs_t wp, char_t *path, char_t *query)
{
	char *ptr;
	int i, cust_flag = 0, cust_idx, idx;
	voipCfgParam_t *pCfg;

	if (web_flash_get(&pCfg) != 0)
		return;


	ptr = websGetVar(wp, T("Country"), T(""));	
	if (strcmp(ptr, "Apply") == 0)
	{
		/* select country */
		idx = atoi(websGetVar(wp, T("tone_country"), T("")));
		if (idx < 0 || idx >= TONE_MAX)
			idx = 0;
			
		pCfg->tone_of_country = idx;
		if (idx == TONE_CUSTOMER)
			cust_flag = 1;

		
		if (cust_flag)
		{
			/* select dial */
			ptr = websGetVar(wp, T("dial"), T(""));
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_1;

			pCfg->tone_of_custdial = i;
		
			/* select ring */
			ptr = websGetVar(wp, T("ring"), T(""));
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_2;

			pCfg->tone_of_custring = i;
			
			/* select busy */
			ptr = websGetVar(wp, T("busy"), T(""));
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_3;

			pCfg->tone_of_custbusy = i;
			
			/* select waiting */
			ptr = websGetVar(wp, T("waiting"), T(""));
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_4;
				
			pCfg->tone_of_custwaiting = i;
		}
	}
	
	/* Select Custom Tone */
	ptr = websGetVar(wp, T("selfItem"), T(""));	
	for(i=0; i < TONE_CUSTOMER_MAX; i++)
	{
		if (!gstrcmp(ptr, cust_tone[i]))
			break;
	}
	if (i == TONE_CUSTOMER_MAX)
		i = TONE_CUSTOMER_1;
	
	pCfg->tone_of_customize = i;	
	
	/* Tone Parameters */
	ptr = websGetVar(wp, T("Tone"), T(""));	
	if (strcmp(ptr, "Apply") == 0)
	{
		// Custom Tone Parameters Set
		cust_idx = pCfg->tone_of_customize;
		
		ptr = websGetVar(wp, T("type"), T(""));
		
		for(i=0; i < TONE_TYPE_MAX; i++)
		{
			if (!gstrcmp(ptr, tone_type[i]))
				break;
		}
		if (i == TONE_TYPE_MAX)
			i = TONE_TYPE_ADDITIVE;
		
		pCfg->cust_tone_para[cust_idx].toneType = i;

		///////////////////////////////////////////

		ptr = websGetVar(wp, T("cycle"), T(""));
		
		for(i=0; i < TONE_CYCLE_MAX; i++)
		{
			if (!gstrcmp(ptr, tone_cycle[i]))
				break;
		}
		if (i == TONE_CYCLE_MAX)
			i = TONE_CYCLE_CONTINUOUS;
			
		pCfg->cust_tone_para[cust_idx].cycle = i;
		
		pCfg->cust_tone_para[cust_idx].cadNUM = atoi(websGetVar(wp, T("cadNUM"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOn0 = atoi(websGetVar(wp, T("CadOn0"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOn1 = atoi(websGetVar(wp, T("CadOn1"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOn2 = atoi(websGetVar(wp, T("CadOn2"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOn3 = atoi(websGetVar(wp, T("CadOn3"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOff0 = atoi(websGetVar(wp, T("CadOff0"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOff1 = atoi(websGetVar(wp, T("CadOff1"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOff2 = atoi(websGetVar(wp, T("CadOff2"), T("")));
		pCfg->cust_tone_para[cust_idx].CadOff3 = atoi(websGetVar(wp, T("CadOff3"), T("")));
		pCfg->cust_tone_para[cust_idx].PatternOff = atoi(websGetVar(wp, T("PatternOff"), T("")));
		pCfg->cust_tone_para[cust_idx].ToneNUM = atoi(websGetVar(wp, T("ToneNUM"), T("")));
		pCfg->cust_tone_para[cust_idx].Freq1 = atoi(websGetVar(wp, T("Freq1"), T("")));
		pCfg->cust_tone_para[cust_idx].Freq2 = atoi(websGetVar(wp, T("Freq2"), T("")));
		pCfg->cust_tone_para[cust_idx].Freq3 = atoi(websGetVar(wp, T("Freq3"), T("")));
		pCfg->cust_tone_para[cust_idx].Freq4 = atoi(websGetVar(wp, T("Freq4"), T("")));
		pCfg->cust_tone_para[cust_idx].Gain1 = atoi(websGetVar(wp, T("Gain1"), T("")));
		pCfg->cust_tone_para[cust_idx].Gain2 = atoi(websGetVar(wp, T("Gain2"), T("")));
		pCfg->cust_tone_para[cust_idx].Gain3 = atoi(websGetVar(wp, T("Gain3"), T("")));
		pCfg->cust_tone_para[cust_idx].Gain4 = atoi(websGetVar(wp, T("Gain4"), T("")));	
	}
	
	/* Disconnect tone det config */
	ptr = websGetVar(wp, T("Distone"), T(""));	
	if (strcmp(ptr, "Apply") == 0)
	{
		/* select disconnect tone num */
		ptr = websGetVar(wp, T("distone_num"), T(""));	
		for(i=0; i < DIS_CONNECT_TONE_MAX; i++)
		{
			if (!gstrcmp(ptr, number_of_distone[i]))
				break;
		}
		if (i == DIS_CONNECT_TONE_MAX)
			i = 2;
		pCfg->distone_num = i;

		ptr = websGetVar(wp, T("d1freqnum"), T(""));	
		for(i=1; i < DIS_CONNECT_TONE_FREQ_MAX; i++)
		{
			if (!gstrcmp(ptr, number_of_distone[i]))
				break;
		}
		if (i == DIS_CONNECT_TONE_FREQ_MAX)
			i = 2;
		pCfg->d1freqnum = i;


		pCfg->d1Freq1 = atoi(websGetVar(wp, T("d1Freq1"), T("")));
		pCfg->d1Freq2 = atoi(websGetVar(wp, T("d1Freq2"), T("")));
		pCfg->d1Accur = atoi(websGetVar(wp, T("d1Accur"), T("")));
		pCfg->d1Level = atoi(websGetVar(wp, T("d1Level"), T("")));
		pCfg->d1ONup = atoi(websGetVar(wp, T("d1ONup"), T("")));
		pCfg->d1ONlow = atoi(websGetVar(wp, T("d1ONlow"), T("")));
		pCfg->d1OFFup = atoi(websGetVar(wp, T("d1OFFup"), T("")));
		pCfg->d1OFFlow = atoi(websGetVar(wp, T("d1OFFlow"), T("")));

		ptr = websGetVar(wp, T("d2freqnum"), T(""));	
		for(i=1; i < DIS_CONNECT_TONE_FREQ_MAX; i++)
		{
			if (!gstrcmp(ptr, number_of_distone[i]))
				break;
		}
		if (i == DIS_CONNECT_TONE_FREQ_MAX)
			i = 2;
		pCfg->d2freqnum = i;

		pCfg->d2Freq1 = atoi(websGetVar(wp, T("d2Freq1"), T("")));
		pCfg->d2Freq2 = atoi(websGetVar(wp, T("d2Freq2"), T("")));
		pCfg->d2Accur = atoi(websGetVar(wp, T("d2Accur"), T("")));
		pCfg->d2Level = atoi(websGetVar(wp, T("d2Level"), T("")));
		pCfg->d2ONup = atoi(websGetVar(wp, T("d2ONup"), T("")));
		pCfg->d2ONlow = atoi(websGetVar(wp, T("d2ONlow"), T("")));
		pCfg->d2OFFup = atoi(websGetVar(wp, T("d2OFFup"), T("")));
		pCfg->d2OFFlow = atoi(websGetVar(wp, T("d2OFFlow"), T("")));

	}



	web_flash_set(pCfg);
	
#ifdef REBOOT_CHECK
	OK_MSG("/voip_tone.asp");
#else
	web_restart_solar();

	websRedirect(wp, T("/voip_tone.asp"));
#endif

}


#ifdef CONFIG_APP_BOA
int asp_voip_ToneGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_ToneGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	int i, cust_idx;
	voipCfgParam_t *pCfg;

	if (web_flash_get(&pCfg) != 0)
		return -1;

	cust_idx = pCfg->tone_of_customize;
	
	if (strcmp(argv[0], "tone_country")==0)
	{
		for (i=0; i < TONE_MAX ;i++)
		{
			if (i == pCfg->tone_of_country)
				websWrite(wp, "<option value=%d selected>%s</option>", i, tone_country[i]);
			else
				websWrite(wp, "<option value=%d>%s</option>", i, tone_country[i]);
		}
	}
	else if (strcmp(argv[0], "dial")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custdial) // dial
				websWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				websWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	else if (strcmp(argv[0], "ring")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custring) // ring
				websWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				websWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	else if (strcmp(argv[0], "busy")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custbusy) // busy
				websWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				websWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	else if (strcmp(argv[0], "waiting")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custwaiting) // waiting
				websWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				websWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	// Get Custome Tone 
	else if (strcmp(argv[0], "selfItem")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_customize)
				websWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				websWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	// Get Custom Tone Parameters	
	else if (strcmp(argv[0], "type")==0)
	{
		for (i=0; i < TONE_TYPE_MAX ;i++)
		{
			if (i == pCfg->cust_tone_para[cust_idx].toneType)
				websWrite(wp, "<option selected>%s</option>", tone_type[i]);
			else
				websWrite(wp, "<option>%s</option>", tone_type[i]);
		}
	}
	else if (strcmp(argv[0], "cycle")==0)
	{
		for (i=0; i < TONE_CYCLE_MAX ;i++)
		{
			if (i == pCfg->cust_tone_para[cust_idx].cycle)
				websWrite(wp, "<option selected>%s</option>", tone_cycle[i]);
			else
				websWrite(wp, "<option>%s</option>", tone_cycle[i]);
		}
	}	
	else if (strcmp(argv[0], "cadNUM")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].cadNUM);
	else if (strcmp(argv[0], "CadOn0")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn0);
	else if (strcmp(argv[0], "CadOn1")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn1);
	else if (strcmp(argv[0], "CadOn2")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn2);
	else if (strcmp(argv[0], "CadOn3")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn3);
	else if (strcmp(argv[0], "CadOff0")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff0);
	else if (strcmp(argv[0], "CadOff1")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff1);
	else if (strcmp(argv[0], "CadOff2")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff2);
	else if (strcmp(argv[0], "CadOff3")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff3);
	else if (strcmp(argv[0], "PatternOff")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].PatternOff);
	else if (strcmp(argv[0], "ToneNUM")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].ToneNUM);
	else if (strcmp(argv[0], "Freq1")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq1);
	else if (strcmp(argv[0], "Freq2")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq2);
	else if (strcmp(argv[0], "Freq3")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq3);
	else if (strcmp(argv[0], "Freq4")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq4);
	else if (strcmp(argv[0], "Gain1")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain1);
	else if (strcmp(argv[0], "Gain2")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain2);
	else if (strcmp(argv[0], "Gain3")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain3);
	else if (strcmp(argv[0], "Gain4")==0)
		websWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain4);
	else if (strcmp(argv[0], "display")==0)
	{
		if (pCfg->tone_of_country == TONE_CUSTOMER)
			websWrite(wp, "style=\"display:online\"");		
		else
			websWrite(wp, "style=\"display:none\"");		
	}
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	else if (strcmp(argv[0], "display_distone")==0)
		websWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_distone")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
#endif
	else if (strcmp(argv[0], "distone_num")==0)
	{
		for (i=0; i < DIS_CONNECT_TONE_MAX ;i++)
		{
			if (i == pCfg->distone_num)
				websWrite(wp, "<option value=%d selected>%s</option>", i, number_of_distone[i]);
			else
				websWrite(wp, "<option value=%d >%s</option>", i, number_of_distone[i]);
		}
	}
	else if (strcmp(argv[0], "d1display")==0)
	{
		if (pCfg->distone_num > 0)
			websWrite(wp, "style=\"display:online\"");		
		else
			websWrite(wp, "style=\"display:none\"");		
	}
	else if (strcmp(argv[0], "d1freqnum")==0)
	{
		for (i=1; i < DIS_CONNECT_TONE_FREQ_MAX ;i++)
		{
			if (i == pCfg->d1freqnum)
				websWrite(wp, "<option selected>%s</option>", number_of_distone[i]);
			else
				websWrite(wp, "<option>%s</option>", number_of_distone[i]);
		}
	}
	else if (strcmp(argv[0], "d1Freq1")==0)
		websWrite(wp, "%d", pCfg->d1Freq1);
	else if (strcmp(argv[0], "d1Freq2")==0)
		websWrite(wp, "%d", pCfg->d1Freq2);
	else if (strcmp(argv[0], "d1Accur")==0)
		websWrite(wp, "%d", pCfg->d1Accur);
	else if (strcmp(argv[0], "d1Level")==0)
		websWrite(wp, "%d", pCfg->d1Level);
	else if (strcmp(argv[0], "d1ONup")==0)
		websWrite(wp, "%d", pCfg->d1ONup);
	else if (strcmp(argv[0], "d1ONlow")==0)
		websWrite(wp, "%d", pCfg->d1ONlow);
	else if (strcmp(argv[0], "d1OFFup")==0)
		websWrite(wp, "%d", pCfg->d1OFFup);
	else if (strcmp(argv[0], "d1OFFlow")==0)
		websWrite(wp, "%d", pCfg->d1OFFlow);
	else if (strcmp(argv[0], "d2display")==0)
	{
		if (pCfg->distone_num > 1)
			websWrite(wp, "style=\"display:online\"");		
		else
			websWrite(wp, "style=\"display:none\"");		
	}
	else if (strcmp(argv[0], "d2freqnum")==0)
	{
		for (i=1; i < DIS_CONNECT_TONE_FREQ_MAX ;i++)
		{
			if (i == pCfg->d2freqnum)
				websWrite(wp, "<option selected>%s</option>", number_of_distone[i]);
			else
				websWrite(wp, "<option>%s</option>", number_of_distone[i]);
		}
	}
	else if (strcmp(argv[0], "d2Freq1")==0)
		websWrite(wp, "%d", pCfg->d2Freq1);
	else if (strcmp(argv[0], "d2Freq2")==0)
		websWrite(wp, "%d", pCfg->d2Freq2);
	else if (strcmp(argv[0], "d2Accur")==0)
		websWrite(wp, "%d", pCfg->d2Accur);
	else if (strcmp(argv[0], "d2Level")==0)
		websWrite(wp, "%d", pCfg->d2Level);
	else if (strcmp(argv[0], "d2ONup")==0)
		websWrite(wp, "%d", pCfg->d2ONup);
	else if (strcmp(argv[0], "d2ONlow")==0)
		websWrite(wp, "%d", pCfg->d2ONlow);
	else if (strcmp(argv[0], "d2OFFup")==0)
		websWrite(wp, "%d", pCfg->d2OFFup);
	else if (strcmp(argv[0], "d2OFFlow")==0)
		websWrite(wp, "%d", pCfg->d2OFFlow);



	

	return 0;
}


