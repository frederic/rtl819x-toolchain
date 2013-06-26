<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>Config</title>
<script language="javascript" src=voip_script.js></script>
<script language="javascript">
function resetClick()
{
	if ( !confirm('Do you really want to reset the VoIP settings to default?') ) 
		return false;
	else
		return true;
}

function verifyBrowser() 
{
	var ms = navigator.appVersion.indexOf("MSIE");
	ie4 = (ms>0) && (parseInt(navigator.appVersion.substring(ms+5, ms+6)) >= 4);
	var ns = navigator.appName.indexOf("Netscape");
	ns= (ns>=0) && (parseInt(navigator.appVersion.substring(0,1))>=4);
	if (ie4)
	{
		return "ie4";
	}
	else
	{
		if(ns)
			return "ns";
		else
			return false;
	}
}

function saveClick(url)
{
	if (verifyBrowser() == "ie4") 
	{
		window.location.href = url;
		return false;
	}
	else
		return true;
}

function InitConfig()
{
	enable_fw();
	enable_config();
}

</script>
</head>
<body bgcolor="#ffffff" text="#000000" onload="InitConfig()" >

<p>
<b>VoIP Save/Reload Setting</b>
<table cellSpacing=1 cellPadding=2 border=0 width=375>
<form method="post" action=/goform/voip_config_set name="setting_save_form">
<tr>
   	<td bgColor=#aaddff width=150>Save Settings to File</td>
	<td bgColor=#ddeeff>
		<input type="submit" value="Save..." name="setting_save" onclick="return saveClick('/config_voip.dat')">
	</td>
</tr>
</form>
<form method="post" action=/goform/voip_config_set enctype="multipart/form-data" name="setting_load_form">
<tr>
   	<td bgColor=#aaddff width=150>Load Settings from File</td>
	<td bgColor=#ddeeff>
		<input type="file" name="binary" size=24>
	    <input type="submit" value="Upload" name="setting_load">
	</td>
</tr>
</form>
<form method="post" action="/goform/voip_config_set" name="setting_reset_form">
<tr>
   	<td bgColor=#aaddff width=150>Reset Settings to Default</td>
	<td bgColor=#ddeeff>
	    <input type="submit" value="Reset" name="setting_reset" onclick="return resetClick()">
	</td>
</tr>
</form>
</table>
<p>

<b>Auto Config</b>
<table cellSpacing=1 cellPadding=2 border=0 width=500>
<form method="get" action="/goform/voip_config_set" name=config_form>
<tr>
   	<td bgColor=#aaddff width=150>Mode</td>
	<td bgColor=#ddeeff>
		<input type=radio name=mode value=0 onclick="enable_config()"<%voip_config_get("mode_disable");%>> Disable
		<input type=radio name=mode value=1 onclick="enable_config()"<%voip_config_get("mode_http");%>> HTTP
<!-- added by Jack for auto provision -->
		<input type=radio name=mode value=2 onclick="enable_config()"<%voip_config_get("mode_tftp");%>> TFTP
		<input type=radio name=mode value=3 onclick="enable_config()"<%voip_config_get("mode_ftp");%>> FTP
<!-- end -->
		
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>HTTP Server Address</td>
	<td bgColor=#ddeeff>
		<input type=text name=http_addr size=20 maxlength=39 value="<%voip_config_get("http_addr");%>">
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>HTTP Server Port</td>
	<td bgColor=#ddeeff>
		<input type=text name=http_port size=10 maxlength=5 value="<%voip_config_get("http_port");%>">
	</td>
</tr>

<!-- added by Jack for auto provision -->
<tr>
		<td bgColor=#aaddff>TFTP Server Address</td>
	<td bgColor=#ddeeff>
		<input type=text name=tftp_addr size=20 maxlength=39 value="<%voip_config_get("tftp_addr");%>">
	</td>
</tr>
<tr>
		<td bgColor=#aaddff>FTP Server Address</td>
	<td bgColor=#ddeeff>
		<input type=text name=ftp_addr size=20 maxlength=39 value="<%voip_config_get("ftp_addr");%>">
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>FTP Username</td>
	<td bgColor=#ddeeff>
		<input type=text name=ftp_user size=10 maxlength=20 value="<%voip_config_get("ftp_user");%>">
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>FTP Password</td>
	<td bgColor=#ddeeff>
		<input type=password name=ftp_passwd size=10 maxlength=20 value="<%voip_config_get("ftp_passwd");%>">
	</td>
</tr>
<!-- End -->
<tr>
   	<td bgColor=#aaddff>File Path</td>
	<td bgColor=#ddeeff>
		<input type=text name=file_path size=10 maxlength=60 value="<%voip_config_get("file_path");%>">
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>Expire Time</td>
	<td bgColor=#ddeeff>
		<input type=text name=expire size=10 maxlength=5 value="<%voip_config_get("expire");%>"> days
	</td>
