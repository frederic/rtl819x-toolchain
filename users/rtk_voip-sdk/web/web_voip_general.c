#include <stdio.h>
#include "web_voip.h"

char dtmf[DTMF_MAX][12] = {"RFC2833", "SIP INFO", "Inband", "DTMF_delete"};

char cid[CID_MAX][25] = {"FSK_BELLCORE", "FSK_ETSI", "FSK_BT", "FSK_NTT", "DTMF"};

char cid_dtmf[CID_DTMF_MAX][8] = {"DTMF_A","DTMF_B","DTMF_C","DTMF_D"};

char fax_modem_det_string[FAX_MODEM_DET_MAX][7] = {"AUTO", "FAX", "MODEM", "AUTO_2"};

char g726pack[G726_PACK_MAX][9] = {"None", "Left", "Right"};

char *g7111_modes[G7111_MODES] = {"R1", "R2A", "R2B", "R3"};

#ifdef SUPPORT_VOICE_QOS
char dscp[DSCP_MAX][25] = {"Class 0 (DSCP 0x00)",
							"Class 1 (DSCP 0x08)",
							"Class 2 (DSCP 0x10)",
							"Class 3 (DSCP 0x18)",
							"Class 4 (DSCP 0x20)",
							"Class 5 (DSCP 0x28)",
							"Class 6 (DSCP 0x30)",
							"Class 7 (DSCP 0x38)",
							"EF (DSCP 0x2e)"
							};
#endif

char *supported_codec_string[SUPPORTED_CODEC_MAX] = {
	"G711-ulaw",
	"G711-alaw",
#ifdef CONFIG_RTK_VOIP_G729AB
	"G729",
#endif
#ifdef CONFIG_RTK_VOIP_G7231
	"G723",
#endif
#ifdef CONFIG_RTK_VOIP_G726
	"G726-16k",
	"G726-24k",
	"G726-32k",
	"G726-40k",
#endif
#ifdef CONFIG_RTK_VOIP_GSMFR
	"GSM-FR",
#endif
#ifdef CONFIG_RTK_VOIP_ILBC
	"iLBC",
#endif
#ifdef CONFIG_RTK_VOIP_G722
	"G722",
#endif
#ifdef CONFIG_RTK_VOIP_SPEEX_NB
	"SPEEX-NB",
#endif
#ifdef CONFIG_RTK_VOIP_G7111
	"G711U-WB",
	"G711A-WB",
#endif
	};

void asp_sip_codec_var(webs_t wp, voipCfgPortParam_t *pCfg)
{
	int i;

	for (i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		websWrite(wp,
			"<input type=hidden id=preced_id name=preced%d value=-1>", i
			);
	}

	for (i=0; i<G7111_MODES; i++)
	{
		websWrite(wp,
			"<input type=hidden id=g7111_modes name=%s value=-1>",
				g7111_modes[i]
			);
	}
}

void asp_sip_codec(webs_t wp, voipCfgPortParam_t *pCfg)
{
	int i, j;
#ifdef SUPPORT_CUSTOMIZE_FRAME_SIZE
	int step, loop;
#endif

	websWrite(wp,
		"<tr align=center>" \
		"<td bgColor=#aaddff width=85 rowspan=2>Type</td>"
		);

// framesize is reserved
#ifdef SUPPORT_CUSTOMIZE_FRAME_SIZE
	websWrite(wp,
		"<td bgColor=#ddeeff width=85 rowspan=2>Packetization</td>"
		);
#endif

	websWrite(wp,
		"<td bgColor=#ddeeff colspan=%d>Precedence</td>",
		SUPPORTED_CODEC_MAX
		);


	websWrite(wp, "</tr>\n");

	// Draw precedence number
	websWrite(wp, "<tr align=center>");
	for (i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		websWrite(wp, "<td bgColor=#ddeeff>%d</td>", i + 1);
	}
	websWrite(wp, "</tr>\n");

	// Draw Codecs
	for (i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		// codec name
		websWrite(wp,
			"<tr>" \
			"<td bgColor=#aaddff>%s</td>",
			supported_codec_string[i]
			);

// framesize is reserved
#ifdef SUPPORT_CUSTOMIZE_FRAME_SIZE
		// framesize
		websWrite(wp,
			"<td bgColor=#ddeeff>" \
			"<select name=frameSize%d>",
			i
			);

		switch (i)
		{
		case SUPPORTED_CODEC_G711U:
		case SUPPORTED_CODEC_G711A:
#ifdef CONFIG_RTK_VOIP_G722
		case SUPPORTED_CODEC_G722:
#endif
			// 10, 20 ... 60 ms
			step = 10;
			loop = 6;
			break;
#ifdef CONFIG_RTK_VOIP_G7231
		case SUPPORTED_CODEC_G723:
			// 30, 60, 90 ms
			step = 30;
			loop = 3;
			break;
#endif
		default:
			// 10, 20 ... 90ms
			step = 10;
			loop = 9;
			break;
		}

		for (j=0; j<loop; j++)
			websWrite(wp,
				"<option %s value=%d>%d ms</option>",
				j == pCfg->frame_size[i] ? "selected" : "",
				j,
				step * (j + 1)
				);

		websWrite(wp,
			"</select>" \
			"</td>\n"
			);
#endif /* SUPPORT_CUSTOMIZE_FRAME_SIZE */

		// precedence
		for (j=0; j<SUPPORTED_CODEC_MAX; j++)
		{
			websWrite(wp,
				"<td bgColor=#ddeeff align=center>" \
				"<input type=checkbox name=precedence %s onclick=\""
				"checkPrecedence(sipform.precedence, %d, %d, %d, 1)\">" \
				"</td>\n",
				j == pCfg->precedence[i] ? "checked" : "",
				i, j, SUPPORTED_CODEC_MAX
			);
		}


		websWrite(wp, "</tr>\n");
	}
}

void asp_sip_codec_opt(webs_t wp, voipCfgPortParam_t *pCfg)
{
	int i;
#ifdef CONFIG_RTK_VOIP_G7111
	int j;
#endif

	i = 1;
#ifdef CONFIG_RTK_VOIP_G726
	i ++;
#endif
#ifdef CONFIG_RTK_VOIP_G7231
	i ++;
#endif
#ifdef CONFIG_RTK_VOIP_ILBC
	i ++;
#endif
#ifdef CONFIG_RTK_VOIP_SPEEX_NB
	i ++;
#endif
#ifdef CONFIG_RTK_VOIP_G7111
	i += 2 + G7111_MODES; // 2 header + 4 modes
#endif

	if (i == 1)
		return;

	websWrite(wp, "<table cellSpacing=1 cellPadding=2 border=0>");

	websWrite(wp,
		"<tr>" \
		"<td bgColor=#aaddff width=85 rowspan=%d>Option</td>"
		"</tr>", i
		);

#ifdef CONFIG_RTK_VOIP_G726
	websWrite(wp,"<tr>" );
	websWrite(wp,
		"<td bgColor=#ddeeff colspan=1> G726 Packing Order" \
		"</td>"
		);

	websWrite(wp,
		"<td bgColor=#ddeeff colspan=5>" \
		"<select WIDTH=300 STYLE='width: 120px' name=g726_packing >" \
		"<option %s>Left</option>" \
		"<option %s>Right</option>" \
		"</select>" \
		"</td>",
		pCfg->g726_packing == G726_PACK_LEFT ? "selected" : "",
		pCfg->g726_packing == G726_PACK_RIGHT ? "selected" : ""
		);
	websWrite(wp, "</tr>\n");

	//"<option %s>None</option>"
	//pCfg->g726_packing == G726_PACK_NONE ? "selected" : "",
#endif

#ifdef CONFIG_RTK_VOIP_G7231
	websWrite(wp,"<tr>" );
			websWrite(wp,
		"<td bgColor=#ddeeff colspan=1> G723 Bit Rate" \
		"</td>"
		);

	websWrite(wp,
		"<td bgColor=#ddeeff colspan=5>" \
		"<select WIDTH=300 STYLE='width: 120px' name=g7231Rate>" \
				"<option %s>6.3k</option>" \
				"<option %s>5.3k</option>" \
				"</select>" \
				"</td>",
				pCfg->g7231_rate == G7231_RATE63 ? "selected" : "",
				pCfg->g7231_rate == G7231_RATE53 ? "selected" : ""
				);

	websWrite(wp, "</tr>\n");
#endif

#ifdef CONFIG_RTK_VOIP_ILBC
	websWrite(wp,"<tr>" );
			websWrite(wp,
		"<td bgColor=#ddeeff colspan=1> iLBC Frame Size" \
		"</td>"
		);

	websWrite(wp,
		"<td bgColor=#ddeeff colspan=5>" \
		"<select WIDTH=300 STYLE='width: 120px' name=iLBC_mode>" \
				"<option %s>30ms</option>" \
				"<option %s>20ms</option>" \
				"</select>" \
				"</td>",
				pCfg->iLBC_mode == ILBC_30MS ? "selected" : "",
				pCfg->iLBC_mode == ILBC_20MS ? "selected" : ""
				);

	websWrite(wp, "</tr>\n");
#endif

#ifdef CONFIG_RTK_VOIP_SPEEX_NB
	websWrite(wp,"<tr>" );
			websWrite(wp,
		"<td bgColor=#ddeeff colspan=1> SPEEX NB Rate" \
		"</td>"
		);

	websWrite(wp,
		"<td bgColor=#ddeeff colspan=5>" \
		"<select WIDTH=300 STYLE='width: 120px' name=speex_nb_rate>" \
				"<option %s value=0>2.15k</option>" \
				"<option %s value=1>5.95k</option>" \
				"<option %s value=2>8k</option>" \
				"<option %s value=3>11k</option>" \
				"<option %s value=4>15k</option>" \
				"<option %s value=5>18.2k</option>" \
				"<option %s value=6>24.6k</option>" \
				"<option %s value=7>3.95k</option>" \
				"</select>" \
				"</td>",
				pCfg->speex_nb_rate == SPEEX_RATE2P15 ? "selected" : "",
				pCfg->speex_nb_rate == SPEEX_RATE5P95 ? "selected" : "",
				pCfg->speex_nb_rate == SPEEX_RATE8 ? "selected" : "",
				pCfg->speex_nb_rate == SPEEX_RATE11 ? "selected" : "",
				pCfg->speex_nb_rate == SPEEX_RATE15 ? "selected" : "",
				pCfg->speex_nb_rate == SPEEX_RATE18P2 ? "selected" : "",
				pCfg->speex_nb_rate == SPEEX_RATE24P6 ? "selected" : "",
				pCfg->speex_nb_rate == SPEEX_RATE3P95 ? "selected" : ""
				);

	websWrite(wp, "</tr>\n");
#endif

#ifdef CONFIG_RTK_VOIP_G7111
	websWrite(wp, "<tr>");
	websWrite(wp, "<td bgColor=#ddeeff rowspan=%d> G711 WB Modes </td>",
		2 + G7111_MODES);
	websWrite(wp, "<td bgColor=#ddeeff rowspan=2>Mode</td>");
	websWrite(wp, "<td bgColor=#ddeeff colspan=%d> Priority </td>",
		G7111_MODES);
	websWrite(wp, "</tr>");

	websWrite(wp, "<tr>");
	for (i=0; i<G7111_MODES; i++)
		websWrite(wp, "<td bgColor=#ddeeff>%d</td>", i + 1);
	websWrite(wp, "</tr>");

	for (i=0; i<G7111_MODES; i++)
	{
		websWrite(wp, "<tr>");
		websWrite(wp, "<td bgColor=#ddeeff>%s</td>", g7111_modes[i]);
		for (j=0; j<G7111_MODES; j++)
		{
			websWrite(wp, "<td bgColor=#ddeeff align=center>"
				"<input type=checkbox name=g7111_pri %s onclick=\""
				"checkPrecedence(sipform.g7111_pri, %d, %d, %d, 0)\""
				"</td>\n",
				i + 1 == pCfg->g7111_precedence[j] ? "checked" : "",
				i, j, G7111_MODES
			);
		}
		websWrite(wp, "</tr>");
	}
#endif

	websWrite(wp, "</table>");
}

