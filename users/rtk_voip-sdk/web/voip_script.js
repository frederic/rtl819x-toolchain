function isArray(obj) {
	return (typeof(obj.length) == "undefined") ? false : true;
}

function isNumeric(num,min,max)
{  
	return !isNaN(num) && num >= min && num <= max;
}

function ERROR(obj, msg)
{
	alert(msg);
	obj.select();
	obj.focus();
}

function updatePrecedence(chk_objs, res_objs)
{
	var len;
	var selection;

	len = res_objs.length;
	selection = 0;
	for (i=0; i<len; i++)
	{
		for (j=0; j<len; j++)
		{
			if (chk_objs[i * len + j].checked == true)
			{
				res_objs[i].value = j;
				selection++;
				break;
			}
		}
	}

	return selection;
}

function checkPrecedence(objs, row, col, len, force_checked)
{
	var x, y;
	var i, len, idx;
	var obj;

	x = -1;
	y = -1;
	obj = objs[row * len + col];
	if (obj.checked == true)
	{
		for (i=0; i<len; i++)
		{
			idx = row * len + i;
			if (obj == objs[idx])
				continue;
			if (objs[idx].checked == true)
			{
				x = i;
				objs[idx].checked = false;
				break;
			}
		}

		for (i=0; i<len; i++)
		{
			idx = i * len + col;
			if (obj == objs[idx])
				continue;
			if (objs[idx].checked == true)
			{
				y = i;
				objs[idx].checked = false;
				break;
			}
		}

		if ((x != -1) && (y != -1))
			objs[x + y * len].checked = true;
	}
	else if (force_checked)
	{
		obj.checked = true;
	}
}

function enableCFSpkAGC()
{

	if (sipform.CFuseSpeaker.checked == false)
	{
		sipform.CF_spk_AGC_level.disabled=true;
		sipform.CF_spk_AGC_up_limit.disabled=true;
		sipform.CF_spk_AGC_down_limit.disabled=true;
	}
	else
	{
		sipform.CF_spk_AGC_level.disabled=false;
		sipform.CF_spk_AGC_up_limit.disabled=false;
		sipform.CF_spk_AGC_down_limit.disabled=false;
		
		sipform.CFuseMIC.checked = false;
		enableCFMicAGC();
	}

}

function enableCFMicAGC()
{
	if (sipform.CFuseMIC.checked == false)
	{
		sipform.CF_mic_AGC_level.disabled=true;
		sipform.CF_mic_AGC_up_limit.disabled=true;
		sipform.CF_mic_AGC_down_limit.disabled=true;
	}
	else
	{
		sipform.CF_mic_AGC_level.disabled=false;
		sipform.CF_mic_AGC_up_limit.disabled=false;
		sipform.CF_mic_AGC_down_limit.disabled=false;
		
		sipform.CFuseSpeaker.checked = false;
		enableCFSpkAGC();
	}
}

function check_nortel_proxy()
{
	var use_nortel_proxy;

	if (isArray(sipform.proxyNortel))
		use_nortel_proxy = sipform.proxyNortel[0].checked;
	else
		use_nortel_proxy = sipform.proxyNortel.checked;

	sipform.stunEnable.disabled = use_nortel_proxy;
	sipform.stunAddr.disabled = use_nortel_proxy;
	sipform.stunPort.disabled = use_nortel_proxy;
}

function init()
{
	check_nortel_proxy();
	enableCFSpkAGC();
	enableCFMicAGC();
}

function chkPayloadType(obj)
{
	var v0 = obj.value;
	var v1 = parseInt(v0);
	var ret = false;

	if (isNaN(v1) || v1 < 96 || v1 > 127)
		alert('Payload Type' + v1 + ' is out of range [96-127]');
	else if (v1 == 97 || v1 == 98)
		alert('Payload Type ' + v1 + ' has reserved for iLBC');
	else if (v1 == 99 || v1 == 100)
		alert('Payload Type ' + v1 + ' has reserved for G711.1');
	else if (v1 >= 103 && v1 <= 106)
		alert('Payload Type ' + v1 + ' has reserved for G726');
	else
		ret = true;

	if (!ret && !obj.disabled)
	{
		obj.select();
		obj.focus();
	}

	return ret;
}

