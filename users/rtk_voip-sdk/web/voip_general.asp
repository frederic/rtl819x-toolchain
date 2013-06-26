<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
<style> TABLE {width:375} </style>
<script language="javascript">
<!--
function spd_dial_remove_sel()
{
	var flag=false;

	for (var i=0; i<10; i++)
	{
		if (document.sipform.spd_sel[i].checked)
		{
			flag=true;
			break;
		}
	}

	if (!flag)
	{
		alert('You have to select first.');
		return false;
	}

	if (!confirm('Do you really want to remove the selected items?'))
	{
		return false;
	}


	for (var i=0; i<10; i++)
	{
		if (document.sipform.spd_sel[i].checked)
		{
			document.sipform.spd_sel[i].checked = false;
			document.sipform.spd_sel[i].disabled = true;
			document.all.spd_name[i].value = "";
			document.all.spd_url[i].value = "";
		}
	}
	return true;
}

function spd_dial_remove_all()
{
	if (!confirm('Do you really want to remove all items in the phone book?'))
	{
		return false;
	}

	for (var i=0; i<10; i++)
	{
		document.sipform.spd_sel[i].checked = false;
		document.sipform.spd_sel[i].disabled = true;
		document.all.spd_name[i].value = "";
		document.all.spd_url[i].value = "";
	}
	return true;
}

function spd_dial_edit()
{
	for (var i=0; i<10; i++)
	{
		document.sipform.spd_sel[i].disabled = document.all.spd_url[i].value == "";
		if (document.sipform.spd_sel[i].disabled)
			document.all.spd_name[i].value = "";
	}
}

function dtmfMode_change()
{
	document.sipform.dtmf_2833_pt.disabled = (document.sipform.dtmfMode.selectedIndex != 0);
	document.sipform.dtmf_2833_pi.disabled = (document.sipform.dtmfMode.selectedIndex != 0);
	document.sipform.fax_modem_2833_pt.disabled = (document.sipform.dtmfMode.selectedIndex != 0);
	document.sipform.fax_modem_2833_pi.disabled = (document.sipform.dtmfMode.selectedIndex != 0);
	document.sipform.sipInfo_duration.disabled = (document.sipform.dtmfMode.selectedIndex != 1);
}

function enable_hotline()
{
	document.sipform.hotline_number.disabled = !document.sipform.hotline_enable.checked;
}

function enable_dnd()
{
	document.sipform.dnd_from_hour.disabled = !document.sipform.dnd_mode[1].checked;
	document.sipform.dnd_from_min.disabled = !document.sipform.dnd_mode[1].checked;
	document.sipform.dnd_to_hour.disabled = !document.sipform.dnd_mode[1].checked;
	document.sipform.dnd_to_min.disabled = !document.sipform.dnd_mode[1].checked;
}

function t38param_click_check()
{
	document.sipform.T38MaxBuffer.disabled = !document.sipform.T38ParamEnable.checked;
	document.sipform.T38RateMgt.disabled = !document.sipform.T38ParamEnable.checked;
	document.sipform.T38MaxRate.disabled = !document.sipform.T38ParamEnable.checked;
	document.sipform.T38EnableECM.disabled = !document.sipform.T38ParamEnable.checked;
	document.sipform.T38ECCSignal.disabled = !document.sipform.T38ParamEnable.checked;
	document.sipform.T38ECCData.disabled = !document.sipform.T38ParamEnable.checked;
	document.sipform.T38EnableSpoof.disabled = !document.sipform.T38ParamEnable.checked;
	document.sipform.T38DuplicateNum.disabled = !document.sipform.T38ParamEnable.checked;
}

function vad_enable()
{
	document.sipform.sid_mode.disabled = !document.sipform.useVad.checked;
	document.sipform.sid_noiselevel.disabled = (!document.sipform.sid_mode[1].checked) || (!document.sipform.useVad.checked) ;
	document.sipform.sid_noisegain.disabled = (!document.sipform.sid_mode[2].checked) || (!document.sipform.useVad.checked) ;
}

function rtcp_click_check()
{
	document.sipform.RTCPInterval.disabled = !document.sipform.useRTCP.checked;
	document.sipform.useRTCPXR.disabled = !document.sipform.useRTCP.checked;
}

//-->
</script>
</head>
<body bgcolor="#ffffff" text="#000000">