void asp_volumne(webs_t wp, int nVolumne)
{
	int i;

	for(i=0; i<10; i++)
	{
		if (i == nVolumne)
			websWrite(wp, "<option selected>%d</option>", (i+1));
		else
			websWrite(wp, "<option>%d</option>", (i+1));
	}
}

void asp_agc_gainup(webs_t wp, int nagc_db)
{
	int i;

	for(i=0; i<9; i++)
	{
		if (i == nagc_db)
			websWrite(wp, "<option selected>%d</option>", (i+1));
		else
			websWrite(wp, "<option>%d</option>", (i+1));
	}

}

void asp_agc_gaindown(webs_t wp, int nagc_db)
{
	int i;

	for(i=0; i<9; i++)
	{
		if (i == nagc_db)
			websWrite(wp, "<option selected>-%d</option>", (i+1));
		else
			websWrite(wp, "<option>-%d</option>", (i+1));
	}

}
void asp_maxDelay(webs_t wp, int nMaxDelay)
{
	int i;

#if 1
	if( nMaxDelay < 13 )
		nMaxDelay = 13;
	else if( nMaxDelay > 60 )
		nMaxDelay = 60;

	for(i=13; i<=60; i++)
	{
		if (nMaxDelay == i)
			websWrite(wp, "<option value=%d selected>%d</option>", i, i * 10);
		else
			websWrite(wp, "<option value=%d >%d</option>", i, i * 10);
	}
#else
	for(i=60; i<=180; i+=30)
	{
		if (nMaxDelay == i)
			websWrite(wp, "<option selected>%d</option>", i);
		else
			websWrite(wp, "<option>%d</option>", i);
	}
#endif
}

void asp_echoTail(webs_t wp, int nEchoTail)
{
	int i;
	char option[] = {1, 2, 4, 8, 16, 32};

	for(i=0; i<sizeof(option); i++)
	{
		if (option[i] == nEchoTail)
			websWrite(wp, "<option selected>%d</option>", option[i]);
		else
			websWrite(wp, "<option>%d</option>", option[i]);
	}
}

void asp_jitterDelay(webs_t wp, int nJitterDelay)
{
	int i;

	if( nJitterDelay < 4 || nJitterDelay > 40 )
		nJitterDelay = 4;

	for( i = 4; i <= 40; i ++ ) {
		if( i == nJitterDelay )
			websWrite(wp, "<option value=%d selected>%d</option>", i, i * 10);
		else
			websWrite(wp, "<option value=%d>%d</option>", i, i * 10);
	}
}

void asp_jitterFactor(webs_t wp, int nJitterFactor)
{
	int i;

	if( nJitterFactor < 0 || nJitterFactor > 13 )
		nJitterFactor = 7;

	for( i = 0; i <= 13; i ++ ) {
		if( i == nJitterFactor )
			websWrite(wp, "<option value=%d selected>%d</option>", i, i );
		else
			websWrite(wp, "<option value=%d>%d</option>", i, i );
	}
}

void asp_sip_speed_dial(webs_t wp, voipCfgPortParam_t *pCfg)
{
	int i;

	for (i=0; i<MAX_SPEED_DIAL; i++)
	{
		websWrite(wp,
			"<tr bgcolor=#ddeeff>" \
			"<td align=center>%d</td>", i);

		websWrite(wp,
			"<td><input type=text id=spd_name name=spd_name%d size=10 maxlength=%d value=\"%s\"></td>",
			i, MAX_SPEED_DIAL_NAME - 1, pCfg->speed_dial[i].name);

		websWrite(wp,
			"<td><input type=text id=spd_url name=spd_url%d size=20 maxlength=%d value=\"%s\" onChange=\"spd_dial_edit()\"></td>",
			i, MAX_SPEED_DIAL_URL - 1, pCfg->speed_dial[i].url);

		websWrite(wp,
			"<td align=center><input type=checkbox name=spd_sel %s></td>",
			pCfg->speed_dial[i].url[0] ? "" : "disabled");

		websWrite(wp, "</tr>");
	}
}

#ifdef CONFIG_APP_BOA
int asp_voip_GeneralGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_GeneralGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	voipCfgParam_t *pVoIPCfg;
	voipCfgPortParam_t *pCfg;
	int i;
	int voip_port;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	int certNum=0;
#endif

	if (web_flash_get(&pVoIPCfg) != 0)
		return -1;

	voip_port = atoi(websGetVar(wp, T("port"), "0"));
	if (voip_port < 0 || voip_port >= g_VoIP_Ports)
		return -1;

	pCfg = &pVoIPCfg->ports[voip_port];

	if (strcmp(argv[0], "voip_port")==0)
	{
		websWrite(wp, "%d", voip_port);
	}
	// proxy
	else if (strcmp(argv[0], "proxy")==0)
	{
		websWrite(wp, "<p><b>Default Proxy</b>\n"  \
			"<table cellSpacing=1 cellPadding=2 border=0>\n" \
			"<tr>\n" \
			"<td bgColor=#aaddff width=155>Select Default Proxy</td>\n" \
			"<td bgColor=#ddeeff width=170>"
		);

		websWrite(wp, "<select name=default_proxy>");
		for (i=0; i<MAX_PROXY ;i++)
		{
			websWrite(wp, "<option value=%d %s>Proxy%d</option>",
				i,
				i == pCfg->default_proxy ? "selected" : "",
				i
			);
		}

		websWrite(wp, "</select>");
		websWrite(wp, "</td></tr></table>");

		for (i=0; i<MAX_PROXY; i++)
		{
			websWrite(wp, "<p><b>Proxy%d</b>\n", i);

			// account
			websWrite(wp,
				"<table cellSpacing=1 cellPadding=2 border=0>\n" \
				"<tr>\n" \
				"<td bgColor=#aaddff width=155>Display Name</td>\n" \
				"<td bgColor=#ddeeff width=170>\n" \
				"<input type=text id=display name=display%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].display_name);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Number</td>\n" \
 				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=number name=number%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].number);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Login ID</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=loginID name=loginID%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].login_id);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Password</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=password id=password name=password%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].password);
			// register server
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Proxy</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=checkbox id=proxyEnable name=proxyEnable%d %s>Enable\n" \
				"</td></tr>\n",
				i, (pCfg->proxies[i].enable & PROXY_ENABLED) ? "checked" : "");
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Proxy Addr</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=proxyAddr name=proxyAddr%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].addr);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Proxy Port</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=proxyPort name=proxyPort%d size=10 maxlength=5 value=%d></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].port);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>SIP Domain</td>\n" \
				"<td bgColor=#ddeeff>" \
				"<input type=text id=domain_name name=domain_name%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].domain_name);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Reg Expire (sec)</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=regExpiry name=regExpiry%d size=20 maxlength=5 value=%d></td>\n"
				"</tr>\n",
				i, pCfg->proxies[i].reg_expire);
			// nat traversal server
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Outbound Proxy</td>\n" \
				"<td bgColor=#ddeeff><input type=checkbox id=obEnable name=obEnable%d %s>Enable</td>\n" \
				"</tr>\n",
				i, (pCfg->proxies[i].outbound_enable) ? "checked" : "");
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Outbound Proxy Addr</td>\n" \
				"<td bgColor=#ddeeff>" \
				"<input type=text id=obProxyAddr name=obProxyAddr%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].outbound_addr);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Outbound Proxy Port</td>\n" \
				"<td bgColor=#ddeeff>" \
				"<input type=text id=obProxyPort name=obProxyPort%d size=10 maxlength=5 value=%d></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].outbound_port);
			websWrite(wp,
				"<tr>\n"	\
				"<td bgColor=#aaddff>Nortel SoftSwitch</td>\n" \
				"<td bgColor=#ddeeff>\n" \
    			"<input type=checkbox id=proxyNortel name=proxyNortel%d %s %s onclick=\"check_nortel_proxy()\">Enable\n" \
				"</td></tr>\n",
				i,
				(pCfg->proxies[i].enable & PROXY_NORTEL) ? "checked" : "",
				i == 0 ? "" : "disabled");