function checkDialPlanRegex( szRegex )
{
	var re = new RegExp ("^[0-9xX\*\#\+nNmM]*$", "g");

	return re.test(szRegex);
}

function checkDialPlanDigits( szDigits )
{
	var re = new RegExp ("^[0-9\*\#]*$", "g");

	return re.test(szDigits);
}

function checkPSTNRoutingPrefixRegex( szRegex )
{
	var re = new RegExp ("^[0-9,]*$", "g");

	return re.test(szRegex);
}

function changeStartEnd() 
{
	var proxy_enables, proxy_addrs, proxy_ports, reg_expires;
	var obproxy_enables, obproxy_addrs, obproxy_ports;
	var stun_enables, stun_addrs, stun_ports;
	var sip_ports, rtp_ports, t38_ports;
	var voipport = parseInt(sipform.voipPort.value);
	var sipport = parseInt(sipform.sipPort.value);
	var rtpport = parseInt(sipform.rtpPort.value);
	var t38port = parseInt(sipform.T38_PORT.value);
	var reg_pstn	= /^[*#0-9]*$/;

	// proxies
	if (isArray(sipform.proxyEnable))
	{
		proxy_enables = sipform.proxyEnable;
		proxy_addrs = sipform.proxyAddr;
		proxy_ports = sipform.proxyPort;
		reg_expires = sipform.regExpiry;
		obproxy_enables = sipform.obEnable;
		obproxy_addrs = sipform.obProxyAddr;
		obproxy_ports = sipform.obProxyPort;
		reg_expires = sipform.regExpiry;
	}
	else
	{
		proxy_enables = new Array();
		proxy_enables[0] = sipform.proxyEnable;
		proxy_addrs = new Array();
		proxy_addrs[0] = sipform.proxyAddr;
		proxy_ports = new Array();
		proxy_ports[0] = sipform.proxyPort;
		reg_expires = new Array();
		reg_expires[0] = sipform.regExpiry;
		obproxy_enables = new Array();
		obproxy_enables[0] = sipform.obEnable;
		obproxy_addrs = new Array();
		obproxy_addrs[0] = sipform.obProxyAddr;
		obproxy_ports = new Array();
		obproxy_ports[0] = sipform.obProxyPort;
	}

	for (var i=0; i<proxy_enables.length; i++)
	{
		var regexpiry   = parseInt(reg_expires[i].value);

		if (proxy_enables[i].checked == true)
		{
			if (proxy_addrs[i].value == "")
			{
				ERROR(proxy_addrs[i], 'Porxy Addr cannot be empty');
				return false;			
			}
			if (!isNumeric(proxy_ports[i].value, 0, 65535))
			{
				ERROR(proxy_ports[i], 'Porxy Port is out of range [0-65535]');
				return false;
			}
		}
		if (!isNumeric(reg_expires[i].value, 10, 86400))
		{
			ERROR(reg_expires[i], 'Register Expire out of range [10-86400]');
			return false;
		}
		if (obproxy_enables[i].checked == true)
		{
			if (obproxy_addrs[i].value == "")
			{
				ERROR(obproxy_addrs[i], 'Outbound Porxy Addr cannot be empty');
				return false;
			}
			if (!isNumeric(obproxy_ports[i].value, 0, 65535))
			{
				ERROR(obproxy_ports[i], 'Outbound Porxy Port is out of range [0-65535]');
				return false;
			}
		}
	}

	if (sipform.stunEnable.checked == true)
	{
		if (sipform.stunAddr.value == "")
		{
			ERROR(sipform.stunAddr, 'Stun Server Addr cannot be empty');
			return false;
		}
		if (!isNumeric(sipform.stunPort.value, 0, 65535))
		{
			ERROR(sipform.stunPort, 'Stun Server Port is out of range [0-65535]');
			return false;
		}
	}

	if (!isNumeric(sipform.sipPort.value, 5000, 10000))
	{
		ERROR(sipform.sipPort, 'SIP Port out of range [5000-10000]');
		return false;
	}

	if (!isNumeric(sipform.rtpPort.value, 5000, 10000))
	{
		ERROR(sipform.rtpPort, 'Media Port out of range [5000-10000]');
		return false;
	}

	if (!isNumeric(sipform.T38_PORT.value, 5000, 10000))
	{
		ERROR(sipform.T38_PORT, 'T38 Port out of range [5000-10000]');
		return false;
	}

	if (!isNumeric(sipform.V152_payload_type.value, 96, 127))
	{
		ERROR(sipform.V152_payload_type, 'T38 Port out of range [96-127]');
		return false;
	}

	if (isArray(document.all.sipPorts))
	{
		sip_ports = document.all.sipPorts;
		rtp_ports = document.all.rtpPorts;
		t38_ports = document.all.t38Ports;
	}
	else
	{
		sip_ports = new Array();
		sip_ports[0] = document.all.sipPorts;
		rtp_ports = new Array();
		rtp_ports[0] = document.all.rtpPorts;
		t38_ports = new Array();
		t38_ports[0] = document.all.t38Ports;
	}

	for (var i=0; i<sip_ports.length; i++)
	{
		var port, min, max;

		if (i == voipport) // the same page
		{
			// check sip
			max = sipport;
			min = parseInt(max) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range1 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(max) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range2 " + min + " ~ " + max);
				return false;
			}
			// check rtp
			max = parseInt(rtpport) + 3;
			min = parseInt(rtpport) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range3 " + min + " ~ " + max);
				return false;
			}
		}
		else 
		{
			// check sip
			port = sip_ports[i].value;
			if (sipport == port)
			{
				ERROR(sipform.sipPort, "The SIP port value couldn't be the same.");
				return false;
			}
			max = port;
			min = parseInt(max) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range4 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(max) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range5 " + min + " ~ " + max);
				return false;
			}
			// check rtp
			port = rtp_ports[i].value;
			max = parseInt(port) + 3;
			min = port;
			if (sipport >= min && sipport <= max)
			{
				ERROR(sipform.sipPort, "The SIP port value couldn't be the in the range6 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range7 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range8 " + min + " ~ " + max);
				return false;
			}
			// check t38
			port = t38_ports[i].value;
			max = parseInt(port) + 1;
			min = port;
			if (sipport >= min && sipport <= max)
			{
				ERROR(sipform.sipPort, "The SIP port value couldn't be the in the range9 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range10 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range11 " + min + " ~ " + max);
				return false;
			}
		}
	}

	if (sipform.CFAll[2].checked &&
		!reg_pstn.test(sipform.CFAll_No.value))
	{
		alert("PSTN must be chars of '0' ~ '9', '*', '#'");
		sipform.CFAll_No.select();
		sipform.CFAll_No.focus();
		return false;
	}

	if (updatePrecedence(sipform.precedence, sipform.preced_id) == 0)
	{
		alert('You must select one codec at least');
		return false;
	}

	if (typeof( sipform.g7111_pri ) != "undefined" && 
		typeof( sipform.g7111_modes ) != "undefined" &&
		updatePrecedence(sipform.g7111_pri, sipform.g7111_modes) == 0)
	{
		alert('You must select one g711-wb mode at least');
		return false;
	}

	if (chkPayloadType(sipform.dtmf_2833_pt) == false)
		return false;

	if (chkPayloadType(sipform.fax_modem_2833_pt) == false)
		return false;

	if (chkPayloadType(sipform.rtp_redundant_payload_type) == false)
		return false;

	if (chkPayloadType(sipform.V152_payload_type) == false)
		return false;

	if (checkDialPlanRegex(sipform.ReplaceRuleSource.value) == false) {
		alert('Replace rule can contains [0-9 x * # + n m] only.');
		return false;
	}
	
	if (checkDialPlanRegex(sipform.dialplan.value) == false) {
		alert('Dial plan can contains [0-9 x * # + n m] only.');
		return false;
	}	

	if (checkDialPlanRegex(sipform.PrefixUnsetPlan.value) == false) {
		alert('Prefix unset plan can contains [0-9 x * # + n m] only.');
		return false;
	}	

	if (checkDialPlanDigits(sipform.ReplaceRuleTarget.value) == false) {
		alert('To be replaced digits can contains [0-9 * #] only.');
		return false;
	}	

	if (checkDialPlanDigits(sipform.AutoPrefix.value) == false) {
		alert('Auto prefix can contains [0-9 * #] only.');
		return false;
	}	

	if (!isNumeric(sipform.dnd_from_hour.value, 0, 23))
	{
		alert('The hour should be between 0~23.');
		sipform.dnd_from_hour.select();
		sipform.dnd_from_hour.focus();
		return false;
	}

	if (!isNumeric(sipform.dnd_from_min.value, 0, 59))
	{
		alert('The minute should be between 0~59.');
		sipform.dnd_from_min.select();
		sipform.dnd_from_min.focus();
		return false;
	}

	if (!isNumeric(sipform.dnd_to_hour.value, 0, 23))
	{
		alert('The hour should be between 0~23.');
		sipform.dnd_to_hour.select();
		sipform.dnd_to_hour.focus();
		return false;
	}

	if (!isNumeric(sipform.dnd_to_min.value, 0, 59))
	{
		alert('The minute should be between 0~59.');
		sipform.dnd_to_min.select();
		sipform.dnd_to_min.focus();
		return false;
	}

	if (!isNumeric(sipform.alarm_hh.value, 0, 23))
	{
		alert('The alarm hour should be between 0~23.');
		sipform.alarm_hh.select();
		sipform.alarm_hh.focus();
		return false;
	}

	if (!isNumeric(sipform.alarm_mm.value, 0, 59))
	{
		alert('The alarm minute should be between 0~59.');
		sipform.alarm_mm.select();
		sipform.alarm_mm.focus();
		return false;
	}

	if (!checkPSTNRoutingPrefixRegex(sipform.PSTNRoutingPrefix.value))
	{
		alert("The PSTN routing prefix should be 0~9 and ','.");
		sipform.PSTNRoutingPrefix.select();
		sipform.PSTNRoutingPrefix.focus();
		return false;
	}	
	
	if (!isNumeric(sipform.T38MaxBuffer.value, 200, 600))
	{
		alert('The T.38 maximum buffer should be between 200~600.');
		sipform.T38MaxBuffer.select();
		sipform.T38MaxBuffer.focus();
		return false;
	}

	if (sipform.useRTCP.checked && !isNumeric(sipform.RTCPInterval.value, 5, 200))
	{
		alert('The RTCP interval should be between 5~200.');
		sipform.RTCPInterval.select();
		sipform.RTCPInterval.focus();
		return false;
	}

	sipform.submit();
}