<form method="post" action="/goform/voip_general_set" name=sipform>
<input type=hidden name=voipPort value="<%voip_general_get("voip_port");%>">

<%voip_general_get("proxy");%>

<p>
<b>NAT Traversal</b>
<table cellSpacing=1 cellPadding=2 border=0>
<%voip_general_get("stun");%>
</table>

<p>
<b>SIP Advanced</b>
<table cellSpacing=1 cellPadding=2 border=0>
	<tr>
		<td bgColor=#aaddff>SIP Port</td>
    	<td bgColor=#ddeeff>
		<input type=text name=sipPort size=10 maxlength=5 value="<%voip_general_get("sipPort"); %>">
		<%voip_general_get("sipPorts");%>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Media Port</td>
    	<td bgColor=#ddeeff>
		<input type=text name=rtpPort size=10 maxlength=5 value="<%voip_general_get("rtpPort"); %>">
		<%voip_general_get("rtpPorts");%>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff>DMTF Relay</td>
    	<td bgColor=#ddeeff>
			<select name=dtmfMode onchange="dtmfMode_change()">
				"<%voip_general_get("dtmfMode");%>"
			</select>
		</td>
	</tr>
	
	<tr>
		<td bgColor=#aaddff>DTMF RFC2833 Payload Type</td>
		<td bgColor=#ddeeff>
			<input type=text name=dtmf_2833_pt size=12 maxlength=3 value=<%voip_general_get("dtmf_2833_pt");%>>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>DTMF RFC2833 Packet Interval</td>
		<td bgColor=#ddeeff>
			<input type=text name=dtmf_2833_pi size=12 maxlength=3 value=<%voip_general_get("dtmf_2833_pi");%>>(msec) (Must be multiple of 10msec)
		</td>
	</tr>
	
	<tr>
		<td bgColor=#aaddff>Use DTMF RFC2833 PT as Fax/Modem RFC2833 PT</td>
		<td bgColor=#ddeeff><input type=checkbox name=fax_modem_2833_pt_same_dtmf <%voip_general_get("fax_modem_2833_pt_same_dtmf");%>>Enable</td>
	</tr>
	
	<tr>
		<td bgColor=#aaddff>Fax/Modem RFC2833 Payload Type</td>
		<td bgColor=#ddeeff>
			<input type=text name=fax_modem_2833_pt size=12 maxlength=3 value=<%voip_general_get("fax_modem_2833_pt");%>>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Fax/Modem RFC2833 Packet Interval</td>
		<td bgColor=#ddeeff>
			<input type=text name=fax_modem_2833_pi size=12 maxlength=3 value=<%voip_general_get("fax_modem_2833_pi");%>>(msec) (Must be multiple of 10msec)
		</td>
	</tr>
	
	<tr>
		<td bgColor=#aaddff>SIP INFO Duration (ms)</td>
		<td bgColor=#ddeeff>
			<input type=text name=sipInfo_duration size=12 maxlength=4 value=<%voip_general_get("sipInfo_duration");%>>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Call Waiting</td>
		<td bgColor=#ddeeff><input type=checkbox name=call_waiting onclick="enable_callwaiting();" <%voip_general_get("call_waiting");%>>Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Call Waiting Caller ID</td>
		<td bgColor=#ddeeff><input type=checkbox name=call_waiting_cid <%voip_general_get("call_waiting_cid");%>>Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Reject Direct IP Call</td>
		<td bgColor=#ddeeff><input type=checkbox name=reject_direct_ip_call <%voip_general_get("reject_direct_ip_call");%>>Enable</td>
	</tr>
</table>

<p>
<b>Forward Mode</b>
<table cellSpacing=1 cellPadding=2 border=0>
<tr>
<td bgColor=#aaddff width=155>Immediate Forward to</td>
<td bgColor=#ddeeff width=170>
<%voip_general_get("CFAll");%>
</td>
</tr>
<tr>
<td bgColor=#aaddff>Immediate Number</td>
<td bgColor=#ddeeff>
<%voip_general_get("CFAll_No");%>
</td>
</tr>
<tr>
<td bgColor=#aaddff>Busy Forward to</td>
<td bgColor=#ddeeff>
<%voip_general_get("CFBusy");%>
</td>
</tr>
<tr>
<td bgColor=#aaddff>Busy Number</td>
<td bgColor=#ddeeff>
<%voip_general_get("CFBusy_No");%>
</td>
</tr>
<%voip_general_get("CFNoAns");%>
</table>