#ifdef CONFIG_RTK_VOIP_SIP_TLS
			if(voip_port == 0 && i == 0)
				certNum=1;
			else if(voip_port == 0 && i == 1)
				certNum=2;
			else if(voip_port == 1 && i == 0)
				certNum=3;
			else if(voip_port == 1 && i == 1)
				certNum=4;

			if(certNum >= 1 && certNum <= 4){
				websWrite(wp,
					"<tr>\n" \
					"<td bgColor=#aaddff>SIP TLS Enable</td>\n" \
					"<td bgColor=#ddeeff>\n" \
					"<input type=checkbox id=sipTLSEnable name=sipTLSEnable%d %s>Enable</td>\n" \
					"</tr>\n",
					i, (pCfg->proxies[i].siptls_enable) ? "checked" : "");
				if(pCfg->proxies[i].siptls_enable)
					websWrite(wp,
						"<tr>\n" \
						"<td bgColor=#aaddff>TLS Certificate</td>\n" \
						"<td bgColor=#ddeeff>\n" \
						"<input type=\"button\" value=\"Show Certificate\" name=\"showCert\" onClick=\"window.open('voip_tls.asp?cert=%d','SIP_TLS','height=400, left=400');\"></td>\n" \
						"</tr>\n",certNum);
				}
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Register Status</td>\n" \
				"<td bgColor=#ddeeff><iframe src=voip_sip_status.asp?port=%d&index=%d " \
				"frameborder=0 height=20 width=160 scrolling=no marginheight=0 marginwidth=0>\n" \
				"</iframe></td>\n" \
				"</tr>\n",
				voip_port, i);
			websWrite(wp, "</table>");
		}
	}
	else if (strcmp(argv[0], "stun")==0)
	{
		websWrite(wp,
			"<tr>\n" \
			"<td bgColor=#aaddff>Stun</td>\n" \
			"<td bgColor=#ddeeff><input type=checkbox id=stunEnable name=stunEnable %s>Enable</td>\n" \
			"</tr>\n",
			(pCfg->stun_enable) ? "checked" : "");
		websWrite(wp,
			"<tr>\n" \
			"<td bgColor=#aaddff>Stun Server Addr</td>\n" \
			"<td bgColor=#ddeeff>\n" \
			"<input type=text id=stunAddr name=stunAddr size=20 maxlength=39 value=%s></td>\n" \
			"</tr>\n",
			pCfg->stun_addr);
		websWrite(wp,
			"<tr>\n" \
			"<td bgColor=#aaddff>Stun Server Port</td>\n" \
			"<td bgColor=#ddeeff>\n" \
			"<input type=text id=stunPort name=stunPort size=10 maxlength=5 value=%d></td>\n" \
			"</tr>\n",
			pCfg->stun_port);
	}
	else if (strcmp(argv[0], "registerStatus")==0)
	{
		FILE *fh;
		char buf[MAX_VOIP_PORTS * MAX_PROXY];

		i = atoi(websGetVar(wp, T("index"), "0"));
		if (i < 0 || i >= MAX_PROXY)
		{
			printf("Unknown proxy index %d", i);
			websWrite(wp, "%s", "ERROR");
			return 0;
		}
//		fprintf(stderr, "proxy index %d", i);

		if ((pCfg->proxies[i].enable & PROXY_ENABLED) == 0) {
			websWrite(wp, "%s", "Not Registered");
			return 0;
		}

		fh = fopen(_PATH_TMP_STATUS, "r");
		if (!fh) {
			printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
			printf("\nerrno=%d\n", errno);
		}

		memset(buf, 0, sizeof(buf));
		if (fread(buf, sizeof(buf), 1, fh) == 0) {
			printf("Web: The content of /tmp/status is NULL!!\n");
			printf("\nerrno=%d\n", errno);
			websWrite(wp, "%s", "ERROR");
		}
		else {
//			fprintf(stderr, "buf is %s.\n", buf);
			switch (buf[voip_port * MAX_PROXY + i]) {
				case '0':
					websWrite(wp, "%s", "Not Registered");
					break;
				case '1':
					websWrite(wp, "%s", "Registered");
					break;
				case '2':
					websWrite(wp, "%s", "Registering");
					break;
				default:
					websWrite(wp, "%s", "ERROR");
					break;
			}
		}

		fclose(fh);
	}
	// advanced
	else if (strcmp(argv[0], "sipPort")==0)
		websWrite(wp, "%d", pCfg->sip_port);
	else if (strcmp(argv[0], "sipPorts")==0)
	{
		for (i=0; i<g_VoIP_Ports; i++)
		{
			websWrite(wp,
				"<input type=hidden id=sipPorts name=sipPorts value=\"%d\">",
				pVoIPCfg->ports[i].sip_port);
		}
	}
	else if (strcmp(argv[0], "rtpPort")==0)
		websWrite(wp, "%d", pCfg->media_port);
	else if (strcmp(argv[0], "rtpPorts")==0)
	{
		for (i=0; i<g_VoIP_Ports; i++)
		{
			websWrite(wp,
				"<input type=hidden id=rtpPorts name=rtpPorts value=\"%d\">",
				pVoIPCfg->ports[i].media_port);
		}
	}
	else if (strcmp(argv[0], "dtmfMode")==0)
	{
		for (i=0; i<DTMF_MAX; i++)
		{
			if (i == pCfg->dtmf_mode)
				websWrite(wp, "<option selected>%s</option>", dtmf[i]);
			else
				websWrite(wp, "<option>%s</option>", dtmf[i]);
		}
	}
	else if (strcmp(argv[0], "caller_id")==0)
	{
		websWrite(wp, "<select name=caller_id %s>",
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");

		for (i=0; i<CID_MAX ;i++)
		{
			if (i == (pCfg->caller_id_mode & 7))
				websWrite(wp, "<option selected>%s</option>", cid[i]);
			else
				websWrite(wp, "<option>%s</option>", cid[i]);
		}

		websWrite(wp, "</select>");
	}
#ifdef SUPPORT_VOICE_QOS
	else if (strcmp(argv[0], "display_voice_qos")==0)
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "voice_qos")==0)
	{
		for (i=0; i<DSCP_MAX ;i++)
		{
			if (i == pCfg->voice_qos)
				websWrite(wp, "<option selected>%s</option>", dscp[i]);
			else
				websWrite(wp, "<option>%s</option>", dscp[i]);
		}
	}
#else
	else if (strcmp(argv[0], "display_voice_qos")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
	else if (strcmp(argv[0], "voice_qos")==0)
	{
		websWrite(wp, "%s", "");
	}
#endif
	else if (strcmp(argv[0], "sipInfo_duration")==0)
	{
		if (pCfg->dtmf_mode == DTMF_SIPINFO)
			websWrite(wp, "%d", pCfg->sip_info_duration);
		else
			websWrite(wp, "%d disabled", pCfg->sip_info_duration);
	}
	else if (strcmp(argv[0], "dtmf_2833_pt")==0)
	{
		if (pCfg->dtmf_mode == DTMF_RFC2833)
			websWrite(wp, "%d", pCfg->dtmf_2833_pt);
		else
			websWrite(wp, "%d disabled", pCfg->dtmf_2833_pt);
	}
	else if (strcmp(argv[0], "dtmf_2833_pi")==0)
	{
		if (pCfg->dtmf_mode == DTMF_RFC2833)
			websWrite(wp, "%d", pCfg->dtmf_2833_pi);
		else
			websWrite(wp, "%d disabled", pCfg->dtmf_2833_pi);
	}
	else if (strcmp(argv[0], "fax_modem_2833_pt_same_dtmf")==0)
		websWrite(wp, "%s", (pCfg->fax_modem_2833_pt_same_dtmf) ? "checked" : "");
	else if (strcmp(argv[0], "fax_modem_2833_pt")==0)
	{
		if (pCfg->dtmf_mode == DTMF_RFC2833)
			websWrite(wp, "%d", pCfg->fax_modem_2833_pt);
		else
			websWrite(wp, "%d disabled", pCfg->fax_modem_2833_pt);
	}
	else if (strcmp(argv[0], "fax_modem_2833_pi")==0)
	{
		if (pCfg->dtmf_mode == DTMF_RFC2833)
			websWrite(wp, "%d", pCfg->fax_modem_2833_pi);
		else
			websWrite(wp, "%d disabled", pCfg->fax_modem_2833_pi);
	}
	else if (strcmp(argv[0], "call_waiting")==0)
		websWrite(wp, "%s", (pCfg->call_waiting_enable) ? "checked" : "");
	else if (strcmp(argv[0], "call_waiting_cid")==0)
	{
		if (pCfg->call_waiting_enable == 0)
			websWrite(wp, "%s", (pCfg->call_waiting_cid) ? "checked disabled" : "disabled");
		else
			websWrite(wp, "%s", (pCfg->call_waiting_cid) ? "checked" : "");
	}
	else if (strcmp(argv[0], "reject_direct_ip_call")==0)
		websWrite(wp, "%s", (pCfg->direct_ip_call == 0) ? "checked" : "");
	// forward
	else if (strcmp(argv[0], "CFAll")==0)
	{
   		websWrite(wp, "<input type=\"radio\" name=\"CFAll\" value=0 %s>Off",
   			pCfg->uc_forward_enable == 0 ? "checked" : "");
   		websWrite(wp, "<input type=\"radio\" name=\"CFAll\" value=1 %s>VoIP",
   			pCfg->uc_forward_enable == 1 ? "checked" : "");
		websWrite(wp, "<input type=\"radio\" name=\"CFAll\" value=2 %s %s>PSTN",
			pCfg->uc_forward_enable == 2 ? "checked" : "",
			!RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
	}
	else if (strcmp(argv[0], "CFAll_No")==0)
	{
		websWrite(wp, "<input type=text name=CFAll_No size=20 maxlength=39 value=%s>",
			pCfg->uc_forward);

	}
	else if (strcmp(argv[0], "CFBusy")==0)
	{
   		websWrite(wp, "<input type=\"radio\" name=\"CFBusy\" value=0 %s %s>Off",
 			pCfg->busy_forward_enable == 0 ? "checked" : "",
			RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
		websWrite(wp, "<input type=\"radio\" name=\"CFBusy\" value=1 %s %s>VoIP",
    		pCfg->busy_forward_enable == 1 ? "checked" : "",
			RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
	}
	else if (strcmp(argv[0], "CFBusy_No")==0)
	{
		websWrite(wp, "<input type=text name=CFBusy_No size=20 maxlength=39 value=\"%s\" %s>",
			pCfg->busy_forward,
			RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ) ? "disabled=true" : "");
	}
	else if (strcmp(argv[0], "CFNoAns")==0 && !RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ))
	{
   		websWrite(wp,
			"<tr>" \
			"<td bgColor=#aaddff>No Answer Forward to</td>" \
			"<td bgColor=#ddeeff>" \
   			"<input type=\"radio\" name=\"CFNoAns\" value=0 %s>Off" \
   			"<input type=\"radio\" name=\"CFNoAns\" value=1 %s>VoIP" \
			"</td>" \
			"</tr>",
			pCfg->na_forward_enable == 0 ? "checked" : "",
			pCfg->na_forward_enable == 1 ? "checked" : ""
		);
   		websWrite(wp,
			"<tr>" \
			"<td bgColor=#aaddff>No Answer Number</td>" \
			"<td bgColor=#ddeeff>" \
			"<input type=text name=CFNoAns_No size=20 maxlength=39 value=\"%s\">" \
			"</td>" \
			"</tr>",
			pCfg->na_forward
		);
   		websWrite(wp,
			"<tr>" \
			"<td bgColor=#aaddff>No Answer Time (sec)</td>" \
			"<td bgColor=#ddeeff>" \
			"<input type=text name=CFNoAns_Time size=20 maxlength=39 value=%d>" \
			"</td>" \
			"</tr>",
			pCfg->na_forward_time
		);
	#ifdef FXO_REDIAL
		if (voip_port == 0) // if FXS0
		{
	   		websWrite(wp,
				"<tr>" \
				"<td bgColor=#aaddff>No Answer Forward for PSTN</td>" \
				"<td bgColor=#ddeeff>" \
   				"<select name=PSTN_CFNoAns>" \
				"<option %s value=0>Off</option>" \
				"<option %s value=1>2 Stage Dialing</option>" \
				"<option %s value=2>Direct Forward</option>" \
				"</select>" \
				"</td>" \
				"</tr>",
				pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward_enable == 0 ? "selected" : "",
				pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward_enable == 1 ? "selected" : "",
				pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward_enable == 2 ? "selected" : ""
			);
   			websWrite(wp,
				"<tr>" \
				"<td bgColor=#aaddff>No Answer Number for PSTN</td>" \
				"<td bgColor=#ddeeff>" \
				"<input type=text name=PSTN_CFNoAns_No size=20 maxlength=39 value=\"%s\">" \
				"</td>" \
				"</tr>",
				pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward
			);
   			websWrite(wp,
				"<tr>" \
				"<td bgColor=#aaddff>No Answer Time for PSTN</td>" \
				"<td bgColor=#ddeeff>" \
				"<input type=text name=PSTN_CFNoAns_Time size=20 maxlength=39 value=%d>" \
				"</td>" \
				"</tr>",
				pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward_time
			);
		}
	#endif
	}
	else if (strcmp(argv[0], "CFNoAns")==0 && RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ))
	{
	#ifdef FXO_REDIAL
		// do nothing
	#else
   		websWrite(wp,
			"<tr>" \
			"<td bgColor=#aaddff>No Answer Forward to</td>" \
			"<td bgColor=#ddeeff>" \
   			"<input type=\"radio\" name=\"CFNoAns\" value=0 %s disabled=true>Off" \
   			"<input type=\"radio\" name=\"CFNoAns\" value=1 %s disabled=true>VoIP" \
			"</td>" \
			"</tr>",
			pCfg->na_forward_enable == 0 ? "checked" : "",
			pCfg->na_forward_enable == 1 ? "checked" : ""
		);
   		websWrite(wp,
			"<tr>" \
			"<td bgColor=#aaddff>No Answer Number</td>" \
			"<td bgColor=#ddeeff>" \
			"<input type=text name=CFNoAns_No size=20 maxlength=39 value=\"%s\" disabled=true>" \
			"</td>" \
			"</tr>",
			pCfg->na_forward
		);
   		websWrite(wp,
			"<tr>" \
			"<td bgColor=#aaddff>No Answer Time (sec)</td>" \
			"<td bgColor=#ddeeff>" \
			"<input type=text name=CFNoAns_Time size=20 maxlength=39 value=%d disabled=true>" \
			"</td>" \
			"</tr>",
			pCfg->na_forward_time
		);
	#endif
	}
	// Speed dial
	else if (strcmp(argv[0], "speed_dial_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no speed_dial, but FXO still need it. */
		if( RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) )
			;
		else
#endif
		{
			websWrite(wp, "<p>\n<b>Speed Dial</b>\n" );
		}
	}
	else if (strcmp(argv[0], "speed_dial_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no speed_dial, but FXO still need it. */
		if( RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) )
			websWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "speed_dial")==0)
		asp_sip_speed_dial(wp, pCfg);
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	else if (strcmp(argv[0], "display_dialplan_title")==0) {
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no dialplan, but FXO still need it. */
		if( RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) )
			;
		else
