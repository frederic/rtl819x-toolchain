
/*
 *      Header file of AP mib
 *      Authors: 
 *
 */

#define NOREP
#define _VOIP_MIB_ID_NAME(name) MIB_VOIP_##name, #name

#ifdef MIB_VOIP_TONE_IMPORT
//voipMibEntry_T mibtbl_cust_tone[] = {
MIBDEF(unsigned long,   toneType,          	NOREP, 		CUST_TONE_TYPE,     	V_DWORD, 	st_ToneCfgParam, 0, 0)
MIBDEF(unsigned short,  cycle,  			NOREP, 		CUST_TONE_CYCLE,		V_WORD,   	st_ToneCfgParam, 0, 0)
MIBDEF(unsigned short,  cadNUM,  			NOREP, 		CUST_TONE_CAD_NUM,		V_WORD,   	st_ToneCfgParam, 0, 0)

MIBDEF(unsigned long,   CadOn0,          	NOREP, CUST_TONE_CAD_ON0,           V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   CadOn1,          	NOREP, CUST_TONE_CAD_ON1,           V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   CadOn2,          	NOREP, CUST_TONE_CAD_ON2,           V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   CadOn3,          	NOREP, CUST_TONE_CAD_ON3,          	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   CadOff0,          	NOREP, CUST_TONE_CAD_OFF0,          V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   CadOff1,          	NOREP, CUST_TONE_CAD_OFF1,          V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   CadOff2,          	NOREP, CUST_TONE_CAD_OFF2,          V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   CadOff3,          	NOREP, CUST_TONE_CAD_OFF3,          V_DWORD,    st_ToneCfgParam, 0, 0)

MIBDEF(unsigned long,   PatternOff,         NOREP, CUST_TONE_PATTERN_OFF,    	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   ToneNUM,          	NOREP, CUST_TONE_NUM,           	V_DWORD,    st_ToneCfgParam, 0, 0)