function changeCountry() 
{ 
	if (document.all.tone_country.value == "17") 
	{ 
		document.all.tonetable.style.display="inline"; 
	} 
	else 
	{ 
		document.all.tonetable.style.display="none"; 
	} 
}

function changeNum()
{ 
	if (document.all.distone_num.value == "2") 
	{ 
		document.all.distone1tab.style.display="inline"; 
		document.all.distone2tab.style.display="inline"; 
	} 
	else if (document.all.distone_num.value == "1") 
	{ 
		document.all.distone1tab.style.display="inline"; 
		document.all.distone2tab.style.display="none"; 
	}
	else
	{
		document.all.distone1tab.style.display="none"; 
		document.all.distone2tab.style.display="none"; 
	}
}


function check_other()
{
	var reg=/\*[0-9]{1}/;

	if (!reg.test(document.other_form.funckey_transfer.value+";"))
	{
		alert('Function key must be 2 chars of the combination of * + 0~9');
		document.other_form.funckey_transfer.select();
		document.other_form.funckey_transfer.focus();
		return false;
	}

	if (!reg.test(document.other_form.funckey_pstn.value+";"))
	{
		alert('Function key must be 2 chars of the combination of * + 0~9');
		document.other_form.funckey_pstn.select();
		document.other_form.funckey_pstn.focus();
		return false;
	}	

	if (document.other_form.auto_dial.value != "0" && 
		!isNumeric(document.other_form.auto_dial.value, 3, 9))
	{
		alert('The auto dial time should be between 3~9.');
		document.other_form.auto_dial.select();
		document.other_form.auto_dial.focus();
		return false;
	}

	if (document.other_form.auto_dial.value == "0" &&
		document.other_form.auto_dial_always.checked)
	{
		alert('The auto dial time should be set if disable dial-out by Hash key.');
		document.other_form.auto_dial.select();
		document.other_form.auto_dial.focus();
		return false;
	}

	if (document.other_form.off_hook_alarm.value != "0" && 
		!isNumeric(document.other_form.off_hook_alarm.value, 10, 60))
	{
		alert('The off-hook alarm time should be between 10~60.');
		document.other_form.off_hook_alarm.select();
		document.other_form.off_hook_alarm.focus();
		return false;
	}
	return true;
}