#endif
		{
			websWrite(wp, "%s", "<p><b>Dial plan</b>");
		}
	} else if (strcmp(argv[0], "display_dialplan")==0) {
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no dialplan, but FXO still need it. */
		if( RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) )
			websWrite(wp, "style=\"display:none\"");
		else
#endif
		{
			websWrite(wp, "%s", "");
		}
	} else if (strcmp(argv[0], "dialplan")==0)
		websWrite(wp, "%s", pCfg->dialplan);
	else if (strcmp(argv[0], "ReplaceRuleOption")==0) {
    	websWrite(wp, "<input type=\"radio\" name=\"ReplaceRuleOption\" value=1 %s>On",
    				( pCfg ->replace_rule_option ? "checked" : "" ) );
    	websWrite(wp, "<input type=\"radio\" name=\"ReplaceRuleOption\" value=0 %s>Off",
    				( !pCfg ->replace_rule_option ? "checked" : "" ) );
	} else if (strcmp(argv[0], "ReplaceRuleSource")==0)
		websWrite(wp, "%s", pCfg->replace_rule_source);
	else if (strcmp(argv[0], "ReplaceRuleTarget")==0)
		websWrite(wp, "%s", pCfg->replace_rule_target);
	else if (strcmp(argv[0], "AutoPrefix")==0)
		websWrite(wp, "%s", pCfg->auto_prefix);
	else if (strcmp(argv[0], "PrefixUnsetPlan")==0)
		websWrite(wp, "%s", pCfg->prefix_unset_plan);
