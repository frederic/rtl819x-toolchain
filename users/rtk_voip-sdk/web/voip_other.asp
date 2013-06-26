<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
<script language="javascript">
<!--
function enableAutoBypass()
{
	document.other_form.auto_bypass_warning.disabled =
		!document.other_form.auto_bypass_relay.checked;
}
function InitOther()
{
	enableAutoBypass();
}
-->
</script>
</head>
<body bgcolor="#ffffff" text="#000000" onload="InitOther()">
<form method="get" action="/goform/voip_other_set" name=other_form>

<%voip_general_get("not_ipphone_option_start");%>
<b>Function Key</b>
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("not_ipphone_table");%> >
<tr bgColor=#888888>
	<td colspan=2>
	<font color=#ffffff>
	Must be * + 0~9
	</font>
	</td>
</tr>	
<tr <%voip_other_get("display_funckey_pstn");%>>
   	<td bgColor=#aaddff width=150>Switch to PSTN</td>
	<td bgColor=#ddeeff>
		<input type=text name=funckey_pstn size=5 maxlength=2 value="<%voip_other_get("funckey_pstn");%>">
		( default: *0 )
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>Call Transfer</td>
	<td bgColor=#ddeeff>
		<input type=text name=funckey_transfer size=5 maxlength=2 value="<%voip_other_get("funckey_transfer");%>">
		( default: *1 )
	</td>
</tr>	
</table>

<%voip_other_get("auto_dial_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_other_get("auto_dial_display");%>>
<tr>
   	<td bgColor=#aaddff width=150>Auto Dial Time</td>
	<td bgColor=#ddeeff>
		<input type=text name=auto_dial size=3 maxlength=1 value="<%voip_other_get("auto_dial");%>">
		( 3~9 sec, 0 is disable )
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>Dial-out by Hash Key</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=auto_dial_always <%voip_other_get("auto_dial_always");%>>Disabled
	</td>
</tr>
</table>

<p <%voip_other_get("display_funckey_pstn");%>>
<b>PSTN Relay<b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr>
   	<td bgColor=#aaddff width=150>Auto bypass relay</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=auto_bypass_relay onclick=enableAutoBypass() <%voip_other_get("auto_bypass_relay");%>>Enable
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>Warning tone</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=auto_bypass_warning <%voip_other_get("auto_bypass_warning");%>>Enable
	</td>
</tr>
</table>
</p>

<%voip_other_get("off_hook_alarm_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_other_get("off_hook_alarm_display");%>>
<tr>
   	<td bgColor=#aaddff width=150>Off-Hook Alarm Time</td>
	<td bgColor=#ddeeff>
		<input type=text name=off_hook_alarm size=3 maxlength=2 value="<%voip_other_get("off_hook_alarm");%>">
		( 10~60 sec, 0 is disable )
	</td>
</tr>
</table>

<p <%voip_other_get("display_cid_det");%>>
<b>VoIP to PSTN</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr bgColor=#888888>
	<td bgColor=#aaddff width=150>One Stage Dialing</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=one_stage_dial <%voip_other_get("one_stage_dial");%>>Enable
	</td>
</tr>
<tr bgColor=#888888>
   	<td bgColor=#aaddff width=150>Two Stage Dialing</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=two_stage_dial <%voip_other_get("two_stage_dial");%>>Enable
	</td>
</tr>	
</table>
</p>

<p <%voip_other_get("display_cid_det");%>>
<b>PSTN to VoIP</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr bgColor=#888888>
	<td bgColor=#aaddff width=150>Caller ID Auto Detection</td>
	<td bgColor=#ddeeff><%voip_other_get("caller_id_auto_det");%></td>
	<input type=hidden name=caller_id_test value=hello>
</tr>	
<tr bgColor=#888888>
	<td bgColor=#aaddff width=150>Caller ID Detection Mode</td>
	<td bgColor=#ddeeff><%voip_other_get("caller_id_det");%></td>
</tr>	
</table>
</p>

<%voip_general_get("not_ipphone_option_start");%>
<p>
<b>FXS Pulse Dial Detection</b>	
<%voip_general_get("not_ipphone_option_end");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("not_ipphone_table");%> >
	<tr>
	<td bgColor=#aaddff> 
		<%voip_other_get("pulse_dial_detection");%>
	</td>	
	</tr>
	
	<tr>
   	<td bgColor=#aaddff width=150>Interdigit Pause Duration</td>
	<td bgColor=#ddeeff>
		<input type=text name=pulse_det_Pause size=5 maxlength=4 value="<%voip_other_get("pulse_det_Pause");%>">
		(msec)
	</td>	
	</tr>
	
</table>
</p>


<p <%voip_other_get("display_pulse_dial_gen");%>>
<b>FXO Pulse Dial Generation</b>	
<table cellSpacing=1 cellPadding=2 border=0 width=450>

	<tr>
	<td bgColor=#aaddff> 
		<%voip_other_get("pulse_dial_generation");%>
	</td>	
	</tr>
	
	<tr>
	<td bgColor=#aaddff width=150>Pulse Per Second</td>
	<td bgColor=#ddeeff> 
		<select name=pulse_gen_PPS>
		<%voip_other_get("pulse_gen_PPS");%>
		</select>
	</td>	
	</tr>
	
   	<tr>
    	<td bgColor=#aaddff rowspan=2>Make Duration</td>
    	<td bgColor=#ddeeff>
		<input type=text name=pulse_gen_Make size=5 maxlength=2 value="<%voip_other_get("pulse_gen_Make");%>">
		(msec)
	</td>

	</tr>
	
	<tr>
		<td bgColor=#ddeeff><font size=2>1~99 msec for 10PPS, 1~49 msec for 20PPS</font>
		</td>	
	</tr>
	
	
	<tr>
   	<td bgColor=#aaddff width=150>Interdigit Pause Duration</td>
	<td bgColor=#ddeeff>
		<input type=text name=pulse_gen_Pause size=5 maxlength=4 value="<%voip_other_get("pulse_gen_Pause");%>">
		(msec)
	</td>	
	</tr>
	
</table>
</p>


<%voip_general_get("not_ipphone_option_start");%>
<table cellSpacing=1 cellPadding=2 border=0 width=375>
<tr>
   	<td colspan=3 align=center>
   		<input type="submit" value="Apply" onclick="return check_other()">
   		&nbsp;&nbsp;&nbsp;&nbsp;    	
   		<input type="reset" value="Reset">	
   	</td>
</tr>
</table>
<%voip_general_get("not_ipphone_option_end");%>



</form>
</body>
</html>