<%voip_general_get("speed_dial_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 <%voip_general_get("speed_dial_display");%>>
	<tr align=center>
		<td bgcolor=#aaddff>Position</td>
		<td bgcolor=#aaddff>Name</td>
		<td bgcolor=#aaddff>Phone Number</td>
		<td bgcolor=#aaddff>Select</td>
	</tr>
	<%voip_general_get("speed_dial");%>
	<tr align=center>
		<td colspan=4 bgcolor=#ddeeff>
		<input type=button value="Remove Selected" name="RemoveSelected" onClick="spd_dial_remove_sel()">
		<input type=button value="Remove All" name="RemoveAll" onClick="spd_dial_remove_all()">
		</td>
	</tr>
</table>

<%voip_general_get("not_ipphone_option_start");%>
<p><b>Abbreviated Dial</b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 <%voip_general_get("not_ipphone_table");%> >
	<tr align=center>
		<td bgcolor=#aaddff>Abbreviated Name</td>
		<td bgcolor=#aaddff>Phone Number</td>
	</tr>
	<%voip_general_get("abbreviated_dial");%>
</table>

<%voip_general_get("display_dialplan_title");%>
<table cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("display_dialplan");%>>
	<tr>
    	<td bgColor=#aaddff width=155>Replace prefix code</td>
    	<td bgColor=#ddeeff width=170>
    	<%voip_general_get("ReplaceRuleOption");%>
    	</td>
	</tr>
	<tr>
     	<td bgColor=#aaddff width=155>Relace rule</td>
   		<td bgColor=#ddeeff width=170>
    	<input type="text" name="ReplaceRuleSource" size=12 maxlength=79 value="<%voip_general_get("ReplaceRuleSource");%>"> ->
    	<input type="text" name="ReplaceRuleTarget" size=3 maxlength=9 value="<%voip_general_get("ReplaceRuleTarget");%>"></td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>Dial Plan</td>
    	<td bgColor=#ddeeff width=170>
		<input type="text" name="dialplan" size=20 maxlength=79 value="<%voip_general_get("dialplan");%>"></td>
	</tr>
	<tr>
     	<td bgColor=#aaddff width=155>Auto Prefix</td>
    	<td bgColor=#ddeeff width=170>
    	<input type="text" name="AutoPrefix" size=5 maxlength=4 value="<%voip_general_get("AutoPrefix");%>">
	</tr>
	<tr>
     	<td bgColor=#aaddff width=155>Prefix Unset Plan</td>
    	<td bgColor=#ddeeff width=170>
    	<input type="text" name="PrefixUnsetPlan" size=20 maxlength=79 value="<%voip_general_get("PrefixUnsetPlan");%>">
	</tr>
</table>

<%voip_general_get("not_ipphone_option_start");%>
<p><b>PSTN Routing Prefix</b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("not_ipphone_table");%> >
	<tr>
		<td bgColor=#aaddff width=155>Prefix List (Delimiter: ',')</td>
    	<td bgColor=#ddeeff>
		<input type="text" name="PSTNRoutingPrefix" size=20 maxlength=127 value="<%voip_general_get("PSTNRoutingPrefix");%>" <%voip_general_get("PSTNRoutingPrefixDisabled");%>>
    	</td>
	</tr>
</table>

<p>
<b>Codec</b>
<!-- RTP Redundant  -->
<table  cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("RTP_RED_BUILD");%>>
	<tr>
		<td bgColor=#aaddff rowspan=2>RTP Redundant<br>(First precedence)</td>
		<td bgColor=#ddeeff>Codec</td>
		<td bgColor=#ddeeff>
			<select name=rtp_redundant_codec>
				<%voip_general_get("rtp_redundant_codec_options");%>
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#ddeeff>Payload Type</td>
		<td bgColor=#ddeeff><input type=text name=rtp_redundant_payload_type size=10 value="<%voip_general_get("rtp_redundant_payload_type");%>"></td>
	</tr>
</table>
<%voip_general_get("codec_var");%>
<table cellSpacing=1 cellPadding=2 border=0>
<%voip_general_get("codec");%>
</table>
<%voip_general_get("codec_opt");%>

<p>
<table cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("display_voice_qos");%>>
<b>QoS</b>
  	<tr>
    	<td bgColor=#aaddff>Voice QoS</td>
		<td bgColor=#ddeeff>
		<select name=voice_qos>
			 "<%voip_general_get("voice_qos");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
</table>

<!-- V.152 -->
<p <%voip_general_get("not_dect_port_option");%>>
<table  cellSpacing=1 cellPadding=1 border=0 >
<b>V.152</b>
	<tr>
		<td bgColor=#aaddff>V.152</td>
		<td bgColor=#ddeeff><input type=checkbox name=useV152 size=20 <%voip_general_get("useV152");%>>Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>V.152 payload type</td>
		<td bgColor=#ddeeff><input type=text name=V152_payload_type size=20 value="<%voip_general_get("V152_payload_type");%>"></td>
	</tr>
	<tr>
		<td bgColor=#aaddff>V.152 codec type</td>
		<td bgColor=#ddeeff>
			<select name=V152_codec_type>
				<%voip_general_get("V152_codec_type_options");%>
			</select>
		</td>
	</tr>
</table>

<!-- ++T.38 config add by Jack Chan++ -->
<p <%voip_general_get("not_dect_port_option");%>>
<!-- style:display:none(hidden) style:display:table(show) -->
<table  cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("T38_BUILD");%>>
<b>T.38(FAX)</b>
	<tr>
		<td bgColor=#aaddff>T.38</td>
		<td bgColor=#ddeeff><input type=checkbox name=useT38 size=20 <%voip_general_get("useT38");%>>Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>T.38 Port</td>
		<td bgColor=#ddeeff><input type=text name=T38_PORT size=20 maxlength=39 value="<%voip_general_get("T38_PORT");%>"></td>
		<%voip_general_get("t38Ports");%>
	</tr>
	<tr>
		<td bgColor=#aaddff>Fax Modem Detection Mode</td>
		<td bgColor=#ddeeff>
			<select name=fax_modem_det_mode>
				"<%voip_general_get("fax_modem_det_mode");%>"
			</select>
		</td>
	</tr>
</table>
<!-- --end-- -->

<!-- ++T.38 config add by Jack Chan++ -->
<p <%voip_general_get("not_dect_port_option");%>>
<!-- style:display:none(hidden) style:display:table(show) -->
<table  cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("T38_BUILD");%>>
<b>T.38(customize parameters)</b>
	<tr>
		<td bgColor=#aaddff>Custome parameters</td>
		<td bgColor=#ddeeff><input type=checkbox name=T38ParamEnable size=20 <%voip_general_get("T38ParamEnable");%> onclick="t38param_click_check()">Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Max buffer</td>
		<td bgColor=#ddeeff><input type=text name=T38MaxBuffer size=20 maxlength=39 value="<%voip_general_get("T38MaxBuffer");%>"></td>
	</tr>
	<tr>
		<td bgColor=#aaddff>TCF</td>
		<td bgColor=#ddeeff>
			<select name=T38RateMgt>
				<%voip_general_get("T38RateMgt");%>
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Max Rate</td>
		<td bgColor=#ddeeff>
			<select name=T38MaxRate>
				<%voip_general_get("T38MaxRate");%>
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>ECM</td>
		<td bgColor=#ddeeff><input type=checkbox name=T38EnableECM size=20 <%voip_general_get("T38EnableECM");%>>Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>ECC Signal</td>
		<td bgColor=#ddeeff>
			<select name=T38ECCSignal>
				<%voip_general_get("T38ECCSignal");%>
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>ECC Data</td>
		<td bgColor=#ddeeff>
			<select name=T38ECCData>
				<%voip_general_get("T38ECCData");%>
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Spoofing</td>
		<td bgColor=#ddeeff><input type=checkbox name=T38EnableSpoof size=20 <%voip_general_get("T38EnableSpoof");%>>Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Packet Duplicate Num</td>
		<td bgColor=#ddeeff>
			<select name=T38DuplicateNum>
				<%voip_general_get("T38DuplicateNum");%>
			</select>
		</td>
	</tr>
</table>
<script language="JavaScript">
<!--
	t38param_click_check();
//-->
</script>
<!-- --end-- -->

<p>
<!-- style:display:none(hidden) style:display:table(show) -->
<table  cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("SRTP_BUILD");%>>
<b>VoIP Security</b>
	<tr>
		<td bgColor=#aaddff>Make secruity call</td>
		<td bgColor=#ddeeff><input type=checkbox name=useSRTP size=20 <%voip_general_get("useSRTP");%>>Enable</td>
	</tr>
</table>
<!-- ++VoIP Security++ -->

<!-- --end-- -->
<%voip_general_get("hotline_option_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("hotline_option_display");%>>
<tr>
   	<td bgColor=#aaddff width=150>Use Hot Line</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=hotline_enable onClick="enable_hotline()" <%voip_general_get("hotline_enable");%>>Enable
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>Hot Line Number</td>
	<td bgColor=#ddeeff>
		<input type=text name=hotline_number size=20 maxlength=39 value="<%voip_general_get("hotline_number");%>">
	</td>
</tr>
</table>
<script language=javascript>enable_hotline()</script>

<p>
<b>DND (Don't Disturb)</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr>
   	<td bgColor=#aaddff width=150>DND Mode</td>
	<td bgColor=#ddeeff>
		<input type=radio name=dnd_mode value=2 onClick="enable_dnd()" <%voip_general_get("dnd_always");%>>Always
		<input type=radio name=dnd_mode value=1 onClick="enable_dnd()" <%voip_general_get("dnd_enable");%>>Enable
		<input type=radio name=dnd_mode value=0 onClick="enable_dnd()" <%voip_general_get("dnd_disable");%>>Disable
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>From</td>
	<td bgColor=#ddeeff>
		<input type=text name=dnd_from_hour size=3 maxlength=2 value="<%voip_general_get("dnd_from_hour");%>">:
		<input type=text name=dnd_from_min size=3 maxlength=2 value="<%voip_general_get("dnd_from_min");%>">
		(hh:mm)
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>To</td>
	<td bgColor=#ddeeff>
		<input type=text name=dnd_to_hour size=3 maxlength=2 value="<%voip_general_get("dnd_to_hour");%>">:
		<input type=text name=dnd_to_min size=3 maxlength=2 value="<%voip_general_get("dnd_to_min");%>">
		(hh:mm)
	</td>
</tr>
</table>
<script language=javascript>enable_dnd()</script>

<%voip_general_get("not_ipphone_option_start");%>
<p>
<b>Authentication</b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("not_ipphone_table");%> >
<tr>
   	<td bgColor=#aaddff width=150>Off-Hook Password</td>
	<td bgColor=#ddeeff>
		<input type=text name=offhook_passwd size=20 maxlength=9 value="<%voip_general_get("offhook_passwd");%>">
	</td>
</tr>
</table>

<%voip_general_get("not_ipphone_option_start");%>
<p <%voip_general_get("not_dect_port_option");%>>
<b>Alarm</b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("not_ipphone_table");%> >
<tr>
   	<td bgColor=#aaddff width=150>Enable</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=alarm_enable <%voip_general_get("alarm_enable");%> <%voip_general_get("alarm_disabled");%>>
	</td>
</tr>
</tr>
   	<td bgColor=#aaddff width=150>Time</td>
	<td bgColor=#ddeeff>
		<input type=text name=alarm_hh size=3 maxlength=2 value="<%voip_general_get("alarm_hh");%>" <%voip_general_get("alarm_disabled");%>>:
		<input type=text name=alarm_mm size=3 maxlength=2 value="<%voip_general_get("alarm_mm");%>" <%voip_general_get("alarm_disabled");%>> (hh:mm)
	</td>
</tr>
</table>

<p>
<b>DSP</b>
<table cellSpacing=1 cellPadding=2 border=0>
<!--
  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=2>FXS Volume</td>
		<td bgColor=#ddeeff width=170>Handset Gain</td>
		<td bgColor=#ddeeff>
			<select name=slic_txVolumne>
				  "<%voip_general_get("slic_txVolumne");%>"
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#ddeeff width=170>Handset Volume</td>
		<td bgColor=#ddeeff>
			<select name=slic_rxVolumne>
				 "<%voip_general_get("slic_rxVolumne");%>"
			</select>
		</td>
	</tr>
-->

  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=3>Jitter Buffer Control</td>
		<td bgColor=#ddeeff width=170>
		Min delay (ms):
		</td>
		<td bgColor=#ddeeff>
		<select name=jitterDelay>
			<%voip_general_get("jitterDelay");%>
		</select>
		</td>
	</tr>
	<tr>
		<td bgcolor=#ddeeff width=170>
		Max delay (ms):
		</td>
		<td bgColor=#ddeeff>
		<select name=maxDelay>
	        <%voip_general_get("maxDelay");%>
	    </select>
		</td>
	</tr>
	<tr>
		<td bgcolor=#ddeeff width=170>
		Optimization factor:
		</td>
		<td bgColor=#ddeeff>
		<select name=jitterFactor>
			<%voip_general_get("jitterFactor");%>
		</select>
		</td>
	</tr>

<!--
  	<tr>
    	<td bgColor=#aaddff>LEC Tail Length (ms)</td>
		<td bgColor=#ddeeff>
		<select name=echoTail>
			 "<%voip_general_get("echoTail");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
-->
  	<tr>
    	<td bgColor=#aaddff width=155>LEC</td>
		<td bgColor=#ddeeff width=170>
			<input type=checkbox name=useLec size=20 <%voip_general_get("useLec");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>NLP</td>
		<td bgColor=#ddeeff width=170>
			<input type=checkbox name=useNlp size=20 <%voip_general_get("useNlp");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>VAD</td>
		<td bgColor=#ddeeff width=170>
		   <input type=checkbox name=useVad size=20 onClick="vad_enable()" <%voip_general_get("useVad");%>>Enable

		</td>
		<td bgColor=#ddeeff></td>
	</tr>
	<tr>
		<td bgColor=#aaddff width=155>VAD Amp. Threshold (0 < Amp < 200)</td>
		<td bgColor=#ddeeff width=170>
			<input type=text name=Vad_threshold size=4 maxlength=5 value="<%voip_general_get("Vad_threshold");%>"> (Amp.)
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

	<tr>
		<td bgColor=#aaddff width=155 rowspan=3>SID Noise Level</td>

		<td bgColor=#ddeeff ><input type=radio name=sid_mode value=0  onClick="vad_enable()" <%voip_general_get("sid_config_enable");%>>Disable configuration </td>
		<td bgColor=#ddeeff></td> <tr>
		<td bgColor=#ddeeff ><input type=radio name=sid_mode value=1  onClick="vad_enable()" <%voip_general_get("sid_fixed_level");%>>Fixed noise level  </td>
		<td bgColor=#ddeeff ><input type=text name=sid_noiselevel size=4 maxlength=3 value="<%voip_general_get("sid_noiselevel");%>"> (0>Value>127 dBov)	</td> <tr>

		<td bgColor=#ddeeff ><input type=radio name=sid_mode value=2  onClick="vad_enable()" <%voip_general_get("sid_adjust_level");%>>Adjust noise level</td>
		<td bgColor=#ddeeff ><input type=text name=sid_noisegain size=4 maxlength=4 value="<%voip_general_get("sid_noisegain");%>">(-127~127 dBov, 0:Not change)</td> <tr>
	</tr>

	<tr>
    	<td bgColor=#aaddff width=155>CNG</td>
		<td bgColor=#ddeeff width=170>
		<input type=checkbox name=useCng size=20 <%voip_general_get("useCng");%>>Enable

		</td>
		<td bgColor=#ddeeff></td>
	</tr>

<script language=javascript>vad_enable()</script>

	<tr>
		<td bgColor=#aaddff width=155>CNG Max. Amp. (0 < Amp < 200, 0 means no limit for Max. Amp)</td>
		<td bgColor=#ddeeff width=170>
			<input type=text name=Cng_threshold size=4 maxlength=5 value="<%voip_general_get("Cng_threshold");%>"> (Amp.)
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>PLC</td>
		<td bgColor=#ddeeff width=170>
			<input type=checkbox name=usePLC size=20 <%voip_general_get("usePLC");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-CNG</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_CNG_TDM size=20 <%voip_general_get("useANSTONE_CNG_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_CNG_IP size=20 <%voip_general_get("useANSTONE_CNG_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-ANS</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_ANS_TDM size=20 <%voip_general_get("useANSTONE_ANS_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_ANS_IP size=20 <%voip_general_get("useANSTONE_ANS_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-ANSAM</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_ANSAM_TDM size=20 <%voip_general_get("useANSTONE_ANSAM_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=useANSTONE_ANSAM_IP size=20 <%voip_general_get("useANSTONE_ANSAM_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-ANSBAR</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_ANSBAR_TDM size=20 <%voip_general_get("useANSTONE_ANSBAR_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=useANSTONE_ANSBAR_IP size=20 <%voip_general_get("useANSTONE_ANSBAR_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-ANSAMBAR</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_ANSAMBAR_TDM size=20 <%voip_general_get("useANSTONE_ANSAMBAR_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=useANSTONE_ANSAMBAR_IP size=20 <%voip_general_get("useANSTONE_ANSAMBAR_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-BELLANS</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_BELLANS_TDM size=20 <%voip_general_get("useANSTONE_BELLANS_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=useANSTONE_BELLANS_IP size=20 <%voip_general_get("useANSTONE_BELLANS_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-V.22ANS</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_V22ANS_TDM size=20 <%voip_general_get("useANSTONE_V22ANS_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=useANSTONE_V22ANS_IP size=20 <%voip_general_get("useANSTONE_V22ANS_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-V8bis_Cre</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_V8bis_Cre_TDM size=20 <%voip_general_get("useANSTONE_V8bis_Cre_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=useANSTONE_V8bis_Cre_IP size=20 <%voip_general_get("useANSTONE_V8bis_Cre_IP");%>>Enable IP
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155>ANSTONE-V21flag</td>
		<td bgColor=#ddeeff width=140>
			<input type=checkbox name=useANSTONE_V21flag_TDM size=20 <%voip_general_get("useANSTONE_V21flag_TDM");%>>Enable TDM
		</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=useANSTONE_V21flag_IP size=20 <%voip_general_get("useANSTONE_V21flag_IP");%>>Enable IP
		</td>
	</tr>

	<tr>
    	<td bgColor=#aaddff width=155>RTCP</td>
		<td bgColor=#ddeeff width=170>
			<input type=checkbox name=useRTCP size=20 <%voip_general_get("useRTCP");%> onclick="rtcp_click_check();">Enable
		</td>
		<td bgColor=#ddeeff>
			Interval: <input type=text name=RTCPInterval maxlength=3 size=5 value=<%voip_general_get("RTCPInterval");%>> (sec)
		</td>
	</tr>
  	<tr <%voip_general_get("rtcp_xr_option");%>>
    	<td bgColor=#aaddff width=155>RTCP XR</td>
		<td bgColor=#ddeeff width=170>
			<input type=checkbox name=useRTCPXR size=20 <%voip_general_get("useRTCPXR");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
	<script language=javascript>rtcp_click_check();</script>

	<tr>
	<td bgColor=#aaddff width=155 rowspan=3>Fax/Modem RFC2833 Support</td>
		<td bgColor=#ddeeff width=140 colspan=2>
			<input type=checkbox name=useFaxModem2833Relay size=20 <%voip_general_get("useFaxModem2833Relay");%>>Enable Fax/Modem RFC2833 Relay(For TX)
		</td>		
	</tr>
	
	<tr>
		<td bgColor=#ddeeff width=140 colspan=2>
			<input type=checkbox name=useFaxModemInbandRemoval size=20 <%voip_general_get("useFaxModemInbandRemoval");%>>Enable Fax/Modem Inband Removal(For TX)
		</td>
	</tr>

	<tr>
		<td bgColor=#ddeeff width=140 colspan=2>
			<input type=checkbox name=useFaxModem2833RxTonePlay size=20 <%voip_general_get("useFaxModem2833RxTonePlay");%>>Enable Fax/Modem Tone Play(For RX)	
		</td>
	</tr>
	
  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=4>Speaker AGC</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=CFuseSpeaker size=20 onClick="enableCFSpkAGC(this.checked)" <%voip_general_get("CFuseSpeaker");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>

		<td bgColor=#ddeeff width=170>require level:</td>
		<td bgColor=#ddeeff>
			<select name=CF_spk_AGC_level>
				  "<%voip_general_get("CF_spk_AGC_level");%>"
			</select>
		</td>
	</tr>


  	<tr>

		<td bgColor=#ddeeff width=170>Max gain up: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_spk_AGC_up_limit>
				  "<%voip_general_get("CF_spk_AGC_up_limit");%>"
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#ddeeff width=170>Max gain down: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_spk_AGC_down_limit>
				 "<%voip_general_get("CF_spk_AGC_down_limit");%>"
			</select>
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=4>MIC AGC</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=CFuseMIC size=20 onClick="enableCFMicAGC(this.checked)" <%voip_general_get("CFuseMIC");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>

		<td bgColor=#ddeeff width=170>require level:</td>
		<td bgColor=#ddeeff>
			<select name=CF_mic_AGC_level>
				  "<%voip_general_get("CF_mic_AGC_level");%>"
			</select>
		</td>
	</tr>

  	<tr>

		<td bgColor=#ddeeff width=170>Max gain up: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_mic_AGC_up_limit>
				  "<%voip_general_get("CF_mic_AGC_up_limit");%>"
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#ddeeff width=170>Max gain down: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_mic_AGC_down_limit>
				 "<%voip_general_get("CF_mic_AGC_down_limit");%>"
			</select>
		</td>
	</tr>

	<!-- ----------------------------------------------------------- -->
	<!-- Not IP phone option start -->
	<%voip_general_get("not_ipphone_option_start");%>
  	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>Caller ID Mode</td>
	<td bgColor=#ddeeff><%voip_general_get("caller_id");%></td>
	<td bgColor=#ddeeff></td>
	</tr>

	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>FSK Date & Time Sync</td>
	<td bgColor=#ddeeff><%voip_general_get("FSKdatesync");%></td>
	<td bgColor=#ddeeff></td>
	</tr>

	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>Reverse Polarity before Caller ID</td>
	<td bgColor=#ddeeff><%voip_general_get("revPolarity");%></td>
	<td bgColor=#ddeeff></td>
	</tr>

	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>Short Ring before Caller ID</td>
	<td bgColor=#ddeeff><%voip_general_get("sRing");%></td>
	<td bgColor=#ddeeff></td>
	</tr>

	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>Dual Tone before Caller ID</td>
	<td bgColor=#ddeeff><%voip_general_get("dualTone");%></td>
	<td bgColor=#ddeeff></td>
	</tr>

	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>Caller ID Prior First Ring</td>
	<td bgColor=#ddeeff><%voip_general_get("PriorRing");%></td>
	<td bgColor=#ddeeff></td>
	</tr>

	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>Caller ID DTMF Start Digit</td>
	<td bgColor=#ddeeff><%voip_general_get("cid_dtmfMode_S");%></td>
	<td bgColor=#ddeeff></td>
	</tr>

	<tr <%voip_general_get("not_dect_port_option");%>>
	<td bgColor=#aaddff>Caller ID DTMF End Digit</td>
	<td bgColor=#ddeeff><%voip_general_get("cid_dtmfMode_E");%></td>
	<td bgColor=#ddeeff></td>
	</tr>


	<tr <%voip_general_get("not_dect_port_option");%>>
	   	<td bgColor=#aaddff width=155>Flash Time Setting (ms) [ Space:10, Min:80, Max:2000 ]</td>
		<td bgColor=#ddeeff width=170>
		<%voip_general_get("flash_hook_time");%>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
	<%voip_general_get("not_ipphone_option_end");%>
	<!-- not IP phone option end -->
	<!-- ----------------------------------------------------------- -->

<!-- thlin: Gen FSK Caller ID with sw DSP only -->
<!--
	<tr>
	<td bgColor=#aaddff>Caller ID Soft FSK Gen</td>
	<td bgColor=#ddeeff><%voip_general_get("SoftFskGen");%></td>
	<td bgColor=#ddeeff>Hardware caller id only support si3215/3210 slic</td>
	</tr>
-->

	<tr>
		<td bgColor=#aaddff width=155>Speaker Voice Gain (dB) [ -32~31 ],Mute:-32</td>
		<td bgColor=#ddeeff width=170>
			<input type=text name=spk_voice_gain size=4 maxlength=5 value="<%voip_general_get("spk_voice_gain");%>">
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

	<tr>
		<td bgColor=#aaddff width=155>Mic Voice Gain (dB) [ -32~31 ],Mute:-32</td>
		<td bgColor=#ddeeff width=170>
			<input type=text name=mic_voice_gain size=4 maxlength=5 value="<%voip_general_get("mic_voice_gain");%>">
		</td>
		<td bgColor=#ddeeff></td>
	</tr>


	<tr>
    	<td colspan=3 align=center>
    		<input type="button" value="Apply" onclick="changeStartEnd();">
    		&nbsp;&nbsp;&nbsp;&nbsp;
    		<input type="reset" value="Reset">
    	</td>
	</tr>
</table>
<script language=javascript>init();</script>
<p>
</form>
</table>
</body>
</html>