#else
	else if (strcmp(argv[0], "display_dialplan_title")==0)
		websWrite(wp, "%s", "");
  	else if (strcmp(argv[0], "display_dialplan")==0)
  		websWrite(wp, "%s", "style=\"display:none\"");
  	else if (strcmp(argv[0], "dialplan")==0)
		websWrite(wp, "%s", "");
  	else if (strcmp(argv[0], "ReplaceRuleOption")==0)
    	websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "ReplaceRuleSource")==0)
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "ReplaceRuleTarget")==0)
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "AutoPrefix")==0)
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "PrefixUnsetPlan")==0)
		websWrite(wp, "%s", "");
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
	// PSTN Routing Prefix
	else if (strcmp(argv[0], "PSTNRoutingPrefix")==0)
		websWrite(wp, "%s", pCfg->PSTN_routing_prefix );
	else if (strcmp(argv[0], "PSTNRoutingPrefixDisabled")==0)
		websWrite(wp, "%s", ( RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "") );

	// DSP
	else if (strcmp(argv[0], "codec_var") == 0)
		asp_sip_codec_var(wp, pCfg);
	else if (strcmp(argv[0], "codec") == 0)
		asp_sip_codec(wp, pCfg);
	else if (strcmp(argv[0], "codec_opt") == 0)
		asp_sip_codec_opt(wp, pCfg);
	else if (strcmp(argv[0], "slic_txVolumne")==0)
		asp_volumne(wp, pCfg->slic_txVolumne);
	else if (strcmp(argv[0], "slic_rxVolumne")==0)
		asp_volumne(wp, pCfg->slic_rxVolumne);
	else if (strcmp(argv[0], "maxDelay")==0)
		asp_maxDelay(wp, pCfg->maxDelay);
	else if (strcmp(argv[0], "echoTail")==0)
		asp_echoTail(wp, pCfg->echoTail);
	else if (strcmp(argv[0], "flash_hook_time")==0)
	{
		websWrite(wp, "<input type=text name=flash_hook_time_min size=4 maxlength=5 value=%d %s>" \
			" <  Flash Time  < " \
			"<input type=text name=flash_hook_time size=4 maxlength=5 value=%d %s>",
			pCfg->flash_hook_time_min,
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "",
			pCfg->flash_hook_time,
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : ""
		);
	}
	else if (strcmp(argv[0], "spk_voice_gain")==0)
		websWrite(wp, "%d", pCfg->spk_voice_gain);
	else if (strcmp(argv[0], "mic_voice_gain")==0)
		websWrite(wp, "%d", pCfg->mic_voice_gain);
	else if (strcmp(argv[0], "useLec")==0)
		websWrite(wp, "%s", (pCfg->lec) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useNlp")==0)
		websWrite(wp, "%s", (pCfg->nlp) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useVad")==0)
		websWrite(wp, "%s", (pCfg->vad) ? T("checked") : T(""));
	else if (strcmp(argv[0], "Vad_threshold")==0)
		websWrite(wp, "%d", pCfg->vad_thr);
	else if (strcmp(argv[0], "useCng")==0)
		websWrite(wp, "%s", (pCfg->cng) ? T("checked") : T(""));
	else if (strcmp(argv[0], "Cng_threshold")==0)
		websWrite(wp, "%d", pCfg->cng_thr);
	else if (strcmp(argv[0], "sid_adjust_level") == 0)
		websWrite(wp, "%s", (pCfg->sid_gainmode == 2) ? "checked" : "");
	else if (strcmp(argv[0], "sid_fixed_level") == 0)
		websWrite(wp, "%s", (pCfg->sid_gainmode == 1) ? "checked" : "");
	else if (strcmp(argv[0], "sid_config_enable") == 0)
		websWrite(wp, "%s", (pCfg->sid_gainmode == 0) ? "checked" : "");
	else if (strcmp(argv[0], "sid_noiselevel")==0)
		websWrite(wp, "%d", pCfg->sid_noiselevel);
	else if (strcmp(argv[0], "sid_noisegain")==0)
		websWrite(wp, "%d", pCfg->sid_noisegain);
	else if (strcmp(argv[0], "usePLC")==0)
		websWrite(wp, "%s", (pCfg->PLC) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_CNG_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x1) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_CNG_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x100) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANS_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x2) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANS_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x200) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANSAM_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x4) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANSAM_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x400) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANSBAR_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x8) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANSBAR_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x800) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANSAMBAR_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x10) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_ANSAMBAR_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x1000) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_BELLANS_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x20) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_BELLANS_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x2000) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_V22ANS_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x40) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_V22ANS_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x4000) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_V8bis_Cre_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x80) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_V8bis_Cre_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x8000) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_V21flag_TDM")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x10000) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useANSTONE_V21flag_IP")==0)
		websWrite(wp, "%s", (pCfg->anstone&0x20000) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useRTCP")==0)
		websWrite(wp, "%s", (pCfg->RTCP_Interval) ? T("checked") : T(""));
	else if (strcmp(argv[0], "RTCPInterval")==0)
		websWrite(wp, "%u", pCfg->RTCP_Interval);	
	else if (strcmp(argv[0], "useRTCPXR")==0)
		websWrite(wp, "%s", (pCfg->RTCP_XR) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useFaxModem2833Relay")==0)
		websWrite(wp, "%s", (pCfg->faxmodem_rfc2833&0x1) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useFaxModemInbandRemoval")==0)
		websWrite(wp, "%s", (pCfg->faxmodem_rfc2833&0x2) ? T("checked") : T(""));
	else if (strcmp(argv[0], "useFaxModem2833RxTonePlay")==0)
		websWrite(wp, "%s", (pCfg->faxmodem_rfc2833&0x4) ? T("checked") : T(""));
	else if (strcmp(argv[0], "CFuseSpeaker")==0)
		websWrite(wp, "%s", (pCfg->speaker_agc) ? T("checked") : T(""));
	else if (strcmp(argv[0], "CF_spk_AGC_level")==0)
		asp_agc_gainup(wp, pCfg->spk_agc_lvl);
	else if (strcmp(argv[0], "CF_spk_AGC_up_limit")==0)
		asp_agc_gainup(wp, pCfg->spk_agc_gu);
	else if (strcmp(argv[0], "CF_spk_AGC_down_limit")==0)
		asp_agc_gaindown(wp, pCfg->spk_agc_gd);
	else if (strcmp(argv[0], "CFuseMIC")==0)
		websWrite(wp, "%s", (pCfg->mic_agc) ? T("checked") : T(""));
	else if (strcmp(argv[0], "CF_mic_AGC_level")==0)
		asp_agc_gainup(wp, pCfg->mic_agc_lvl);
	else if (strcmp(argv[0], "CF_mic_AGC_up_limit")==0)
		asp_agc_gainup(wp, pCfg->mic_agc_gu);
	else if (strcmp(argv[0], "CF_mic_AGC_down_limit")==0)
		asp_agc_gaindown(wp, pCfg->mic_agc_gd);
	else if (strcmp(argv[0], "FSKdatesync")==0)
	{
		websWrite(wp, "<input type=checkbox name=FSKdatesync size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x080) ? T("checked") : T(""),
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
	}
	else if (strcmp(argv[0], "revPolarity")==0)
	{
		websWrite(wp, "<input type=checkbox name=revPolarity size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x040) ? T("checked") : T(""),
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
	}
	else if (strcmp(argv[0], "sRing")==0)
	{
		websWrite(wp, "<input type=checkbox name=sRing size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x020) ? T("checked") : T(""),
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
	}
	else if (strcmp(argv[0], "dualTone")==0)
	{
		websWrite(wp, "<input type=checkbox name=dualTone size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x010) ? T("checked") : T(""),
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
	}
	else if (strcmp(argv[0], "PriorRing")==0)
	{
		websWrite(wp, "<input type=checkbox name=PriorRing size=20 %s>Enable",
			(pCfg->caller_id_mode & 0x008) ? T("checked") : T(""));
	}
	else if (strcmp(argv[0], "cid_dtmfMode_S")==0)
	{
		websWrite(wp, "<select name=cid_dtmfMode_S %s>",
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");

		for (i=0; i<CID_DTMF_MAX; i++)
		{
			if (i == (pCfg->cid_dtmf_mode & 0x03))
				websWrite(wp, "<option selected>%s</option>", cid_dtmf[i]);
			else
				websWrite(wp, "<option>%s</option>", cid_dtmf[i]);
		}

		websWrite(wp, "</select>");
	}
	else if (strcmp(argv[0], "cid_dtmfMode_E")==0)
	{
		websWrite(wp, "<select name=cid_dtmfMode_E %s>",
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");

		for (i=0; i<CID_DTMF_MAX; i++)
		{
			if (i == ((pCfg->cid_dtmf_mode>>2) & 0x03))
				websWrite(wp, "<option selected>%s</option>", cid_dtmf[i]);
			else
				websWrite(wp, "<option>%s</option>", cid_dtmf[i]);
		}

		websWrite(wp, "</select>");
	}
	else if (strcmp(argv[0], "SoftFskGen")==0)
	{
		websWrite(wp, "<input type=checkbox name=SoftFskGen size=20 %s %s>Enable",
			pCfg->cid_fsk_gen_mode ? T("checked") : T(""),
			!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");
	}
	else if (strcmp(argv[0], "jitterDelay")==0)
		asp_jitterDelay(wp, pCfg->jitter_delay);
	else if (strcmp(argv[0], "jitterFactor")==0)
		asp_jitterFactor(wp, pCfg->jitter_factor);
#ifdef CONFIG_RTK_VOIP_T38	/*kernel config true*/
	else if (strcmp(argv[0], "T38_BUILD")==0)
		websWrite(wp, "%s", "");
	//T.38 config
	else if(strcmp(argv[0], "useT38")==0)
		websWrite(wp, "%s", (pCfg->useT38) ? "checked" : "");
	else if(strcmp(argv[0], "T38_PORT")==0)
		websWrite(wp, "%d", pCfg->T38_port);
	else if(strcmp(argv[0], "t38Ports")==0)
	{
		for (i=0; i<g_VoIP_Ports; i++)
		{
			websWrite(wp,
				"<input type=hidden id=t38Ports name=t38Ports value=\"%d\">",
				pVoIPCfg->ports[i].T38_port);
		}
	}
	else if(strcmp(argv[0], "T38ParamEnable")==0)
		websWrite(wp, "%s", (pCfg->T38ParamEnable) ? "checked" : "");
	else if(strcmp(argv[0], "T38MaxBuffer")==0)
		websWrite(wp, "%d", pCfg->T38MaxBuffer);
	else if(strcmp(argv[0], "T38RateMgt")==0) {
		websWrite(wp,
			"<option value=1 %s>Local TCF</option>"
			"<option value=2 %s>Remote TCF</option>"
			,
			( pCfg->T38RateMgt == 1 ? "selected" : "" ),
			( pCfg->T38RateMgt != 1 ? "selected" : "" )
			);
	} else if(strcmp(argv[0], "T38MaxRate")==0) {
		websWrite(wp,
			"<option value=0 %s>2400</option>"
			"<option value=1 %s>4800</option>"
			"<option value=2 %s>7200</option>"
			"<option value=3 %s>9600</option>"
			"<option value=4 %s>12000</option>"
			"<option value=5 %s>14400</option>"
			,
			( pCfg->T38MaxRate == 0 ? "selected" : "" ),
			( pCfg->T38MaxRate == 1 ? "selected" : "" ),
			( pCfg->T38MaxRate == 2 ? "selected" : "" ),
			( pCfg->T38MaxRate == 3 ? "selected" : "" ),
			( pCfg->T38MaxRate == 4 ? "selected" : "" ),
			( pCfg->T38MaxRate >= 5 ? "selected" : "" )
			);
	} else if(strcmp(argv[0], "T38EnableECM")==0) {
		websWrite(wp, "%s", (pCfg->T38EnableECM) ? "checked" : "");
	} else if(strcmp(argv[0], "T38ECCSignal")==0) {
		websWrite(wp,
			"<option value=0 %s>0</option>"
			"<option value=1 %s>1</option>"
			"<option value=2 %s>2</option>"
			"<option value=3 %s>3</option>"
			"<option value=4 %s>4</option>"
			"<option value=5 %s>5</option>"
			"<option value=6 %s>6</option>"
			"<option value=7 %s>7</option>"
			,
			( pCfg->T38ECCSignal == 0 ? "selected" : "" ),
			( pCfg->T38ECCSignal == 1 ? "selected" : "" ),
			( pCfg->T38ECCSignal == 2 ? "selected" : "" ),
			( pCfg->T38ECCSignal == 3 ? "selected" : "" ),
			( pCfg->T38ECCSignal == 4 ? "selected" : "" ),
			( pCfg->T38ECCSignal == 5 || pCfg->T38ECCSignal > 7 ? "selected" : "" ),
			( pCfg->T38ECCSignal == 6 ? "selected" : "" ),
			( pCfg->T38ECCSignal == 7 ? "selected" : "" )
			);
	} else if(strcmp(argv[0], "T38ECCData")==0) {
		websWrite(wp,
			"<option value=0 %s>0</option>"
			"<option value=1 %s>1</option>"
			"<option value=2 %s>2</option>"
			,
			( pCfg->T38ECCData == 0 ? "selected" : "" ),
			( pCfg->T38ECCData == 1 ? "selected" : "" ),
			( pCfg->T38ECCData >= 2 ? "selected" : "" )
			);
	} else if(strcmp(argv[0], "T38EnableSpoof")==0) {
		websWrite(wp, "%s", (pCfg->T38EnableSpoof) ? "checked" : "");
	} else if(strcmp(argv[0], "T38DuplicateNum")==0) {
		websWrite(wp, 
			"<option value=0 %s>0</option>"
			"<option value=1 %s>1</option>"
			"<option value=2 %s>2</option>"
			, 
			( pCfg->T38DuplicateNum == 0 ? "selected" : "" ),
			( pCfg->T38DuplicateNum == 1 ? "selected" : "" ),
			( pCfg->T38DuplicateNum >= 2 ? "selected" : "" )
			);
	}
#else /*CONFIG_RTK_VOIP_T38*/
	else if (strcmp(argv[0], "T38_BUILD")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
	else if(strcmp(argv[0], "useT38")==0)
		websWrite(wp, "%s", "");
	else if(strcmp(argv[0], "T38_PORT")==0)
		websWrite(wp, "%d",5000);	/* give 5000 to pass js check. */
	else if(strcmp(argv[0], "t38Ports")==0)
	{
		for (i=0; i<g_VoIP_Ports; i++)
		{
			websWrite(wp,
				"<input type=hidden id=t38Ports name=t38Ports value=\"%d\">",
				5005);
		}
	}
	else if(strcmp(argv[0], "T38ParamEnable")==0)
		websWrite(wp, "%s", "");
	else if(strcmp(argv[0], "T38MaxBuffer")==0)
		websWrite(wp, "%d", 500);
	else if(strcmp(argv[0], "T38RateMgt")==0) {
		websWrite(wp,
			"<option value=2 selected>remote TCF</option>"
			);
	} else if(strcmp(argv[0], "T38MaxRate")==0) {
		websWrite(wp,
			"<option value=5 selected>14400</option>"
			);
	} else if(strcmp(argv[0], "T38EnableECM")==0) {
		websWrite(wp, "%s", "");
	} else if(strcmp(argv[0], "T38ECCSignal")==0) {
		websWrite(wp,
			"<option value=5 selected>5</option>"
			);
	} else if(strcmp(argv[0], "T38ECCData")==0) {
		websWrite(wp,
			"<option value=2 selected>2</option>"
			);
	} else if(strcmp(argv[0], "T38EnableSpoof")==0) {
		websWrite(wp, "%s", "");
	} else if(strcmp(argv[0], "T38DuplicateNum")==0) {
		websWrite(wp, 
			"<option value=0 selected>0</option>"
			);
	}
#endif /*CONFIG_RTK_VOIP_T38*/
	else if (strcmp(argv[0], "fax_modem_det_mode") == 0)
	{
		for (i=0; i<FAX_MODEM_DET_MAX; i++)
		{
			if (i == pCfg->fax_modem_det)
				websWrite(wp, "<option selected>%s</option>", fax_modem_det_string[i]);
			else
				websWrite(wp, "<option>%s</option>", fax_modem_det_string[i]);
		}
	}
	// RTP Redundant
#ifdef SUPPORT_RTP_REDUNDANT
	else if(strcmp(argv[0], "RTP_RED_BUILD") == 0)
		websWrite(wp, "");
	else if(strcmp(argv[0], "rtp_redundant_codec_options") == 0) {
		websWrite(wp,
			"<option value=-1>Disable</option>"
			"<option value=0 %s>PCM u-law</option>"
			"<option value=8 %s>PCM a-law</option>"
#ifdef CONFIG_RTK_VOIP_G729AB
			"<option value=18 %s>G.729</option>"
#endif
			,pCfg->rtp_redundant_codec == 0 ? "selected" : ""
			,pCfg->rtp_redundant_codec == 8 ? "selected" : ""
#ifdef CONFIG_RTK_VOIP_G729AB
			,pCfg->rtp_redundant_codec == 18 ? "selected" : ""
#endif
		);
	} else if(strcmp(argv[0], "rtp_redundant_payload_type") == 0)
		websWrite(wp, "%d", pCfg->rtp_redundant_payload_type);
#else
	else if(strcmp(argv[0], "RTP_RED_BUILD") == 0)
		websWrite(wp, "style=\"display:none\"");
	else if(strcmp(argv[0], "rtp_redundant_codec_options") == 0)
		websWrite(wp, "");
	else if(strcmp(argv[0], "rtp_redundant_payload_type") == 0)
		websWrite(wp, "");
#endif /* CONFIG_RTK_VOIP_SRTP */

	// V.152
	else if(strcmp(argv[0], "useV152")==0)
		websWrite(wp, "%s", (pCfg->useV152) ? "checked" : "");
	else if(strcmp(argv[0], "V152_payload_type")==0)
		websWrite(wp, "%d", pCfg->v152_payload_type);
	else if(strcmp(argv[0], "V152_codec_type")==0)
		websWrite(wp, "%d", pCfg->v152_codec_type);
	else if(strcmp(argv[0], "V152_codec_type_options")==0)
	{
		static const struct {
			unsigned char codec_type;
			const char *codec_type_string;
		} const v152_codec_options[] = {
			{ 0, "PCM u-law" },
			{ 8, "PCM a-law" },
		};
		#define SIZE_OF_V152_OPTIONS	( sizeof( v152_codec_options ) / sizeof( v152_codec_options[ 0 ] ) )

		for( i = 0; i < SIZE_OF_V152_OPTIONS; i ++ ) {
			if( v152_codec_options[ i ].codec_type == pCfg->v152_codec_type )
				websWrite( wp, "<option value=%d selected>%s</option>", v152_codec_options[ i ].codec_type, v152_codec_options[ i ].codec_type_string );
			else
				websWrite( wp, "<option value=%d>%s</option>", v152_codec_options[ i ].codec_type, v152_codec_options[ i ].codec_type_string );
		}
	}
	else if (strcmp(argv[0], "hotline_option_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no hotline option, but FXO still need it. */
		if( RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) )
			;
		else
#endif
		{
			websWrite(wp, "<p>\n<b>Hot Line</b>\n");
		}
	}
	else if (strcmp(argv[0], "hotline_option_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no hotline option, but FXO still need it. */
		if( RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) )
			websWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "hotline_enable") == 0)
	{
		websWrite(wp, "%s", (pCfg->hotline_enable) ? "checked" : "");
	}
	else if (strcmp(argv[0], "hotline_number") == 0)
	{
		websWrite(wp, "%s", pCfg->hotline_number);
	}
	else if (strcmp(argv[0], "dnd_always") == 0)
	{
		websWrite(wp, "%s", (pCfg->dnd_mode == 2) ? "checked" : "");
	}
	else if (strcmp(argv[0], "dnd_enable") == 0)
	{
		websWrite(wp, "%s", (pCfg->dnd_mode == 1) ? "checked" : "");
	}
	else if (strcmp(argv[0], "dnd_disable") == 0)
	{
		websWrite(wp, "%s", (pCfg->dnd_mode == 0) ? "checked" : "");
	}
	else if (strcmp(argv[0], "dnd_from_hour") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_from_hour);
	}
	else if (strcmp(argv[0], "dnd_from_min") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_from_min);
	}
	else if (strcmp(argv[0], "dnd_to_hour") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_to_hour);
	}
	else if (strcmp(argv[0], "dnd_to_min") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_to_min);
	}
#ifdef CONFIG_RTK_VOIP_SRTP
	else if(strcmp(argv[0], "SRTP_BUILD") == 0)
		websWrite(wp, "%s", "");
	else if(strcmp(argv[0], "useSRTP") == 0)
		websWrite(wp, "%s", (pCfg->security_enable) ? "checked" : "");
#else
	else if(strcmp(argv[0], "SRTP_BUILD") == 0)
		websWrite(wp, "%s", "style=\"display:none\"");
	else if(strcmp(argv[0], "useSRTP") == 0)
		websWrite(wp, "%s", "");
#endif /* CONFIG_RTK_VOIP_SRTP */
	// auth
	else if (strcmp(argv[0], "offhook_passwd")==0)
	{
		websWrite(wp, "%s", pCfg->offhook_passwd);
	}
	// alarm
	else if (strcmp(argv[0], "alarm_enable")==0)
	{
		websWrite(wp, "%s", ( pCfg->alarm_enable ? "checked" : "") );
	}
	else if (strcmp(argv[0], "alarm_hh")==0)
	{
		websWrite(wp, "%u", pCfg->alarm_time_hh);
	}
	else if (strcmp(argv[0], "alarm_mm")==0)
	{
		websWrite(wp, "%u", pCfg->alarm_time_mm);
	}
	else if (strcmp(argv[0], "alarm_disabled")==0)
	{
		websWrite(wp, "%s", ( !RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "") );
	}

	// abbreviated dial
	else if (strcmp(argv[0], "abbreviated_dial")==0)
	{
		for( i = 0; i < MAX_ABBR_DIAL_NUM; i ++ ) {
			websWrite(wp, "<tr bgcolor=#ddeeff>\n" );
			websWrite(wp, "<td algin=center><input type=text name=abbr_name%d size=10 maxlength=4 value=\"%s\"></td>\n", i, pCfg->abbr_dial[ i ].name );
			websWrite(wp, "<td algin=center><input type=text name=abbr_url%d size=20 maxlength=60 value=\"%s\"></td>\n", i, pCfg->abbr_dial[ i ].url );
			websWrite(wp, "</tr>\n" );
		}
	}
	else if (strcmp(argv[0], "not_ipphone_option_start")==0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		websWrite( wp, "<!--\n" );
#endif
	}
	else if (strcmp(argv[0], "not_ipphone_option_end")==0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		websWrite( wp, "-->\n" );
#endif
	}
	else if (strcmp(argv[0], "not_ipphone_table")==0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		websWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "not_dect_port_option")==0)
	{
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
		if( RTK_VOIP_IS_DECT_CH( voip_port, g_VoIP_Feature ) ) {
			websWrite(wp, "style=\"display:none\"");
		}
#endif
	}
	else if (strcmp(argv[0], "rtcp_xr_option")==0)
	{
#ifndef CONFIG_RTK_VOIP_RTCP_XR
		websWrite(wp, "style=\"display:none\"");
#endif
	}
	else
	{
		return -1;
	}

	return 0;
}

void asp_voip_GeneralSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;
	voipCfgPortParam_t *pCfg;
	int i;
	char *ptr;
	char szFrameSize[12], szPrecedence[12];
	char redirect_url[50];
	int voip_port;
	char szName[20];

	if (web_flash_get(&pVoIPCfg) != 0)
		return;

	voip_port = atoi(websGetVar(wp, T("voipPort"), "0"));
	if (voip_port < 0 || voip_port >= g_VoIP_Ports)
		return;

	pCfg = &pVoIPCfg->ports[voip_port];

	pCfg->default_proxy = atoi(websGetVar(wp, T("default_proxy"), "0"));

	/* Sip Proxy */
	for (i=0; i< MAX_PROXY; i++)
	{
		/* Sip Account */
		sprintf(szName, "display%d", i);
		strcpy(pCfg->proxies[i].display_name, websGetVar(wp, szName, T("")));
		sprintf(szName, "number%d", i);
		strcpy(pCfg->proxies[i].number, websGetVar(wp, szName, T("")));
		sprintf(szName, "loginID%d", i);
		strcpy(pCfg->proxies[i].login_id, websGetVar(wp, szName, T("")));
		sprintf(szName, "password%d", i);
		strcpy(pCfg->proxies[i].password, websGetVar(wp, szName, T("")));

		/* Register Server */
		pCfg->proxies[i].enable = 0;

		sprintf(szName, "proxyEnable%d", i);
		if (gstrcmp(websGetVar(wp, szName, T("")), "on") == 0)
			pCfg->proxies[i].enable |= PROXY_ENABLED;

		sprintf(szName, "proxyNortel%d", i);
		if (gstrcmp(websGetVar(wp, szName, T("")), "on") == 0)
			pCfg->proxies[i].enable |= PROXY_NORTEL;

		sprintf(szName, "proxyAddr%d", i);
		strcpy(pCfg->proxies[i].addr, websGetVar(wp, szName, T("")));

		sprintf(szName, "proxyPort%d", i);
		pCfg->proxies[i].port = atoi(websGetVar(wp, szName, T("5060")));
		if (pCfg->proxies[i].port == 0)
			pCfg->proxies[i].port = 5060;

		sprintf(szName, "domain_name%d", i);
		strcpy(pCfg->proxies[i].domain_name, websGetVar(wp, szName, T("")));

		sprintf(szName, "regExpiry%d", i);
		pCfg->proxies[i].reg_expire = atoi(websGetVar(wp, szName, T("60")));

#ifdef CONFIG_RTK_VOIP_SIP_TLS
		sprintf(szName, "sipTLSEnable%d", i);
		if (gstrcmp(websGetVar(wp, szName, T("")), "on") == 0)
			pCfg->proxies[i].siptls_enable=1;
		else
			pCfg->proxies[i].siptls_enable=0;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

		/* NAT Travsersal Server */
		sprintf(szName, "obEnable%d", i);
		pCfg->proxies[i].outbound_enable = !gstrcmp(websGetVar(wp, szName, T("")), "on");
		sprintf(szName, "obProxyPort%d", i);
		pCfg->proxies[i].outbound_port = atoi(websGetVar(wp, szName, T("5060")));
		if (pCfg->proxies[i].outbound_port == 0)
			pCfg->proxies[i].outbound_port = 5060;

		sprintf(szName, "obProxyAddr%d", i);
		strcpy(pCfg->proxies[i].outbound_addr, websGetVar(wp, szName, T("")));
	}

	/* NAT Traversal */
	pCfg->stun_enable = !gstrcmp(websGetVar(wp, "stunEnable", T("")), "on");
	pCfg->stun_port	= atoi(websGetVar(wp, "stunPort", T("3478")));
	strcpy(pCfg->stun_addr, websGetVar(wp, "stunAddr", T("")));

	/* Advanced */
	pCfg->sip_port 	= atoi(websGetVar(wp, T("sipPort"), T("5060")));
	pCfg->media_port 	= atoi(websGetVar(wp, T("rtpPort"), T("9000")));

	ptr	 = websGetVar(wp, T("dtmfMode"), T(""));
	for(i=0; i<DTMF_MAX; i++)
	{
		if (!gstrcmp(ptr, dtmf[i]))
			break;
	}
	if (i == DTMF_MAX)
		i = DTMF_INBAND;

	pCfg->dtmf_mode 		= i;
	pCfg->dtmf_2833_pt 		= atoi(websGetVar(wp, T("dtmf_2833_pt"), T("96")));
	pCfg->dtmf_2833_pi 		= atoi(websGetVar(wp, T("dtmf_2833_pi"), T("10")));
	pCfg->fax_modem_2833_pt_same_dtmf = !gstrcmp(websGetVar(wp, T("fax_modem_2833_pt_same_dtmf"), T("")), "on");
	pCfg->fax_modem_2833_pt		= atoi(websGetVar(wp, T("fax_modem_2833_pt"), T("101")));
	pCfg->fax_modem_2833_pi		= atoi(websGetVar(wp, T("fax_modem_2833_pi"), T("10")));
	pCfg->sip_info_duration		= atoi(websGetVar(wp, T("sipInfo_duration"), T("250")));
	pCfg->call_waiting_enable = !gstrcmp(websGetVar(wp, T("call_waiting"), T("")), "on");
	pCfg->call_waiting_cid = !gstrcmp(websGetVar(wp, T("call_waiting_cid"), T("")), "on");
	pCfg->direct_ip_call = gstrcmp(websGetVar(wp, T("reject_direct_ip_call"), T("")), "on");

	/* Forward Mode */
	//pCfg->uc_forward_enable = !gstrcmp(websGetVar(wp, T("CFAll"), T("")), "on");
	pCfg ->uc_forward_enable = atoi( websGetVar(wp, T("CFAll"), T("")) );
	strcpy(pCfg->uc_forward, websGetVar(wp, T("CFAll_No"), T("")));

	//pCfg->busy_forward_enable = !gstrcmp(websGetVar(wp, T("CFBusy"), T("")), "on");
	pCfg ->busy_forward_enable = atoi( websGetVar(wp, T("CFBusy"), T("")) );
	strcpy(pCfg->busy_forward, websGetVar(wp, T("CFBusy_No"), T("")));

#ifdef FXO_REDIAL
	if (!RTK_VOIP_IS_DAA_CH( voip_port, g_VoIP_Feature ))
	{
		pCfg->na_forward_enable = atoi(websGetVar(wp, T("CFNoAns"), T("")));
		pCfg->na_forward_time = atoi(websGetVar(wp, T("CFNoAns_Time"), T("")));
		strcpy(pCfg->na_forward, websGetVar(wp, T("CFNoAns_No"), T("")));
		if (voip_port == 0) // if FXS0
		{
			pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward_enable = atoi(websGetVar(wp, T("PSTN_CFNoAns"), T("")));
			strcpy(pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward, websGetVar(wp, T("PSTN_CFNoAns_No"), T("")));
			pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )].na_forward_time = atoi(websGetVar(wp, T("PSTN_CFNoAns_Time"), T("")));
		}
	}
#else
	//pCfg->na_forward_enable = !gstrcmp(websGetVar(wp, T("CFNoAns"), T("")), "on");
	pCfg ->na_forward_enable = atoi( websGetVar(wp, T("CFNoAns"), T("")) );
	pCfg->na_forward_time	  	= atoi(websGetVar(wp, T("CFNoAns_Time"), T("")));
	strcpy(pCfg->na_forward, websGetVar(wp, T("CFNoAns_No"), T("")));
#endif

	/* Speed Dial */
	for (i=0; i<MAX_SPEED_DIAL; i++)
	{
		char szBuf[20];

		sprintf(szBuf, "spd_name%d", i);
		strcpy(pCfg->speed_dial[i].name, websGetVar(wp, szBuf, T("")));
		sprintf(szBuf, "spd_url%d", i);
		strcpy(pCfg->speed_dial[i].url, websGetVar(wp, szBuf, T("")));
	}

#ifdef CONFIG_RTK_VOIP_DIALPLAN
	/* Dial Plan */
	strcpy((char *) pCfg->dialplan, websGetVar(wp, T("dialplan"), T("")));
	pCfg ->replace_rule_option = atoi( websGetVar(wp, T("ReplaceRuleOption"), T("")) );
	strcpy((char *) pCfg->replace_rule_source, websGetVar(wp, T("ReplaceRuleSource"), T("")));
	strcpy((char *) pCfg->replace_rule_target, websGetVar(wp, T("ReplaceRuleTarget"), T("")));
	strcpy((char *) pCfg->auto_prefix, websGetVar(wp, T("AutoPrefix"), T("")));
	strcpy((char *) pCfg->prefix_unset_plan, websGetVar(wp, T("PrefixUnsetPlan"), T("")));
#endif

	/* PSTN Routing Prefix */
	strcpy((char *) pCfg->PSTN_routing_prefix, websGetVar(wp, T("PSTNRoutingPrefix"), T("")));

	/* DSP */
	for(i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		sprintf(szFrameSize, T("frameSize%d"), i);
		pCfg->frame_size[i] = atoi(websGetVar(wp, szFrameSize, T("0")));
		sprintf(szPrecedence, T("preced%d"), i);
		pCfg->precedence[i] = atoi(websGetVar(wp, szPrecedence, T("-1")));
	}

	/*
	 * Codec Options
	 */
#ifdef CONFIG_RTK_VOIP_G7231
	pCfg->g7231_rate = (gstrcmp(websGetVar(wp, T("g7231Rate"), T("")), "5.3k")) ? G7231_RATE63 :  G7231_RATE53;
#endif
#ifdef CONFIG_RTK_VOIP_G726
	ptr = websGetVar(wp, T("g726_packing"), T(""));
	for(i=0; i<G726_PACK_MAX; i++)
	{
		if (!gstrcmp(ptr, g726pack[i]))
			break;
	}
	if (i == G726_PACK_MAX)
		i = G726_PACK_RIGHT;

	pCfg->g726_packing = i;
#endif
#ifdef CONFIG_RTK_VOIP_ILBC
	pCfg->iLBC_mode = (gstrcmp(websGetVar(wp, T("iLBC_mode"), T("")), "30ms") == 0) ? ILBC_30MS : ILBC_20MS ;
#endif
#ifdef CONFIG_RTK_VOIP_SPEEX_NB
	pCfg->speex_nb_rate = atoi(websGetVar(wp, T("speex_nb_rate"), T("2")));
#endif
#ifdef CONFIG_RTK_VOIP_G7111
	memset(pCfg->g7111_precedence, 0, sizeof(pCfg->g7111_precedence));
	for (i=0; i<G7111_MODES; i++)
	{
		int pri;

		pri = atoi(websGetVar(wp, g7111_modes[i], T("-1")));
		if (pri >=0 && pri < G7111_MODES)
			pCfg->g7111_precedence[pri] = i + 1;
	}
#endif

	/*
	 * RTP Redundant
	 */
#ifdef SUPPORT_RTP_REDUNDANT
	pCfg->rtp_redundant_codec = atoi( websGetVar(wp, T("rtp_redundant_codec"), T("-1")));
	pCfg->rtp_redundant_payload_type = atoi( websGetVar(wp, T("rtp_redundant_payload_type"), T("121")));
#endif

	pCfg->lec = !gstrcmp(websGetVar(wp, T("useLec"), T("")), "on");
	pCfg->nlp = !gstrcmp(websGetVar(wp, T("useNlp"), T("")), "on");
	pCfg->vad = !gstrcmp(websGetVar(wp, T("useVad"), T("")), "on");
	pCfg->vad_thr = atoi(websGetVar(wp, T("Vad_threshold"), T("1")));
	pCfg->cng = !gstrcmp(websGetVar(wp, T("useCng"), T("")), "on");
	pCfg->cng_thr = atoi(websGetVar(wp, T("Cng_threshold"), T("1")));
	pCfg->PLC = !gstrcmp(websGetVar(wp, T("usePLC"), T("")), "on");

	pCfg->sid_gainmode = atoi(websGetVar(wp, T("sid_mode"), T("0")));
	if (pCfg->sid_gainmode == 1)
		pCfg->sid_noiselevel = atoi(websGetVar(wp, T("sid_noiselevel"), T("70")));
	else if (pCfg->sid_gainmode == 2)
		pCfg->sid_noisegain = atoi(websGetVar(wp, T("sid_noisegain"), T("0")));

	i=0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_CNG_TDM"), T("")), "on"))?0x1:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_CNG_IP"), T("")), "on"))?0x100:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANS_TDM"), T("")), "on"))?0x2:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANS_IP"), T("")), "on"))?0x200:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANSAM_TDM"), T("")), "on"))?0x4:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANSAM_IP"), T("")), "on"))?0x400:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANSBAR_TDM"), T("")), "on"))?0x8:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANSBAR_IP"), T("")), "on"))?0x800:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANSAMBAR_TDM"), T("")), "on"))?0x10:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_ANSAMBAR_IP"), T("")), "on"))?0x1000:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_BELLANS_TDM"), T("")), "on"))?0x20:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_BELLANS_IP"), T("")), "on"))?0x2000:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_V22ANS_TDM"), T("")), "on"))?0x40:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_V22ANS_IP"), T("")), "on"))?0x4000:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_V8bis_Cre_TDM"), T("")), "on"))?0x80:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_V8bis_Cre_IP"), T("")), "on"))?0x8000:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_V21flag_TDM"), T("")), "on"))?0x10000:0;
	i|= (!gstrcmp(websGetVar(wp, T("useANSTONE_V21flag_IP"), T("")), "on"))?0x20000:0;
	pCfg->anstone=i;
	if( !gstrcmp(websGetVar(wp, T("useRTCP"), T("")), "on") ) {
		pCfg->RTCP_Interval = atoi( websGetVar(wp, T("RTCPInterval"), T("0")) );
		pCfg->RTCP_XR = !gstrcmp(websGetVar(wp, T("useRTCPXR"), T("")), "on");	
	} else {
		pCfg->RTCP_Interval = 0;
		pCfg->RTCP_XR = 0;
	}

	i=0;
	i|= (!gstrcmp(websGetVar(wp, T("useFaxModem2833Relay"), T("")), "on"))?0x1:0;
	i|= (!gstrcmp(websGetVar(wp, T("useFaxModemInbandRemoval"), T("")), "on"))?0x2:0;
	i|= (!gstrcmp(websGetVar(wp, T("useFaxModem2833RxTonePlay"), T("")), "on"))?0x4:0;
	pCfg->faxmodem_rfc2833=i;

	pCfg->speaker_agc = !gstrcmp(websGetVar(wp, T("CFuseSpeaker"), T("")), "on");
	pCfg->spk_agc_lvl = atoi(websGetVar(wp, T("CF_spk_AGC_level"), T("1"))) - 1;
	pCfg->spk_agc_gu = atoi(websGetVar(wp, T("CF_spk_AGC_up_limit"), T("6"))) - 1;
	pCfg->spk_agc_gd = (-(atoi(websGetVar(wp, T("CF_spk_AGC_down_limit"), T("-6"))))) - 1 ;
	pCfg->mic_agc = !gstrcmp(websGetVar(wp, T("CFuseMIC"), T("")), "on");
	pCfg->mic_agc_lvl = atoi(websGetVar(wp, T("CF_mic_AGC_level"), T("1"))) - 1;
	pCfg->mic_agc_gu = atoi(websGetVar(wp, T("CF_mic_AGC_up_limit"), T("6"))) - 1;
	pCfg->mic_agc_gd = (-(atoi(websGetVar(wp, T("CF_mic_AGC_down_limit"), T("-6"))))) - 1 ;
	pCfg->slic_txVolumne = atoi(websGetVar(wp, T("slic_txVolumne"), T("1"))) - 1;
	pCfg->slic_rxVolumne = atoi(websGetVar(wp, T("slic_rxVolumne"), T("1"))) - 1;
	pCfg->maxDelay  = atoi(websGetVar(wp, T("maxDelay"), T("13")));
	pCfg->echoTail  = atoi(websGetVar(wp, T("echoTail"), T("2")));
	pCfg->jitter_delay = atoi(websGetVar(wp, T("jitterDelay"), T("4")));
	pCfg->jitter_factor = atoi(websGetVar(wp, T("jitterFactor"), T("7")));

	ptr	 = websGetVar(wp, T("caller_id"), T(""));
	for(i=0; i<CID_MAX; i++)
	{
		if (!gstrcmp(ptr, cid[i]))
			break;
	}
	if (i == CID_MAX)
		i = CID_DTMF;

	if(!gstrcmp(websGetVar(wp, T("FSKdatesync"), T("")), "on"))
		i=i | 0x80U;

	if(!gstrcmp(websGetVar(wp, T("revPolarity"), T("")), "on"))
		i=i | 0x40U;

	if(!gstrcmp(websGetVar(wp, T("sRing"), T("")), "on"))
		i=i | 0x20U;

	if(!gstrcmp(websGetVar(wp, T("dualTone"), T("")), "on"))
		i=i | 0x10U;

	if(!gstrcmp(websGetVar(wp, T("PriorRing"), T("")), "on"))
		i=i | 0x08U;

	pCfg->caller_id_mode = i;

	ptr	 = websGetVar(wp, T("cid_dtmfMode_S"), T(""));
	for(i=0; i<CID_DTMF_MAX; i++)
	{
		if (!gstrcmp(ptr, cid_dtmf[i]))
			break;
	}
	if (i == CID_DTMF_MAX)
		i = CID_DTMF_D;
	pCfg->cid_dtmf_mode = i;

	ptr	 = websGetVar(wp, T("cid_dtmfMode_E"), T(""));
	for(i=0; i<CID_DTMF_MAX; i++)
	{
		if (!gstrcmp(ptr, cid_dtmf[i]))
			break;
	}
	if (i == CID_DTMF_MAX)
		i = CID_DTMF_D;
	pCfg->cid_dtmf_mode |= (i<<2);

	pCfg->cid_fsk_gen_mode = !gstrcmp(websGetVar(wp, T("SoftFskGen"), T("")), "on");