</tr>
<tr>
	<td bgColor=#aaddff>Autoconfig Version</td>
	<td bgColor=#ddeeff><%voip_config_get("autoconfig_version");%></td>
</tr>
<tr>
	<td colspan=2 align=center>
		<input type="submit" value="Apply Changes" name="config_apply">
   		&nbsp;&nbsp;&nbsp;&nbsp;    	
		<input type="reset" value="Reset">
	</td>
</tr>
</form>
</table>

<b>Auto Firmware Update</b>
<table cellSpacing=1 cellPadding=2 border=0 width=500>
<form method="get" action="/goform/voip_config_set" name=fw_form>

<tr>
   	<td bgColor=#aaddff width=150>Mode</td>
	<td bgColor=#ddeeff>
	<input type=radio name=fw_mode value=0 onclick="enable_fw()"<%voip_fwupdate_get("fw_mode_off");%>> Off
	<input type=radio name=fw_mode value=1 onclick="enable_fw()"<%voip_fwupdate_get("fw_mode_tftp");%>> TFTP
	<input type=radio name=fw_mode value=2 onclick="enable_fw()"<%voip_fwupdate_get("fw_mode_ftp");%>> FTP
	<input type=radio name=fw_mode value=3 onclick="enable_fw()"<%voip_fwupdate_get("fw_mode_http");%>> HTTP
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>TFTP Server Address</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_tftp_addr size=20 maxlength=60 value="<%voip_fwupdate_get("fw_tftp_addr");%>"> Exp. 60.35.187.30
	</td>
</tr>	 
<tr>
   	<td bgColor=#aaddff>HTTP Server Address</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_http_addr size=20 maxlength=60 value="<%voip_fwupdate_get("fw_http_addr");%>"> Exp. 60.35.17.1
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>HTTP Server Port</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_http_port size=5 maxlength=5 value="<%voip_fwupdate_get("fw_http_port");%>">
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>FTP Server Address</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_ftp_addr size=20 maxlength=60 value="<%voip_fwupdate_get("fw_ftp_addr");%>"> Exp. 60.35.20.5
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>FTP Username</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_ftp_user size=20 maxlength=60 value="<%voip_fwupdate_get("fw_ftp_user");%>">
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>FTP Password</td>
	<td bgColor=#ddeeff>
		<input type=password name=fw_ftp_passwd size=20 maxlength=60 value="<%voip_fwupdate_get("fw_ftp_passwd");%>">
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>File Path</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_file_path size=20 maxlength=60 value="<%voip_fwupdate_get("fw_file_path");%>"> Exp. auto
	</td>
</tr> 
<tr>
   	<td bgColor=#aaddff>Check new firmware</td>
	<td bgColor=#ddeeff>
		<input type=radio name=fw_power_on value=0 <%voip_fwupdate_get("fw_power_on");%>> Power On
		<input type=radio name=fw_power_on value=1 <%voip_fwupdate_get("fw_scheduling");%>> Scheduling
		<input type=radio name=fw_power_on value=2 <%voip_fwupdate_get("fw_both");%>> Both	
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Scheduling Day</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_day size=5 maxlength=10 value="<%voip_fwupdate_get("fw_day");%>"> (1~ 30 days)
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=155>Scheduling Time</td>
   	<td bgColor=#ddeeff width=170>
		<select name=fw_time>
			 <%voip_fwupdate_get("fw_time");%>
		</select>
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Auto Update</td>
   	<td bgColor=#ddeeff>
		<input type=radio name=fw_auto_mode value=1 <%voip_fwupdate_get("fw_auto");%>> Automatic
		<input type=radio name=fw_auto_mode value=0 <%voip_fwupdate_get("fw_notify");%>> Notify Only
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>File Prefix</td>
	<td bgColor=#ddeeff>
		<input type=text name=fw_file_prefix size=20 maxlength=60 value="<%voip_fwupdate_get("fw_file_prefix");%>">
	</td>
</tr> 

<tr>
   	<td bgColor=#aaddff width=155>Next Update Time</td>
   	<td bgColor=#ddeeff width=300><%voip_fwupdate_get("fw_next_time");%></td>
</tr>

<tr>
   	<td bgColor=#aaddff width=155>Firmware Version</td>
   	<td bgColor=#ddeeff width=300><%voip_fwupdate_get("fw_version");%></td>
</tr>

<tr>
	<td colspan=2 align=center>
		<input type="submit" value="Apply Changes" name="fw_apply" onclick="return check_fwupdate()">
   		&nbsp;&nbsp;&nbsp;&nbsp;    	
		<input type="reset" value="Reset">
	</td>
	
</tr>
</form>
</table>
</body>
</html>
