<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
<script language="javascript">
<!--
function InitOther()
{
        enableVlan();
}
-->
</script>

</head>
<body bgcolor="#ffffff" text="#000000" onload="InitOther()">
<form method="get" action="/goform/voip_net_set" name=net_form>

<p>
<b>DSCP Flag</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
    	<tr>
    	<td bgColor=#aaddff>SIP DSCP</td>
		<td bgColor=#ddeeff>
		<select name=sipDscp>
			 "<%voip_net_get("sipDscp");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
	<tr>
    	<td bgColor=#aaddff>RTP DSCP</td>
		<td bgColor=#ddeeff>
		<select name=rtpDscp>
                         "<%voip_net_get("rtpDscp");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
</table>

<p <%voip_net_get("display_wanVlan");%>>
<b>VLAN Tags for Voice, Data and Video.</b>
<table cellSpacing=1 cellPadding=1 border=0 width=450>

<tr>
	<td bgColor=#aaddff width=155>Use VLAN Tags</td>
	<td bgColor=#ddeeff width=170>
                <input type=checkbox name=wanVlanEnable onClick="enableWanVlan();" <%voip_net_get("wanVlanEnable");%>>Enable
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Voice: VLAN ID</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanIdVoice size=5 maxlength=4 value="<%voip_net_get("wanVlanIdVoice");%>">
                ( 1~4090 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Voice: User Priority</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanPriorityVoice size=5 maxlength=1 value="<%voip_net_get("wanVlanPriorityVoice");%>">
		( 0~7 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Voice: CFI</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanCfiVoice size=5 maxlength=1 value="<%voip_net_get("wanVlanCfiVoice");%>">
		( 0~1 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Data: VLAN ID</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanIdData size=5 maxlength=4 value="<%voip_net_get("wanVlanIdData");%>">
                ( 1~4090 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Data: User Priority</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanPriorityData size=5 maxlength=1 value="<%voip_net_get("wanVlanPriorityData");%>">
		( 0~7 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Data: CFI</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanCfiData size=5 maxlength=1 value="<%voip_net_get("wanVlanCfiData");%>">
		( 0~1 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Video: VLAN ID</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanIdVideo size=5 maxlength=4 value="<%voip_net_get("wanVlanIdVideo");%>">
                ( 1~4090 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Video: User Priority</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanPriorityVideo size=5 maxlength=1 value="<%voip_net_get("wanVlanPriorityVideo");%>">
		( 0~7 )
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Video: CFI</td>
	<td bgColor=#ddeeff>
                <input type=text name=wanVlanCfiVideo size=5 maxlength=1 value="<%voip_net_get("wanVlanCfiVideo");%>">
		( 0~1 )
	</td>
</tr>
</table>
<td>Note I:  The Video port is near the WAN port. </td>
<td>Note II: Only support in HWNAT-disabled Mode. </td>


<p <%voip_net_get("display_vlan_wan_tag");%>>
<b>Add VLAN Tags for WAN packet</b>

<td width=200>
<input type=checkbox name=vlan_enable onClick="enableVlan();" <%voip_net_get("vlan_enable");%>>Enable tag for WAN packets
</td>



<table cellSpacing=1 cellPadding=1 border=0 width=500>
<tr>
<td bgColor=#aaddff width=200>For ALL Packets</td>
<td bgColor=#ddeeff width=150>
     VLAN ID <input type=text name=vlan_tag size=4 maxlength=4 value="<%voip_net_get("vlan_tag");%>"> 
</td>
</tr>

<tr>
        <td bgColor=#aaddff width=200>
        <input type=checkbox name=vlan_host_enable onClick="enableVlan();" <%voip_net_get("vlan_host_enable");%>>For HOST Packets
       </td>
        <td bgColor=#ddeeff width=150>
               VLAN ID <input type=text name=vlan_host_tag size=4 maxlength=4 value="<%voip_net_get("vlan_host_tag");%>"> 
        </td>
        <td bgColor=#ddeeff width=150>
               Priority <input type=text name=vlan_host_pri size=1 maxlength=1 value="<%voip_net_get("vlan_host_pri");%>"> ( 0~7 )
        </td>
        
</tr>
<tr>
        <td bgColor=#aaddff width=200>
        <input type=checkbox name=vlan_wifi_enable onClick="enableVlan();" <%voip_net_get("vlan_wifi_enable");%>>For Wifi Root Packets
       </td>
        <td bgColor=#ddeeff width=150>
               VLAN ID <input type=text name=vlan_wifi_tag size=4 maxlength=4 value="<%voip_net_get("vlan_wifi_tag");%>"> 
        </td>
        <td bgColor=#ddeeff width=150>
               Priority <input type=text name=vlan_wifi_pri size=1 maxlength=1 value="<%voip_net_get("vlan_wifi_pri");%>"> ( 0~7 )
        </td>
        
</tr>
<tr>
        <td bgColor=#aaddff width=200>
        <input type=checkbox name=vlan_wifi_vap0_enable onClick="enableVlan();" <%voip_net_get("vlan_wifi_vap0_enable");%>>For Wifi VAP1 Packets
       </td>
        <td bgColor=#ddeeff width=150>
               VLAN ID <input type=text name=vlan_wifi_vap0_tag size=4 maxlength=4 value="<%voip_net_get("vlan_wifi_vap0_tag");%>"> 
        </td>
        <td bgColor=#ddeeff width=150>
               Priority <input type=text name=vlan_wifi_vap0_pri size=1 maxlength=1 value="<%voip_net_get("vlan_wifi_vap0_pri");%>"> ( 0~7 )
        </td>
        
</tr>
<tr>
        <td bgColor=#aaddff width=200>
        <input type=checkbox name=vlan_wifi_vap1_enable onClick="enableVlan();" <%voip_net_get("vlan_wifi_vap1_enable");%>>For Wifi VAP2 Packets
       </td>
        <td bgColor=#ddeeff width=150>
               VLAN ID <input type=text name=vlan_wifi_vap1_tag size=4 maxlength=4 value="<%voip_net_get("vlan_wifi_vap1_tag");%>"> 
        </td>
        <td bgColor=#ddeeff width=150>
               Priority <input type=text name=vlan_wifi_vap1_pri size=1 maxlength=1 value="<%voip_net_get("vlan_wifi_vap1_pri");%>"> ( 0~7 )
        </td>
        
</tr>
<tr>
        <td bgColor=#aaddff width=200>
        <input type=checkbox name=vlan_wifi_vap2_enable onClick="enableVlan();" <%voip_net_get("vlan_wifi_vap2_enable");%>>For Wifi VAP3 Packets
       </td>
        <td bgColor=#ddeeff width=150>
               VLAN ID <input type=text name=vlan_wifi_vap2_tag size=4 maxlength=4 value="<%voip_net_get("vlan_wifi_vap2_tag");%>"> 
        </td>
        <td bgColor=#ddeeff width=150>
               Priority <input type=text name=vlan_wifi_vap2_pri size=1 maxlength=1 value="<%voip_net_get("vlan_wifi_vap2_pri");%>"> ( 0~7 )
        </td>
        
</tr>
<tr>
        <td bgColor=#aaddff width=200>
        <input type=checkbox name=vlan_wifi_vap3_enable onClick="enableVlan();" <%voip_net_get("vlan_wifi_vap2_enable");%>>For Wifi VAP4 Packets
       </td>
        <td bgColor=#ddeeff width=150>
               VLAN ID <input type=text name=vlan_wifi_vap3_tag size=4 maxlength=4 value="<%voip_net_get("vlan_wifi_vap3_tag");%>"> 
        </td>
        <td bgColor=#ddeeff width=150>
               Priority <input type=text name=vlan_wifi_vap3_pri size=1 maxlength=1 value="<%voip_net_get("vlan_wifi_vap3_pri");%>"> ( 0~7 )
        </td>
        
</tr>
</table>
<table cellSpacing=1 cellPadding=1 border=0 width=650>
<tr>
        <td bgColor=#aaddff width=200>
                <input type=checkbox name=vlan_bridge_enable onClick="enableVlan();" <%voip_net_get("vlan_bridge_enable");%>>Bridge with WAN
        </td>
        <td bgColor=#ddeeff width=150>
                VLAN ID <input type=text name=vlan_bridge_tag size=4 maxlength=4 value="<%voip_net_get("vlan_bridge_tag");%>"> 
        </td>
	
        <td bgColor=#ddeeff width=150>
                <input type=checkbox name=vlan_bridge_multicast_enable onClick="enableVlan();" <%voip_net_get("vlan_bridge_multicast_enable");%>>Multicast Packet
        </td>
        <td bgColor=#ddeeff width=150>
                VLAN ID <input type=text name=vlan_bridge_multicast_tag size=4 maxlength=4 value="<%voip_net_get("vlan_bridge_multicast_tag");%>"> 
        </td>

</tr>
</table>



<table cellSpacing=1 cellPadding=1 border=0 width=650>
<tr>     
		<td width=200></td>
        <td bgColor=#ddeeff width=450>
                <input type=checkbox name=vlan_bridge_port_0 <%voip_net_get("vlan_bridge_port_0");%>>LAN Port 0
                <input type=checkbox name=vlan_bridge_port_1 <%voip_net_get("vlan_bridge_port_1");%>>LAN Port 1
                <input type=checkbox name=vlan_bridge_port_2 <%voip_net_get("vlan_bridge_port_2");%>>LAN Port 2
                <input type=checkbox name=vlan_bridge_port_3 <%voip_net_get("vlan_bridge_port_3");%>>LAN Port 3
        </td>
</tr>
<tr>
		  <td width=200></td>
          <td bgColor=#ddeeff width=450>
                <input type=checkbox name=vlan_bridge_port_wifi <%voip_net_get("vlan_bridge_port_wifi");%>>WiFi root
                <input type=checkbox name=vlan_bridge_port_vap0 <%voip_net_get("vlan_bridge_port_vap0");%>>VAP 1
                <input type=checkbox name=vlan_bridge_port_vap1 <%voip_net_get("vlan_bridge_port_vap1");%>>VAP 2
                <input type=checkbox name=vlan_bridge_port_vap2 <%voip_net_get("vlan_bridge_port_vap2");%>>VAP 3
                <input type=checkbox name=vlan_bridge_port_vap3 <%voip_net_get("vlan_bridge_port_vap3");%>>VAP 4
        </td>

</tr>

</table>

<td>Note I:  The lan port 0 is near the WAN port. </td>
<br>
<td>  Note II:  VLAN ID 11 is reserved for other purpose. </td>
</p>

<p <%voip_net_get("display_hwnat");%>>
<b>Enable/Disable HW-NAT</b>
<table cellSpacing=1 cellPadding=2 border=0 width=375>
	<tr>
    	<td bgColor=#aaddff>HWNAT</td>
		<td bgColor=#ddeeff>
		<select name=hwnat_enable>
			 "<%voip_net_get("hwnat_enable");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>


</table>

<p <%voip_net_get("display_bandwidth_mgr");%>>
<b>Bandwidth Manager</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
    	<tr align=center>
		<td bgcolor=#aaddff>Port</td>
		<td bgcolor=#aaddff>Egress(unit: 64kbps)</td>
		<td bgcolor=#aaddff>Ingress(unit: 16kbps)</td>
	</tr>
	<tr align=center>
	<td bgColor=#aaddff>LAN Port 0</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort0_Bandwidth_out size=5 maxlength=5 value="<%voip_net_get("LANPort0_Bandwidth_out");%>">
	</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort0_Bandwidth_in size=5 maxlength=5 value="<%voip_net_get("LANPort0_Bandwidth_in");%>">
	</td>
	</tr>
	<tr align=center>
	<td bgColor=#aaddff>LAN Port 1</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort1_Bandwidth_out size=5 maxlength=5 value="<%voip_net_get("LANPort1_Bandwidth_out");%>">
	</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort1_Bandwidth_in size=5 maxlength=5 value="<%voip_net_get("LANPort1_Bandwidth_in");%>">
	</td>
	</tr>
	<tr align=center>
	<td bgColor=#aaddff>LAN Port 2</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort2_Bandwidth_out size=5 maxlength=5 value="<%voip_net_get("LANPort2_Bandwidth_out");%>">
	</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort2_Bandwidth_in size=5 maxlength=5 value="<%voip_net_get("LANPort2_Bandwidth_in");%>">
	</td>
	</tr>
	<tr align=center>
	<td bgColor=#aaddff>LAN Port 3</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort3_Bandwidth_out size=5 maxlength=5 value="<%voip_net_get("LANPort3_Bandwidth_out");%>">
	</td>
	<td bgColor=#ddeeff>
                <input type=text name=LANPort3_Bandwidth_in size=5 maxlength=5 value="<%voip_net_get("LANPort3_Bandwidth_in");%>">
	</td>
	</tr>
	<tr align=center>
	<td bgColor=#aaddff>WAN Port</td>
	<td bgColor=#ddeeff>
                <input type=text name=WANPort_Bandwidth_out size=5 maxlength=5 value="<%voip_net_get("WANPort_Bandwidth_out");%>">
	</td>
	<td bgColor=#ddeeff>
                <input type=text name=WANPort_Bandwidth_in size=5 maxlength=5 value="<%voip_net_get("WANPort_Bandwidth_in");%>">
	</td>
	</tr>
</table>
	<td>Note: Unlimit: 0, Egress max: 16383, Ingress max: 65535</td>

<p>
<table cellSpacing=1 cellPadding=2 border=0 width=375>
<tr>
   	<td colspan=3 align=center>
   		<input type="submit" value="Apply" onclick="return check_network()">
   		&nbsp;&nbsp;&nbsp;&nbsp;
   		<input type="reset" value="Reset">
   	</td>
</tr>
</table>

</p>
</form>
</body>
</html>