#ifndef CONFIG_RTK_VOIP_IP_PHONE
        i = atoi(websGetVar(wp, T("flash_hook_time"), T("")));
	if ((i >= 100) && ( i <= 2000))
		pCfg->flash_hook_time = i;
	else
		pCfg->flash_hook_time = 300;
#endif

#ifndef CONFIG_RTK_VOIP_IP_PHONE
        i = atoi(websGetVar(wp, T("flash_hook_time_min"), T("")));
	if ( i >= pCfg->flash_hook_time )
		pCfg->flash_hook_time_min = 0;
	else
		pCfg->flash_hook_time_min = i;
#endif

	i = atoi(websGetVar(wp, T("spk_voice_gain"), T("")));
	if ((i>= -32) && (i<=31) )
		pCfg->spk_voice_gain=i;
	else
		pCfg->spk_voice_gain=0;

	i = atoi(websGetVar(wp, T("mic_voice_gain"), T("")));
	if ((i>= -32) && (i<=31) )
		pCfg->mic_voice_gain=i;
	else
		pCfg->mic_voice_gain=0;

#ifdef SUPPORT_VOICE_QOS
	ptr	 = websGetVar(wp, T("voice_qos"), T(""));
	for(i=0; i<DSCP_MAX; i++)
	{
		if (!gstrcmp(ptr, dscp[i]))
			break;
	}
	if (i == DSCP_MAX)
		i = DSCP_CS0;

	pCfg->voice_qos	= i;