function check_network()
{
	/*
		if(document.net_form.vlan_enable.checked){
        if (!isNumeric(document.net_form.vlan_nat_tag.value, 1, 4094))
        {
        	alert('The VLAN ID should be between 1~4094.');
                document.net_form.vlan_nat_tag.select();
                document.net_form.vlan_nat_tag.focus();
                return false;
        }
	        if (document.net_form.vlan_nat_tag.value ==11)
        {
	        		alert('VLAN ID 11 is reserved for other purpose');
	                document.net_form.vlan_nat_tag.select();
	                document.net_form.vlan_nat_tag.focus();
                return false;
        }

		    if (document.net_form.sw_vlan_enable.checked)
		    {
		        	if(!isNumeric(document.net_form.sw_vlan_mgdata.value, 1, 4094))
		        	{
		        		alert('The VLAN ID should be between 1~4094.');
		                document.net_form.sw_vlan_mgdata.select();
		                document.net_form.sw_vlan_mgdata.focus();
		                return false;
		        	}
		        	if(document.net_form.sw_vlan_mgdata.value ==11)
		        	{
		        		alert('VLAN ID 11 is reserved for other purpose');
		                document.net_form.sw_vlan_mgdata.select();
		                document.net_form.sw_vlan_mgdata.focus();
		                return false;
		        	}
		        	if(!isNumeric(document.net_form.sw_vlan_mgdata_pri.value, 0, 7))
		        	{
		        		alert('The priority should be between 0~7.');
		                document.net_form.sw_vlan_mgdata_pri.select();
		                document.net_form.sw_vlan_mgdata_pri.focus();
		                return false;
		        	}
		        	if(!isNumeric(document.net_form.sw_vlan_wifi.value, 1, 4094))
		        	{
		        		alert('The VLAN ID should be between 1~4094.');
		                document.net_form.sw_vlan_wifi.select();
		                document.net_form.sw_vlan_wifi.focus();
		                return false;
		        	}
		        	if(document.net_form.sw_vlan_wifi.value ==11)
		        	{
		        		alert('VLAN ID 11 is reserved for other purpose');
		                document.net_form.sw_vlan_wifi.select();
		                document.net_form.sw_vlan_wifi.focus();
		                return false;
		        	}
		        	if(!isNumeric(document.net_form.sw_vlan_wifi_pri.value, 0, 7))
		        	{
		        		alert('The priority should be between 0~7.');
		                document.net_form.sw_vlan_wifi_pri.select();
		                document.net_form.sw_vlan_wifi_pri.focus();
		                return false;
		        	}
		        		if(!isNumeric(document.net_form.sw_vlan_voice.value, 1, 4094))
		        	{
		        		alert('The VLAN ID should be between 1~4094.');
		                document.net_form.sw_vlan_voice.select();
		                document.net_form.sw_vlan_voice.focus();
		                return false;
		        	}
		        	if(document.net_form.sw_vlan_voice.value ==11)
		        	{
		        		alert('VLAN ID 11 is reserved for other purpose');
		                document.net_form.sw_vlan_voice.select();
		                document.net_form.sw_vlan_voice.focus();
		                return false;
		        	}
		        	if(!isNumeric(document.net_form.sw_vlan_voice_pri.value, 0, 7))
		        	{
		        		alert('The priority should be between 0~7.');
		                document.net_form.sw_vlan_voice_pri.select();
		                document.net_form.sw_vlan_voice_pri.focus();
		                return false;
		        	}
		        	
		     }
		     
		}//nat tag
      */ 

	// VLAN for Voice
        if (!isNumeric(document.net_form.wanVlanIdVoice.value, 1, 4090))
	{
                alert('The VLAN ID should be between 1~4090.');
		document.net_form.wanVlanIdVoice.select();
		document.net_form.wanVlanIdVoice.focus();
		return false;	
	}

	if (!isNumeric(document.net_form.wanVlanPriorityVoice.value, 0, 7))
	{
		alert('The User Priority should be between 0~7.');
		document.net_form.wanVlanPriorityVoice.select();
		document.net_form.wanVlanPriorityVoice.focus();
		return false;	
	}

	if (!isNumeric(document.net_form.wanVlanCfiVoice.value, 0, 1))
	{
		alert('The CFI should be 0 or 1. (0 for Ethernet)');
		document.net_form.wanVlanCfiVoice.select();
		document.net_form.wanVlanCfiVoice.focus();
		return false;	
	}

	// VLAN for Data
        if (!isNumeric(document.net_form.wanVlanIdData.value, 1, 4090))
	{
                alert('The VLAN ID should be between 1~4090.');
		document.net_form.wanVlanIdData.select();
		document.net_form.wanVlanIdData.focus();
		return false;	
	}

	if (!isNumeric(document.net_form.wanVlanPriorityData.value, 0, 7))
	{
		alert('The User Priority should be between 0~7.');
		document.net_form.wanVlanPriorityData.select();
		document.net_form.wanVlanPriorityData.focus();
		return false;	
	}

	if (!isNumeric(document.net_form.wanVlanCfiData.value, 0, 1))
	{
		alert('The CFI should be 0 or 1. (0 for Ethernet)');
		document.net_form.wanVlanCfiData.select();
		document.net_form.wanVlanCfiData.focus();
		return false;	
	}

	// VLAN for Video
        if (!isNumeric(document.net_form.wanVlanIdVideo.value, 1, 4090))
	{
                alert('The VLAN ID should be between 1~4090.');
		document.net_form.wanVlanIdVideo.select();
		document.net_form.wanVlanIdVideo.focus();
		return false;	
	}

	if (!isNumeric(document.net_form.wanVlanPriorityVideo.value, 0, 7))
	{
		alert('The User Priority should be between 0~7.');
		document.net_form.wanVlanPriorityVideo.select();
		document.net_form.wanVlanPriorityVideo.focus();
		return false;	
	}

	if (!isNumeric(document.net_form.wanVlanCfiVideo.value, 0, 1))
	{
		alert('The CFI should be 0 or 1. (0 for Ethernet)');
		document.net_form.wanVlanCfiVideo.select();
		document.net_form.wanVlanCfiVideo.focus();
		return false;	
	}
	if (!isNumeric(document.net_form.LANPort0_Bandwidth_out.value, 0, 16383))
	{
		alert('The Egress Bandwidth should be be between 0~16383 )');
		document.net_form.LANPort0_Bandwidth_out.select();
		document.net_form.LANPort0_Bandwidth_out.focus();
		return false;
	}
	if (!isNumeric(document.net_form.LANPort1_Bandwidth_out.value, 0, 16383))
	{
		alert('The Egress Bandwidth should be be between 0~16383 )');
		document.net_form.LANPort1_Bandwidth_out.select();
		document.net_form.LANPort1_Bandwidth_out.focus();
		return false;
	}
	if (!isNumeric(document.net_form.LANPort2_Bandwidth_out.value, 0, 16383))
	{
		alert('The Egress Bandwidth should be be between 0~16383 )');
		document.net_form.LANPort2_Bandwidth_out.select();
		document.net_form.LANPort2_Bandwidth_out.focus();
		return false;
	}
	if (!isNumeric(document.net_form.LANPort3_Bandwidth_out.value, 0, 16383))
	{
		alert('The Egress Bandwidth should be be between 0~16383 )');
		document.net_form.LANPort3_Bandwidth_out.select();
		document.net_form.LANPort3_Bandwidth_out.focus();
		return false;
	}
	if (!isNumeric(document.net_form.WANPort_Bandwidth_out.value, 0, 16383))
	{
		alert('The Egress Bandwidth should be be between 0~16383 )');
		document.net_form.WANPort_Bandwidth_out.select();
		document.net_form.WANPort_Bandwidth_out.focus();
		return false;
	}
	if (!isNumeric(document.net_form.LANPort0_Bandwidth_in.value, 0, 65535))
	{
		alert('The Ingress Bandwidth should be be between 0~65535 )');
		document.net_form.LANPort0_Bandwidth_in.select();
		document.net_form.LANPort0_Bandwidth_in.focus();
		return false;
	}
	if (!isNumeric(document.net_form.LANPort1_Bandwidth_in.value, 0, 65535))
	{
		alert('The Ingress Bandwidth should be be between 0~65535 )');
		document.net_form.LANPort1_Bandwidth_in.select();
		document.net_form.LANPort1_Bandwidth_in.focus();
		return false;
	}
	if (!isNumeric(document.net_form.LANPort2_Bandwidth_in.value, 0, 65535))
	{
		alert('The Ingress Bandwidth should be be between 0~65535 )');
		document.net_form.LANPort2_Bandwidth_in.select();
		document.net_form.LANPort2_Bandwidth_in.focus();
		return false;
	}
	if (!isNumeric(document.net_form.LANPort3_Bandwidth_in.value, 0, 65535))
	{
		alert('The Ingress Bandwidth should be be between 0~65535 )');
		document.net_form.LANPort3_Bandwidth_in.select();
		document.net_form.LANPort3_Bandwidth_in.focus();
		return false;
	}
	if (!isNumeric(document.net_form.WANPort_Bandwidth_in.value, 0, 65535))
	{
		alert('The Ingress Bandwidth should be be between 0~65535 )');
		document.net_form.WANPort_Bandwidth_in.select();
		document.net_form.WANPort_Bandwidth_in.focus();
		return false;
	}

	return true;
}