MIBDEF(unsigned long,   Freq1,          	NOREP, CUST_TONE_FREQ1,           	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   Freq2,          	NOREP, CUST_TONE_FREQ2,           	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   Freq3,          	NOREP, CUST_TONE_FREQ3,           	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(unsigned long,   Freq4,          	NOREP, CUST_TONE_FREQ4,           	V_DWORD,    st_ToneCfgParam, 0, 0)

MIBDEF(long,   			Gain1,          	NOREP, CUST_TONE_GAIN1,           	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(long,   			Gain2,          	NOREP, CUST_TONE_GAIN2,           	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(long,   			Gain3,          	NOREP, CUST_TONE_GAIN3,           	V_DWORD,    st_ToneCfgParam, 0, 0)
MIBDEF(long,   			Gain4,          	NOREP, CUST_TONE_GAIN4,           	V_DWORD,    st_ToneCfgParam, 0, 0)
#endif /* MIB_VOIP_TONE_IMPORT */


#ifdef MIB_VOIP_SPEED_DIAL_IMPORT
/*      _ctype,         _cname,             _crepeat,   _mib_name,          _mib_type,  _mib_parents_ctype, 
        _default_value, _next_tbl */

MIBSTR(char,            name,       			[MAX_SPEED_DIAL_NAME],         
                        SPEED_DIAL_NAME,   										V_STRING,	SpeedDialCfg_t, 0, 0)
MIBSTR(char,            url,       				[MAX_SPEED_DIAL_URL],         
                        SPEED_DIAL_URL,         								V_STRING,	SpeedDialCfg_t, 0, 0)
#endif

#ifdef MIB_VOIP_PROXY_IMPORT
/*      _ctype,         _cname,             _crepeat,   _mib_name,          _mib_type,  _mib_parents_ctype, 
        _default_value, _next_tbl */

/* account */
MIBSTR(char,            display_name,			[DNS_LEN],	PROXY_DISPLAY_NAME,	V_STRING,	voipCfgProxy_t, 0, 0)
MIBSTR(char,            number,    				[DNS_LEN],  PROXY_NUMBER, 		V_STRING,	voipCfgProxy_t, 0, 0)
MIBSTR(char,            login_id,       		[DNS_LEN],  PROXY_LOGIN_ID,		V_STRING,	voipCfgProxy_t, 0, 0)
MIBSTR(char,            password,    			[DNS_LEN],  PROXY_PASSWORD,		V_STRING, 	voipCfgProxy_t, 0, 0)

/* register server */
MIBDEF(unsigned short,  enable,  				NOREP, 		PROXY_ENABLE,		V_WORD,   	voipCfgProxy_t, 0, 0)
MIBSTR(char,            addr,      				[DNS_LEN],  PROXY_ADDR,    		V_STRING, 	voipCfgProxy_t, 0, 0)
MIBDEF(unsigned short,  port,  					NOREP, 		PROXY_PORT,			V_WORD,   	voipCfgProxy_t, 0, 0)
MIBSTR(char,            domain_name,     		[DNS_LEN],  PROXY_DOMAIN_NAME,	V_STRING,	voipCfgProxy_t, 0, 0)
MIBDEF(unsigned int,    reg_expire,       		NOREP, 		PROXY_REG_EXPIRE,   V_UINT,     voipCfgProxy_t, 0, 0)

/* nat traversal server */
MIBDEF(unsigned short,  outbound_enable,  		NOREP, 		PROXY_OUTBOUND_ENABLE,V_WORD, 	voipCfgProxy_t, 0, 0)
MIBSTR(char,            outbound_addr,    		[DNS_LEN],  PROXY_OUTBOUND_ADDR,V_STRING,  	voipCfgProxy_t, 0, 0)
MIBDEF(unsigned short,  outbound_port,  		NOREP, 		PROXY_OUTBOUND_PORT,V_WORD,   	voipCfgProxy_t, 0, 0)

/* sip tls for security */
MIBDEF(unsigned short,  siptls_enable,  		NOREP, 		TLS_ENABLE,			V_WORD,   	voipCfgProxy_t, 0, 0)
#endif /* MIB_VOIP_PROXY_IMPORT */

#ifdef MIB_VOIP_ABBR_DIAL_IMPORT
/*      _ctype,         _cname,             _crepeat,   _mib_name,          _mib_type,  _mib_parents_ctype, 
        _default_value, _next_tbl */

MIBSTR(char,            name,       					[MAX_ABBR_DIAL_NAME],         
                        ABBREVIATED_DIAL_NAME,       							V_STRING,  	abbrDialCfg_t, 0, 0)
MIBSTR(char,            url,       						[MAX_ABBR_DIAL_URL],         
                        ABBREVIATED_DIAL_URL,             						V_STRING,	abbrDialCfg_t, 0, 0)
#endif /* MIB_VOIP_ABBR_DIAL_IMPORT */

#ifdef MIB_VOIP_PORT_IMPORT
/*      _ctype,         _cname,             _crepeat,   _mib_name,          _mib_type,  _mib_parents_ctype, 
        _default_value, _next_tbl */

// general
MIBLST(voipCfgProxy_t,  proxies,            			[MAX_PROXY],        
                        PROXIES,            V_MIB_LIST, voipCfgPortParam_t,             0,  mibtbl_proxies)

MIBDEF(unsigned char,   default_proxy,      NOREP, 		DEFAULT_PROXY,  	V_BYTE,		voipCfgPortParam_t, 0, 0)

// stun/NAT
MIBDEF(unsigned char,   stun_enable,        NOREP, 		STUN_ENABLE,       	V_BYTE,     voipCfgPortParam_t, 0, 0)
MIBSTR(char,            stun_addr,          			[DNS_LEN],          
                       	STUN_ADDR,          								V_STRING,	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned short,  stun_port,          NOREP, 		STUN_PORT,          V_WORD,     voipCfgPortParam_t, 0, 0)

// advanced
MIBDEF(unsigned short,  sip_port,       	NOREP, 		SIP_PORT,           V_WORD,     voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned short,  media_port,       	NOREP, 		MEDIA_PORT,         V_WORD,     voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   dtmf_mode,       	NOREP, 		DTMF_MODE,          V_BYTE,     voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned short,  dtmf_2833_pt,       			NOREP, 
						DTMF_RFC2833_PAYLOAD_TYPE,            				V_WORD,     voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned short,  dtmf_2833_pi,       			NOREP, 		
						FAXMODEM_RFC2833_PT,								V_WORD,     voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned char,   fax_modem_2833_pt_same_dtmf,   	NOREP, 
						DTMF_RFC2833_PI,            						V_BYTE,     voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned short,  fax_modem_2833_pt,  			NOREP, 		
						FAXMODEM_RFC2833_PI,								V_WORD,   	voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned short,  fax_modem_2833_pi,  			NOREP, 		
						FAXMODEM_RFC2833_PT_SAME_DTMF,						V_WORD,     voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned short,  sip_info_duration,  NOREP, 		SIP_INFO_DURATION,  V_WORD,     voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   call_waiting_enable,NOREP, 		CALL_WAITING_ENABLE,V_BYTE,     voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   direct_ip_call,     NOREP, 		DIRECT_IP_CALL,     V_BYTE,    	voipCfgPortParam_t, 0, 0)

// forward
MIBDEF(unsigned char,   uc_forward_enable,  NOREP, 		UC_FORWARD_ENABLE,   V_BYTE,    voipCfgPortParam_t, 0, 0)
MIBSTR(char,            uc_forward,       				[FW_LEN],       
                        UC_FORWARD,               						   V_STRING,   	voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned char,   busy_forward_enable,NOREP, 		BUSY_FORWARD_ENABLE, V_BYTE,    voipCfgPortParam_t, 0, 0)
MIBSTR(char,            busy_forward,       			[FW_LEN],         
                        BUSY_FORWARD,               					   V_STRING,   	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   na_forward_enable,  NOREP, 		NA_FORWARD_ENABLE,   V_BYTE,    voipCfgPortParam_t, 0, 0)
MIBSTR(char,            na_forward,       				[FW_LEN],         
                        NA_FORWARD,               						   V_STRING, 	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned short,  na_forward_time,    NOREP, 		NA_FORWARD_TIME,     V_WORD,    voipCfgPortParam_t, 0, 0)

/* speed dial */
MIBLST(SpeedDialCfg_t,  speed_dial,            			[MAX_SPEED_DIAL],        
                        SPEED_DIAL,     	V_MIB_LIST, voipCfgPortParam_t,             0, mibtbl_speed_dial)

/* dial plan */
MIBDEF(unsigned char,   replace_rule_option,NOREP, 		REPLACE_RULE_OPTION,V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBSTR(unsigned char,   replace_rule_source,  			[MAX_REPLACE_RULE_SOURCE], 		
						REPLACE_RULE_SOURCE,   							  V_STRING,  	voipCfgPortParam_t, 0, 0)
MIBSTR(unsigned char,   replace_rule_target,  			[MAX_REPLACE_RULE_TARGET], 		
						REPLACE_RULE_TARGET,   							  V_STRING,  	voipCfgPortParam_t, 0, 0)
MIBSTR(unsigned char,   dialplan,  						[MAX_DIALPLAN_LENGTH], 		
						DIAL_PLAN,   									  V_STRING,    	voipCfgPortParam_t, 0, 0)
MIBSTR(unsigned char,   auto_prefix,  					[MAX_AUTO_PREFIX], 		
						AUTO_PREFIX,   									  V_STRING,    	voipCfgPortParam_t, 0, 0)
MIBSTR(unsigned char,   prefix_unset_plan,  			[MAX_PREFIX_UNSET_PLAN], 		
						PREFIX_UNSET_PLAN,   							  V_STRING,    	voipCfgPortParam_t, 0, 0)

/* codec */
MIBARY(unsigned char,   frame_size,  [_CODEC_MAX],		FRAME_SIZE,    V_BYTE_LIST,    	voipCfgPortParam_t, 0, 0)
MIBARY(unsigned char,   precedence,  [_CODEC_MAX], 		PRECEDENCE,    V_BYTE_LIST,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   vad,  				NOREP,		VAD,   				V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   vad_thr,  			NOREP, 		VAD_THRESHOLD,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   cng,  				NOREP, 		CNG,   				V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   cng_thr,  			NOREP, 		CNG_THRESHOLD,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   sid_gainmode,  		NOREP, 		SID_MODE,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   sid_noiselevel, 	NOREP, 		SID_LEVEL,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(signed char,   	sid_noisegain, 		NOREP, 		SID_GAIN,   	   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   PLC,  				NOREP, 		PLC,   				V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   RTCP_Interval,  	NOREP, 		RTCP_INTERVAL,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   RTCP_XR,  			NOREP, 		RTCPXR,   			V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   g7231_rate,  		NOREP, 		G7231_RATE,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   iLBC_mode,  		NOREP, 		ILBC_MODE,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   speex_nb_rate,  	NOREP, 		SPEEX_NB_RATE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   g726_packing,  		NOREP, 		G726_PACK,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBARY(unsigned char,   g7111_precedence,  				[G7111_MODES], 		
						G7111_PRECEDENCE,   						   V_BYTE_LIST,    	voipCfgPortParam_t, 0, 0)

/* RTP redundant */
MIBDEF(unsigned char,   rtp_redundant_payload_type,  	NOREP, 		
						RTP_RED_PAYLOAD_TYPE,   							V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(signed char,   	rtp_redundant_codec, 			NOREP, 		
						RTP_RED_CODEC,   	   								V_BYTE,    	voipCfgPortParam_t, 0, 0)

/* DSP */
MIBDEF(unsigned char,   slic_txVolumne,  	NOREP, 		SLIC_TX_VOLUME,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   slic_rxVolumne,  	NOREP, 		SLIC_RX_VOLUME,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   jitter_delay,  		NOREP, 		JITTER_DELAY,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   maxDelay,  			NOREP, 		MAX_DELAY,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   jitter_factor,  	NOREP, 		JITTER_FACTOR,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned char,   lec,  				NOREP, 		ECHO_LEC, 			V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   nlp,  				NOREP, 		ECHO_NLP,  			V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   echoTail,  			NOREP, 		ECHO_TAIL,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   caller_id_mode,  	NOREP, 		CALLER_ID_MODE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   call_waiting_cid,  	NOREP, 		CALL_WAITING_CID,   V_BYTE,    	voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned char,   cid_dtmf_mode,  	NOREP, 		CID_DTMF_MODE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   speaker_agc,  		NOREP, 		SPEAKERAGC,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   spk_agc_lvl,  		NOREP, 		SPK_AGC_LVL,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   spk_agc_gu,  		NOREP, 		SPK_AGC_GU,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   spk_agc_gd,  		NOREP, 		SPK_AGC_GD,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned char,   mic_agc,  			NOREP, 		MICAGC,   			V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   mic_agc_lvl,  		NOREP, 		MIC_AGC_LVL,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   mic_agc_gu,  		NOREP, 		MIC_AGC_GU,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   mic_agc_gd,  		NOREP, 		MIC_AGC_GD,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   cid_fsk_gen_mode,  	NOREP, 		FSK_GEN_MODE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)

MIBDEF(signed char,   	spk_voice_gain,		NOREP, 		SPK_VOICE_GAIN,	   	V_BYTE,    voipCfgPortParam_t, 0, 0)
MIBDEF(signed char,   	mic_voice_gain,		NOREP, 		MIC_VOICE_GAIN,    	V_BYTE,    voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned int,   	anstone,      	 	NOREP, 		ANSTONE,            V_UINT,     voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   faxmodem_rfc2833,  	NOREP, 		FAX_MODEM_RFC2833,  V_BYTE,    	voipCfgPortParam_t, 0, 0)

//QoS
MIBDEF(unsigned char,   voice_qos,  		NOREP, 		VOICE_QOS,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)

//T.38
MIBDEF(unsigned char,   useT38,  			NOREP, 		T38_USET38,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned short,  T38_port,  			NOREP, 		T38_PORT,			V_WORD,   	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   fax_modem_det,  	NOREP, 		FAX_MODEM_DET,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)

MIBDEF(unsigned char,   T38ParamEnable,  	NOREP, 		T38_PARAM_ENABLE,   V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned short,  T38MaxBuffer,  		NOREP, 		T38_MAX_BUFFER,		V_WORD,   	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   T38RateMgt,  		NOREP, 		T38_RATE_MGT,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   T38MaxRate,  		NOREP, 		T38_MAX_RATE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   T38EnableECM,  		NOREP, 		T38_ENABLE_ECM,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   T38ECCSignal,  		NOREP, 		T38_ECC_SIGNAL,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   T38ECCData,  		NOREP, 		T38_ECC_DATA,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   T38EnableSpoof,  	NOREP, 		T38_ENABLE_SPOOF,   V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   T38DuplicateNum,  	NOREP, 		T38_DUPLICATE_NUM,  V_BYTE,    	voipCfgPortParam_t, 0, 0)

/* V.152 */
MIBDEF(unsigned char,   useV152,  			NOREP, 		USE_V152,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   v152_payload_type,  NOREP, 		V152_PAYLOAD_TYPE, 	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   v152_codec_type,  	NOREP, 		V152_CODEC_TYPE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)

/* Hot Line */
MIBDEF(unsigned char,   hotline_enable,  	NOREP, 		HOTLINE_ENABLE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBSTR(char,            hotline_number,	[DNS_LEN],      HOTLINE_NUMBER,    	V_STRING,	voipCfgPortParam_t, 0, 0)

/* DND */
MIBDEF(unsigned char,   dnd_mode,  			NOREP, 		DND_MODE,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   dnd_from_hour,  	NOREP, 		DND_FROM_HOUR,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   dnd_from_min,  		NOREP, 		DND_FROM_MIN,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   dnd_to_hour,  		NOREP, 		DND_TO_HOUR,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   dnd_to_min,  		NOREP, 		DND_TO_MIN,   		V_BYTE,    	voipCfgPortParam_t, 0, 0)

/* flash hook time */
MIBDEF(unsigned short,  flash_hook_time,  	NOREP, 		FLASH_HOOK_TIME,	V_WORD,   	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned short,  flash_hook_time_min,NOREP, 		FLASH_HOOK_TIME_MIN,V_WORD,   	voipCfgPortParam_t, 0, 0)

/* Security */
MIBDEF(unsigned char,   security_enable,  	NOREP, 		SECURITY_ENABLE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   key_exchange_mode,  NOREP, 		
						KEY_EXCHANGE_MODE,   								V_BYTE,    	voipCfgPortParam_t, 0, 0)

/* Auth */
MIBSTR(char,            offhook_passwd, 				[PASSWD_LEN],         
                        OFFHOOK_PASSWD,               					  V_STRING,  	voipCfgPortParam_t, 0, 0)


/* abbreviated dial (phonebook) */
MIBLST(abbrDialCfg_t,  	abbr_dial,            			[MAX_ABBR_DIAL_NUM],        
                   	   	ABBREVIATED_DIAL,V_MIB_LIST, 	voipCfgPortParam_t,      0,		mibtbl_abbreviated_dial )

MIBDEF(unsigned char,   alarm_enable,  		NOREP, 		ALARM_ENABLE,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   alarm_time_hh,  	NOREP, 		ALARM_TIME_HH,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)
MIBDEF(unsigned char,   alarm_time_mm,  	NOREP, 		ALARM_TIME_MM,   	V_BYTE,    	voipCfgPortParam_t, 0, 0)

MIBSTR(unsigned char,   PSTN_routing_prefix,   			[PSTN_ROUTING_PREFIX_LEN],         
                        PSTN_ROUTING_PREFIX,               					V_STRING, 	voipCfgPortParam_t, 0, 0)

#endif

#ifdef MIB_VOIP_CFG_IMPORT
/* _ctype,  _cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */

// voip flash check
MIBDEF(unsigned long, 	signature,			NOREP, 		SIGNATURE,  		V_DWORD,	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	version, 			NOREP, 		VERSION,  			V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned long, 	feature,			NOREP, 		FEATURE,  			V_DWORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned long, 	extend_feature,		NOREP, 		EXTEND_FEATURE, 	V_DWORD,	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short, 	mib_version,		NOREP, 		MIB_VERSION, 		V_WORD,		voipCfgParam_t, 0, 0)

// RFC flags
MIBDEF(unsigned int, 	rfc_flags,			NOREP, 		RFC_FLAGS, 			V_UINT, 	voipCfgParam_t, 0, 0)

// tone
MIBDEF(unsigned char, 	tone_of_country,	NOREP, 		TONE_OF_COUNTRY,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	tone_of_custdial,	NOREP, 		TONE_OF_CUSTDIAL,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	tone_of_custring,	NOREP, 		TONE_OF_CUSTRING,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	tone_of_custbusy,	NOREP, 		TONE_OF_CUSTBUSY,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	tone_of_custwaiting,NOREP, 		TONE_OF_CUSTWAITING,V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	tone_of_customize,	NOREP, 		TONE_OF_CUSTOMIZE,	V_BYTE,		voipCfgParam_t, 0, 0)

// customize tone
MIBLST(st_ToneCfgParam, cust_tone_para,		[TONE_CUSTOMER_MAX],		
	   					CUST_TONE, 			V_MIB_LIST,	voipCfgParam_t, 				0,	mibtbl_cust_tone)

// disconnect tone det
MIBDEF(unsigned char, 	distone_num,		NOREP, 		DISTONE_NUM,		V_BYTE, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	d1freqnum,			NOREP, 		D1FREQNUM,			V_BYTE, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d1Freq1, 			NOREP, 		D1FREQ1,  			V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d1Freq2, 			NOREP, 		D1FREQ2,  			V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	d1Accur,			NOREP, 		D1ACCUR,			V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d1Level, 			NOREP, 		D1LEVEL,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d1ONup, 			NOREP, 		D1ONUP,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d1ONlow, 			NOREP, 		D1ONLOW,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d1OFFup, 			NOREP, 		D1OFFUP,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d1OFFlow, 			NOREP, 		D1OFFLOW,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	d2freqnum,			NOREP, 		D2FREQNUM,			V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d2Freq1, 			NOREP, 		D2FREQ1,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d2Freq2, 			NOREP, 		D2FREQ2,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	d2Accur,			NOREP, 		D2ACCUR,			V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d2Level, 			NOREP, 		D2LEVEL,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d2ONup, 			NOREP, 		D2ONUP,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d2ONlow, 			NOREP, 		D2ONLOW,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d2OFFup, 			NOREP, 		D2OFFUP,  			V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	d2OFFlow, 			NOREP, 		D2OFFLOW,  			V_WORD,		voipCfgParam_t, 0, 0)

// ring
MIBDEF(unsigned char, 	ring_cad,			NOREP, 		RING_CAD,			V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	ring_group,			NOREP, 		RING_GROUP,			V_BYTE,		voipCfgParam_t, 0, 0)
MIBARY(unsigned int, 	ring_phone_num,					[RING_GROUP_MAX], 			
						RING_PHONE_NUM, 				V_UINT_LIST,					voipCfgParam_t, 0, 0)
MIBARY(unsigned char,	ring_cadence_use,				[RING_GROUP_MAX],			
						RING_CADENCE_USE, 				V_BYTE_LIST,					voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	ring_cadence_sel,	NOREP, 		RING_CADENCE_SEL,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBARY(unsigned short, 	ring_cadon,						[RING_CADENCE_MAX],			
						RING_CAD_ON, 					V_WORD_LIST,					voipCfgParam_t, 0, 0)
MIBARY(unsigned short, 	ring_cadoff,					[RING_CADENCE_MAX],			
						RING_CAD_OFF, 					V_WORD_LIST,					voipCfgParam_t, 0, 0)

// function key
MIBSTR(char, 			funckey_pstn,					[FUNC_KEY_LENGTH],			
						FUNCKEY_PSTN, 					V_STRING,						voipCfgParam_t, 0, 0)
MIBSTR(char, 			funckey_transfer,				[FUNC_KEY_LENGTH],			
						FUNCKEY_TRANSFER, 				V_STRING,						voipCfgParam_t, 0, 0)

// other
MIBDEF(unsigned short,	auto_dial, 			NOREP, 		AUTO_DIAL,  		V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	off_hook_alarm,		NOREP, 		OFF_HOOK_ALARM,		V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	cid_auto_det_select,			NOREP, 
						CALLER_ID_AUTO_DET_SELECT, 		V_WORD,							voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	caller_id_det_mode,	NOREP, 		CALLER_ID_DET_MODE,	V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	one_stage_dial,		NOREP, 		ONE_STAGE_DIAL,		V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	two_stage_dial,		NOREP, 		TWO_STAGE_DIAL,		V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	auto_bypass_relay,	NOREP, 		AUTO_BYPASS_RELAY,	V_BYTE,		voipCfgParam_t, 0, 0)

// pulse dial
MIBDEF(unsigned char, 	pulse_dial_gen,		NOREP, 		PULSE_DIAL_GENERATE,V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	pulse_gen_pps,		NOREP, 		PULSE_GEN_PPS,		V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	pulse_gen_make_time,NOREP, 		PULSE_GEN_MAKE_TIME,V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	pulse_gen_interdigit_pause,		NOREP, 
						PULSE_GEN_INTERDIGIT_PAUSE,		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	pulse_dial_det,		NOREP, 		PULSE_DIAL_DETECT,	V_BYTE, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	pulse_det_pause,	NOREP, 		PULSE_DET_PAUSE, 	V_WORD, 	voipCfgParam_t, 0, 0)

// auto config
MIBDEF(unsigned short,	auto_cfg_ver,		NOREP, 		AUTO_CFG_VER,		V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	auto_cfg_mode,		NOREP, 		AUTO_CFG_MODE,		V_BYTE,		voipCfgParam_t, 0, 0)
MIBSTR(char, 			auto_cfg_http_addr,	[DNS_LEN],	AUTO_CFG_HTTP_ADDR,	V_STRING,	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	auto_cfg_http_port,	NOREP, 		AUTO_CFG_HTTP_PORT,	V_WORD,		voipCfgParam_t, 0, 0)
MIBSTR(char, 			auto_cfg_tftp_addr,	[DNS_LEN],	AUTO_CFG_TFTP_ADDR,	V_STRING,	voipCfgParam_t, 0, 0)
MIBSTR(char, 			auto_cfg_ftp_addr,	[DNS_LEN],	AUTO_CFG_FTP_ADDR,	V_STRING,	voipCfgParam_t, 0, 0)
MIBSTR(char, 			auto_cfg_ftp_user,	[20],		AUTO_CFG_FTP_USER,	V_STRING,	voipCfgParam_t, 0, 0)
MIBSTR(char, 			auto_cfg_ftp_passwd,[20],		AUTO_CFG_FTP_PASSWD,V_STRING,	voipCfgParam_t, 0, 0)
MIBSTR(char, 			auto_cfg_file_path,				[MAX_AUTO_CONFIG_PATH],	
						AUTO_CFG_FILE_PATH,				V_STRING,						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	auto_cfg_expire,	NOREP, 		AUTO_CFG_EXPIRE,	V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char,	fw_update_mode,		NOREP, 		FW_UPDATE_MODE, 	V_BYTE, 	voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_tftp_addr,[DNS_LEN],	FW_UPDATE_TFTP_ADDR,V_STRING,	voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_http_addr,[DNS_LEN],	FW_UPDATE_HTTP_ADDR,V_STRING,	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	fw_update_http_port,NOREP, 		FW_UPDATE_HTTP_PORT,V_WORD,		voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_ftp_addr,	[DNS_LEN],	FW_UPDATE_FTP_ADDR,	V_STRING,	voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_ftp_user,				[MAX_FW_UPDATE_FTP],			
						FW_UPDATE_FTP_USER, 			V_STRING,						voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_ftp_passwd,			[MAX_FW_UPDATE_FTP],			
						FW_UPDATE_FTP_PASSWD, 			V_STRING,						voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_file_path,			[MAX_FW_UPDATE_PATH],			
						FW_UPDATE_FILE_PATH, 			V_STRING,						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	fw_update_power_on,	NOREP, 		FW_UPDATE_POWER_ON,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	fw_update_scheduling_day,		NOREP, 
						FW_UPDATE_SCHEDULING_DAY,		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	fw_update_scheduling_time,		NOREP,
						FW_UPDATE_SCHEDULING_TIME,		V_BYTE, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	fw_update_auto,					NOREP,
						FW_UPDATE_AUTO ,				V_BYTE, 						voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_file_prefix,			[MAX_FW_UPDATE_FILE_PREFIX],			
						FW_UPDATE_FILE_PREFIX, 			V_STRING,						voipCfgParam_t, 0, 0)
MIBDEF(unsigned long, 	fw_update_next_time,			NOREP, 
						FW_UPDATE_NEXT_TIME,			V_DWORD, 						voipCfgParam_t, 0, 0)
MIBSTR(char, 			fw_update_fw_version,			[MAX_FW_VERSION],			
						FW_UPDATE_FW_VERSION,			V_STRING,						voipCfgParam_t, 0, 0)

// VLAN setting of WAN Port
MIBDEF(unsigned char, 	wanVlanEnable,		NOREP, 		WAN_VLAN_ENABLE,	V_BYTE,		voipCfgParam_t, 0, 0)

// VLAN for Voice and protocl packets (SIP, STUN, DHCP, ARP, etc.)
MIBDEF(unsigned short,	wanVlanIdVoice,		NOREP, 		WAN_VLAN_ID_VOICE,	V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	wanVlanPriorityVoice,			NOREP, 
						WAN_VLAN_PRIORITY_VOICE,		V_BYTE, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	wanVlanCfiVoice,				NOREP, 
						WAN_VLAN_CFI_VOICE,				V_BYTE, 						voipCfgParam_t, 0, 0)

// VLAN for Data
MIBDEF(unsigned short,	wanVlanIdData,					NOREP, 
						WAN_VLAN_ID_DATA, 				V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	wanVlanPriorityData,			NOREP, 
						WAN_VLAN_PRIORITY_DATA,			V_BYTE, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	wanVlanCfiData,					NOREP, 
						WAN_VLAN_CFI_DATA,				V_BYTE, 						voipCfgParam_t, 0, 0)

// VLAN for Video
MIBDEF(unsigned short,	wanVlanIdVideo,					NOREP, 
						WAN_VLAN_ID_VIDEO, 				V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	wanVlanPriorityVideo,			NOREP, 
						WAN_VLAN_PRIORITY_VIDEO,		V_BYTE, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	wanVlanCfiVideo,				NOREP, 		
						WAN_VLAN_CFI_VIDEO,				V_BYTE, 						voipCfgParam_t, 0, 0)

MIBDEF(unsigned char, 	vlan_enable,		NOREP, 		VLAN_ENABLE,		V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_tag,			NOREP, 		VLAN_TAG,			V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	vlan_bridge_enable,	NOREP, 		VLAN_BRIDGE_ENABLE,	V_BYTE, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_bridge_tag,	NOREP, 		VLAN_BRIDGE_TAG, 	V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_bridge_port,	NOREP, 		VLAN_BRIDGE_PORT,	V_WORD, 	voipCfgParam_t, 0, 0)

MIBDEF(unsigned char, 	vlan_bridge_multicast_enable,	NOREP, 
						VLAN_BRIDGE_MULTICAST_ENABLE,	V_BYTE, 						voipCfgParam_t, 0, 0)

MIBDEF(unsigned short,	vlan_bridge_multicast_tag,		NOREP, 
						VLAN_BRIDGE_MULTICAST_TAG, 		V_WORD, 						voipCfgParam_t, 0, 0)

MIBDEF(unsigned char, 	vlan_host_enable,	NOREP, 		VLAN_HOST_ENABLE,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_host_tag,		NOREP, 		VLAN_HOST_TAG, 		V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_host_pri,		NOREP, 		VLAN_HOST_PRI, 		V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	vlan_wifi_enable,	NOREP, 		VLAN_WIFI_ENABLE,	V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_tag,		NOREP, 		VLAN_WIFI_TAG, 		V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_pri,		NOREP, 		VLAN_WIFI_PRI, 		V_WORD,		voipCfgParam_t, 0, 0)

MIBDEF(unsigned char, 	vlan_wifi_vap0_enable,			NOREP, 
						VLAN_WIFI_VAP0_ENABLE,			V_BYTE, 						voipCfgParam_t, 0, 0)

MIBDEF(unsigned short,	vlan_wifi_vap0_tag,	NOREP, 		VLAN_WIFI_VAP0_TAG,	V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_vap0_pri,	NOREP, 		VLAN_WIFI_VAP0_PRI,	V_WORD, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	vlan_wifi_vap1_enable,			NOREP, 
						VLAN_WIFI_VAP1_ENABLE,			V_BYTE, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_vap1_tag,	NOREP, 		VLAN_WIFI_VAP1_TAG,	V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_vap1_pri,	NOREP, 		VLAN_WIFI_VAP1_PRI,	V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	vlan_wifi_vap2_enable,			NOREP, 
						VLAN_WIFI_VAP2_ENABLE,			V_BYTE, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_vap2_tag,	NOREP, 		VLAN_WIFI_VAP2_TAG,	V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_vap2_pri,	NOREP, 		VLAN_WIFI_VAP2_PRI,	V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	vlan_wifi_vap3_enable,			NOREP, 
						VLAN_WIFI_VAP3_ENABLE,			V_BYTE, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_vap3_tag,	NOREP, 		VLAN_WIFI_VAP3_TAG,	V_WORD,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	vlan_wifi_vap3_pri,	NOREP, 		VLAN_WIFI_VAP3_PRI,	V_WORD,		voipCfgParam_t, 0, 0)

// enable HW-NAT
MIBDEF(unsigned char, 	hwnat_enable,		NOREP, HWNAT_ENABLE,			V_BYTE, 	voipCfgParam_t, 0, 0)

// bandwidth Mgr
MIBDEF(unsigned short,	bandwidth_LANPort0_Egress,		NOREP, 
						BANDWIDTH_LANPORT0_EGRESS, 		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_LANPort1_Egress,		NOREP, 
						BANDWIDTH_LANPORT1_EGRESS, 		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_LANPort2_Egress,		NOREP, 
						BANDWIDTH_LANPORT2_EGRESS,		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_LANPort3_Egress,		NOREP, 
						BANDWIDTH_LANPORT3_EGRESS, 		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_WANPort_Egress,		NOREP, 
						BANDWIDTH_WANPORT_EGRESS, 		V_WORD, 						voipCfgParam_t, 0, 0)

MIBDEF(unsigned short,	bandwidth_LANPort0_Ingress,		NOREP, 
						BANDWIDTH_LANPORT0_INGRESS,		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_LANPort1_Ingress,		NOREP, 
						BANDWIDTH_LANPORT1_INGRESS,		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_LANPort2_Ingress,		NOREP, 
						BANDWIDTH_LANPORT2_INGRESS,		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_LANPort3_Ingress,		NOREP, 
						BANDWIDTH_LANPORT3_INGRESS,		V_WORD, 						voipCfgParam_t, 0, 0)
MIBDEF(unsigned short,	bandwidth_WANPort_Ingress,		NOREP, 
						BANDWIDTH_WANPORT_INGRESS, 		V_WORD, 						voipCfgParam_t, 0, 0)

// FXO valume
MIBDEF(unsigned char, 	daa_txVolumne,		NOREP, 		DAA_TX_VOLUME,		V_BYTE,		voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	daa_rxVolumne,		NOREP, 		DAA_RX_VOLUME,		V_BYTE,		voipCfgParam_t, 0, 0)

// DSCP
MIBDEF(unsigned char, 	rtpDscp,			NOREP, 		RTP_DSCP,			V_BYTE, 	voipCfgParam_t, 0, 0)
MIBDEF(unsigned char, 	sipDscp,			NOREP, 		SIP_DSCP,			V_BYTE, 	voipCfgParam_t, 0, 0)

MIBLST(voipCfgPortParam_t,  ports,          [VOIP_PORTS],    
                            PORT,           V_MIB_LIST, voipCfgParam_t,               0,  mibtbl_voip_port)

#ifdef CONFIG_RTK_VOIP_IP_PHONE
// TODO This item was not defined
MIBLST(ui_falsh_layout_t,   ui,             NOREP,    
                            TODO,           V_MIB_LIST, voipCfgParam_t,               0,  TODO)
#endif

#endif /* MIB_VOIP_CFG_IMPORT */