#endif

/*++T.38 added by Jack Chan 24/01/07 for VoIP++*/
#ifdef CONFIG_RTK_VOIP_T38
	pCfg->useT38 = !gstrcmp(websGetVar(wp, T("useT38"), T("")), "on");
	pCfg->T38_port = atoi(websGetVar(wp, T("T38_PORT"), T("49172")));

	//T.38 parameters
	pCfg->T38ParamEnable = !gstrcmp(websGetVar(wp, T("T38ParamEnable"), T("")), "on");
	pCfg->T38MaxBuffer = atoi(websGetVar(wp, T("T38MaxBuffer"), T("500")));
	pCfg->T38RateMgt = atoi(websGetVar(wp, T("T38RateMgt"), T("2")));
	pCfg->T38MaxRate = atoi(websGetVar(wp, T("T38MaxRate"), T("5")));
	pCfg->T38EnableECM = !gstrcmp(websGetVar(wp, T("T38EnableECM"), T("")), "on");
	pCfg->T38ECCSignal = atoi(websGetVar(wp, T("T38ECCSignal"), T("5")));
	pCfg->T38ECCData = atoi(websGetVar(wp, T("T38ECCData"), T("2")));
	pCfg->T38EnableSpoof = !gstrcmp(websGetVar(wp, T("T38EnableSpoof"), T("")), "on");
	pCfg->T38DuplicateNum = atoi(websGetVar(wp, T("T38DuplicateNum"), T("0")));