function enable_callwaiting()
{
	document.sipform.call_waiting_cid.disabled = 
		!document.sipform.call_waiting.checked;
}

function enable_cid_det_mode()
{
	document.other_form.caller_id_det.disabled = 
		!document.other_form.caller_id_auto_det[0].checked;
}

function enableWanVlan()
{
	var checked = document.net_form.wanVlanEnable.checked;
	document.net_form.wanVlanIdVoice.disabled=!checked;
	document.net_form.wanVlanPriorityVoice.disabled=!checked;
	document.net_form.wanVlanCfiVoice.disabled=!checked;
	document.net_form.wanVlanIdData.disabled=!checked;
	document.net_form.wanVlanPriorityData.disabled=!checked;
	document.net_form.wanVlanCfiData.disabled=!checked;
	document.net_form.wanVlanIdVideo.disabled=!checked;
	document.net_form.wanVlanPriorityVideo.disabled=!checked;
	document.net_form.wanVlanCfiVideo.disabled=!checked;
}
function enableVlan()
{
        var checked = document.net_form.vlan_enable.checked;
        if(checked == false)
       	{
       		document.net_form.vlan_tag.checked  = false;
    		document.net_form.vlan_host_enable.checked = false;
       		document.net_form.vlan_wifi_enable.checked = false;
       		document.net_form.vlan_wifi_vap0_enable.checked = false;
       		document.net_form.vlan_wifi_vap1_enable.checked = false;
       		document.net_form.vlan_wifi_vap2_enable.checked = false;
       		document.net_form.vlan_wifi_vap3_enable.checked = false;
       		
       		document.net_form.vlan_bridge_enable.checked = false;
       	}
       	
       	document.net_form.vlan_tag.disabled  			= !checked;
    	document.net_form.vlan_host_enable.disabled 	= !checked;
    	document.net_form.vlan_host_tag.disabled     	=!(document.net_form.vlan_host_enable.checked);
		document.net_form.vlan_host_pri.disabled 		=!(document.net_form.vlan_host_enable.checked);
    	document.net_form.vlan_wifi_enable.disabled 	=!checked;
    	document.net_form.vlan_wifi_tag.disabled     	=!(document.net_form.vlan_wifi_enable.checked);
		document.net_form.vlan_wifi_pri.disabled 		=!(document.net_form.vlan_wifi_enable.checked);
    	document.net_form.vlan_wifi_vap0_enable.disabled=!checked;
    	document.net_form.vlan_wifi_vap0_tag.disabled   =!(document.net_form.vlan_wifi_vap0_enable.checked);
		document.net_form.vlan_wifi_vap0_pri.disabled 	=!(document.net_form.vlan_wifi_vap0_enable.checked);
    	document.net_form.vlan_wifi_vap1_enable.disabled=!checked;
    	document.net_form.vlan_wifi_vap1_tag.disabled   =!(document.net_form.vlan_wifi_vap1_enable.checked);
		document.net_form.vlan_wifi_vap1_pri.disabled 	=!(document.net_form.vlan_wifi_vap1_enable.checked);
    	document.net_form.vlan_wifi_vap2_enable.disabled=!checked;
    	document.net_form.vlan_wifi_vap2_tag.disabled   =!(document.net_form.vlan_wifi_vap2_enable.checked);
		document.net_form.vlan_wifi_vap2_pri.disabled 	=!(document.net_form.vlan_wifi_vap2_enable.checked);
    	document.net_form.vlan_wifi_vap3_enable.disabled=!checked; 
    	document.net_form.vlan_wifi_vap3_tag.disabled   =!(document.net_form.vlan_wifi_vap3_enable.checked);
		document.net_form.vlan_wifi_vap3_pri.disabled 	=!(document.net_form.vlan_wifi_vap3_enable.checked);
		
		document.net_form.vlan_bridge_enable.disabled  	  =!checked;
		document.net_form.vlan_bridge_tag.disabled  	  =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_0.disabled     =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_1.disabled     =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_2.disabled     =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_3.disabled     =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_wifi.disabled  =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_vap0.disabled  =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_vap1.disabled  =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_vap2.disabled  =!(document.net_form.vlan_bridge_enable.checked);
		document.net_form.vlan_bridge_port_vap3.disabled  =!(document.net_form.vlan_bridge_enable.checked);
		
		if(document.net_form.vlan_bridge_enable.checked==false)
			document.net_form.vlan_bridge_multicast_enable.checked = false;
			
		document.net_form.vlan_bridge_multicast_enable.disabled   =!(document.net_form.vlan_bridge_enable.checked);;
		document.net_form.vlan_bridge_multicast_tag.disabled  	  =!(document.net_form.vlan_bridge_multicast_enable.checked);
		
}

