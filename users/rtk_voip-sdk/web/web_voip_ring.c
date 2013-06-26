#include <stdio.h>
#include "web_voip.h"

char ring_group[RING_GROUP_MAX][20] = {"Group1", "Group2","Group3","Group4"};

char cad[RING_CADENCE_MAX][20] = {"Cadence1", "Cadence2", "Cadence3", "Cadence4", 
				  "Cadence5", "Cadence6", "Cadence7", "Cadence8"};
				  
char ring_cad[RING_CADENCE_MAX+1][20] = {"Cadence1", "Cadence2", "Cadence3", "Cadence4", 
				    	 "Cadence5", "Cadence6", "Cadence7", "Cadence8", "Default"};


void asp_voip_RingSet(webs_t wp, char_t *path, char_t *query)
{
	char *ptr;
	voipCfgParam_t *pCfg;
	int i, group_idx, cad_idx;

	if (web_flash_get(&pCfg) != 0)
		return;

	/* Ring Cadence */
	ptr = websGetVar(wp, T("ring_cad"), T(""));
		
	for(i=0; i < RING_CADENCE_MAX+1; i++)
	{
		if (!gstrcmp(ptr, ring_cad[i]))
			break;
	}
	if (i == RING_CADENCE_MAX+1)
		i = RING_CADENCE_1;
	
	pCfg->ring_cad = i;
	
		
	/* Group */	
	ptr = websGetVar(wp, T("group"), T(""));
		
	for(i=0; i < RING_GROUP_MAX; i++)
	{
		if (!gstrcmp(ptr, ring_group[i]))
			break;
	}
	if (i == RING_GROUP_MAX)
		i = RING_GROUP_1;
	
	pCfg->ring_group = group_idx = i;
	
	/* Cadence Select */
	ptr = websGetVar(wp, T("cadence_sel"), T(""));
		
	for(i=0; i < RING_CADENCE_MAX; i++)
	{
		if (!gstrcmp(ptr, cad[i]))
			break;
	}
	if (i == RING_CADENCE_MAX)
		i = RING_CADENCE_1;
	
	pCfg->ring_cadence_sel = cad_idx = i;
	
	
	/* Ring_Cad Apply */
	ptr = websGetVar(wp, T("Ring_Cad"), T(""));	
	if (strcmp(ptr, "Apply") == 0)
	{
		/* Ring Cadence for All */
		ptr = websGetVar(wp, T("ring_cad"), T(""));
			
		for(i=0; i < RING_CADENCE_MAX+1; i++)
		{
			if (!gstrcmp(ptr, ring_cad[i]))
				break;
		}
		if (i == RING_CADENCE_MAX+1)
			i = RING_CADENCE_1;
		
		pCfg->ring_cad = i;
	}

	/* Ring_Group Apply */
	ptr = websGetVar(wp, T("Ring_Group"), T(""));	
	if (strcmp(ptr, "Apply") == 0)
	{
		/* Phone Number */
		pCfg->ring_phone_num[group_idx] = atoi(websGetVar(wp, T("phonenumber"), T("")));
	
		/* Cadence Use*/
		ptr = websGetVar(wp, T("cadence_use"), T(""));
			
		for(i=0; i < RING_CADENCE_MAX; i++)
		{
			if (!gstrcmp(ptr, cad[i]))
				break;
		}
		if (i == RING_CADENCE_MAX)
			i = RING_CADENCE_1;
		
		pCfg->ring_cadence_use[group_idx] = i;
	}

	/* Ring_Cadence Apply */
	ptr = websGetVar(wp, T("Ring_Cadence"), T(""));	
	if (strcmp(ptr, "Apply") == 0)
	{	
		/* Cadence ON/OFF */
		pCfg->ring_cadon[cad_idx] = atoi(websGetVar(wp, T("cad_on"), T("")));
		pCfg->ring_cadoff[cad_idx] = atoi(websGetVar(wp, T("cad_off"), T("")));
	}
	

	
	web_flash_set(pCfg);
	
#ifdef REBOOT_CHECK
	OK_MSG("/voip_ring.asp");
#else
	web_restart_solar();

	websRedirect(wp, T("/voip_ring.asp"));
#endif
}


#ifdef CONFIG_APP_BOA
int asp_voip_RingGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_RingGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	int i, group_idx, cad_idx;
	voipCfgParam_t *pCfg;

	if (web_flash_get(&pCfg) != 0)
		return -1;

	group_idx = pCfg->ring_group;
	cad_idx = pCfg->ring_cadence_sel;
	
	/* Cadence */
	if (strcmp(argv[0], "ring_cad")==0)
	{
		for (i=0; i < RING_CADENCE_MAX+1 ;i++)
		{
			if (i == pCfg->ring_cad)
				websWrite(wp, "<option selected>%s</option>", ring_cad[i]);
			else
				websWrite(wp, "<option>%s</option>", ring_cad[i]);
		}
	}	
	/* Group */	
	else if (strcmp(argv[0], "group")==0)
	{
		for (i=0; i < RING_GROUP_MAX ;i++)
		{
			if (i == pCfg->ring_group)
				websWrite(wp, "<option selected>%s</option>", ring_group[i]);
			else
				websWrite(wp, "<option>%s</option>", ring_group[i]);
		}
	}
	/* Phone Number */
	else if (strcmp(argv[0], "phonenumber")==0)
		websWrite(wp, "%d", pCfg->ring_phone_num[group_idx]);
	/* Cadence Use */
	else if (strcmp(argv[0], "cadence_use")==0)
	{
		for (i=0; i < RING_CADENCE_MAX ;i++)
		{
			if (i == pCfg->ring_cadence_use[group_idx])
				websWrite(wp, "<option selected>%s</option>", cad[i]);
			else
				websWrite(wp, "<option>%s</option>", cad[i]);
		}
	}
	/* Cadence Select */
	else if (strcmp(argv[0], "cadence_sel")==0)
	{
		for (i=0; i < RING_CADENCE_MAX ;i++)
		{
			if (i == pCfg->ring_cadence_sel)
				websWrite(wp, "<option selected>%s</option>", cad[i]);
			else
				websWrite(wp, "<option>%s</option>", cad[i]);
		}
	}
	/* Cadence ON/OFF */
	else if(strcmp(argv[0], "cad_on")==0)
		websWrite(wp, "%d", pCfg->ring_cadon[cad_idx]);
	else if(strcmp(argv[0], "cad_off")==0)
		websWrite(wp, "%d", pCfg->ring_cadoff[cad_idx]);
		
	return 0;	
}