#else /*CONFIG_RTK_VOIP_T38*/
	pCfg->useT38 = 0;
	pCfg->T38_port = 0;

	//T.38 parameters
	pCfg->T38ParamEnable = 0;
	pCfg->T38MaxBuffer = 0;
	pCfg->T38RateMgt = 0;
	pCfg->T38MaxRate = 0;
	pCfg->T38EnableECM = 0;
	pCfg->T38ECCSignal = 0;
	pCfg->T38ECCData = 0;
	pCfg->T38EnableSpoof = 0;
	pCfg->T38DuplicateNum = 0;
#endif /*#ifdef CONFIG_RTK_VOIP_T38*/
/*--end--*/

	ptr	 = websGetVar(wp, T("fax_modem_det_mode"), T(""));
	for(i=0; i<FAX_MODEM_DET_MAX; i++)
	{
		if (!gstrcmp(ptr, fax_modem_det_string[i]))
			break;
	}
	if (i == FAX_MODEM_DET_MAX)
		i = FAX_MODEM_DET_MODEM;
	pCfg->fax_modem_det = i;

	// V.152
	pCfg->useV152 = !gstrcmp(websGetVar(wp, T("useV152"), T("")), "on");
	pCfg->v152_payload_type = atoi(websGetVar(wp, T("V152_payload_type"), T("96")));
	pCfg->v152_codec_type = atoi(websGetVar(wp, T("V152_codec_type"), T("0")));

#ifdef CONFIG_RTK_VOIP_SRTP
	pCfg->security_enable = !gstrcmp(websGetVar(wp, T("useSRTP"), T("")), "on");
#else
	pCfg->security_enable = 0;
#endif /*CONFIG_RTK_VOIP_SRTP*/
	// Hot line
	pCfg->hotline_enable = strcmp(websGetVar(wp, T("hotline_enable"), T("")), "on") == 0;
	if (pCfg->hotline_enable)
	{
		strcpy(pCfg->hotline_number, websGetVar(wp, T("hotline_number"), T("")));
	}

	// DND
	pCfg->dnd_mode = atoi(websGetVar(wp, T("dnd_mode"), T("0")));
	if (pCfg->dnd_mode == 1)
	{
		pCfg->dnd_from_hour = atoi(websGetVar(wp, T("dnd_from_hour"), T("0")));
		pCfg->dnd_from_min = atoi(websGetVar(wp, T("dnd_from_min"), T("0")));
		pCfg->dnd_to_hour = atoi(websGetVar(wp, T("dnd_to_hour"), T("0")));
		pCfg->dnd_to_min = atoi(websGetVar(wp, T("dnd_to_min"), T("0")));
	}

	// auth
	strcpy(pCfg->offhook_passwd, websGetVar(wp, T("offhook_passwd"), T("")));

	// abbreviated dial
	for( i = 0; i < MAX_ABBR_DIAL_NUM; i ++ ) {
		// name
		sprintf( szName, "abbr_name%d", i );
		ptr = websGetVar(wp, szName, T(""));
		if( strlen( ptr ) < MAX_ABBR_DIAL_NAME )
			strcpy( pCfg ->abbr_dial[ i ].name, ptr );
		else
			pCfg ->abbr_dial[ i ].name[ 0 ] = '\x0';

		// url
		sprintf( szName, "abbr_url%d", i );
		ptr = websGetVar(wp, szName, T(""));
		if( strlen( ptr ) < MAX_ABBR_DIAL_URL )
			strcpy( pCfg ->abbr_dial[ i ].url, ptr );
		else
			pCfg ->abbr_dial[ i ].url[ 0 ] = '\x0';
	}

	// alarm
	pCfg ->alarm_enable = strcmp(websGetVar(wp, T("alarm_enable"), T("")), "on") == 0;
	if( pCfg ->alarm_enable ) {
		pCfg ->alarm_time_hh = atoi(websGetVar(wp, T("alarm_hh"), T("0")));
		pCfg ->alarm_time_mm = atoi(websGetVar(wp, T("alarm_mm"), T("0")));
#if 0
		pCfg ->alarm_ring_last_day = 0;
		pCfg ->alarm_ring_defer = 0;
#endif
	} else {
		pCfg ->alarm_time_hh = 0;
		pCfg ->alarm_time_mm = 0;
#if 0
		pCfg ->alarm_ring_last_day = 0;
		pCfg ->alarm_ring_defer = 0;
#endif
	}

	web_flash_set(pVoIPCfg);


	sprintf(redirect_url, "/voip_general.asp?port=%d", voip_port);
#ifdef REBOOT_CHECK
	OK_MSG(redirect_url);
#else
	web_restart_solar();
	websRedirect(wp, redirect_url);
#endif
}

#ifdef CONFIG_APP_BOA
int asp_voip_DialPlanGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_DialPlanGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	return 0;
}

void asp_voip_DialPlanSet(webs_t wp, char_t *path, char_t *query)
{
#ifdef REBOOT_CHECK
	OK_MSG("/dialplan.asp");
#else
	websRedirect(wp, T("/dialplan.asp"));
#endif
}