function check_fwupdate()
{
	if (!document.fw_form.fw_mode[0].checked &&
		!isNumeric(document.fw_form.fw_day.value, 1, 30))
	{
                alert('The Scheduling Day should be between 1~30.');
		document.fw_form.fw_day.select();
		document.fw_form.fw_day.focus();
		return false;
	}
}
function enable_fw()
{
	document.fw_form.fw_tftp_addr.disabled =!document.fw_form.fw_mode[1].checked;

	document.fw_form.fw_http_addr.disabled= !document.fw_form.fw_mode[3].checked;
	document.fw_form.fw_http_port.disabled= !document.fw_form.fw_mode[3].checked;

	document.fw_form.fw_ftp_addr.disabled= !document.fw_form.fw_mode[2].checked;
	document.fw_form.fw_ftp_user.disabled= !document.fw_form.fw_mode[2].checked;
	document.fw_form.fw_ftp_passwd.disabled= !document.fw_form.fw_mode[2].checked;

	document.fw_form.fw_file_path.disabled= document.fw_form.fw_mode[0].checked;
	document.fw_form.fw_file_prefix.disabled= document.fw_form.fw_mode[0].checked;
	document.fw_form.fw_day.disabled= document.fw_form.fw_mode[0].checked;
	document.fw_form.fw_time.disabled= document.fw_form.fw_mode[0].checked;
}

function enable_config()
{
	document.config_form.http_addr.disabled =!document.config_form.mode[1].checked;
	document.config_form.http_port.disabled =!document.config_form.mode[1].checked;
	document.config_form.tftp_addr.disabled =!document.config_form.mode[2].checked;

	document.config_form.ftp_addr.disabled =!document.config_form.mode[3].checked;
	document.config_form.ftp_passwd.disabled =!document.config_form.mode[3].checked;
	document.config_form.ftp_user.disabled =!document.config_form.mode[3].checked;

	document.config_form.file_path.disabled =document.config_form.mode[0].checked;
	document.config_form.expire.disabled =document.config_form.mode[0].checked;

}

