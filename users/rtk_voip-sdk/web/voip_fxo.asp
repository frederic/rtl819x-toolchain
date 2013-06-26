<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
</head>

<body bgcolor="#ffffff" text="#000000" onload=enable_cid_det_mode()>

<form method="get" action="/goform/voip_fxo_set" name=fxo_form>


<b>FXO Setting</b>

<p <%voip_fxo_get("display_fxo");%>>

<table cellSpacing=1 cellPadding=2 border=0 width=450>

  	<tr bgColor=#888888>
	<td colspan=3>
	<font color=#ffffff>
	Volume Adjustment
	</font>
	</td>
	</tr>

  	<tr>
	    	<td bgColor=#aaddff width=150 rowspan=2>FXO Volume</td>
		<td bgColor=#ddeeff>PSTN to Phone Volume</td>
		<td bgColor=#ddeeff>
			<select name=daa_rxVolumne>
				 "<%voip_fxo_get("daa_rxVolumne");%>"
			</select>
		</td>
	</tr>
	
  	<tr>
		<td bgColor=#ddeeff>Phone to PSTN Gain</td>
		<td bgColor=#ddeeff>
			<select name=daa_txVolumne>
				  "<%voip_fxo_get("daa_txVolumne");%>"
			</select>
		</td>
	</tr>	
	
</table>

<p>

<table cellSpacing=1 cellPadding=2 border=0 width=450>

	<tr bgColor=#888888>
		<td colspan=2>
		<font color=#ffffff>
		Function Key: Must be * + 0~9
		</font>
		</td>
	</tr>	
	
	<tr <%voip_fxo_get("display_funckey_pstn");%>>
	   	<td bgColor=#aaddff width=150>Switch to PSTN</td>
		<td bgColor=#ddeeff>
			<input type=text name=funckey_pstn size=5 maxlength=2 value="<%voip_fxo_get("funckey_pstn");%>">
			( default: *0 )
		</td>
	</tr>
</table>




<p <%voip_fxo_get("display_cid_det");%>>
	
<table cellSpacing=1 cellPadding=2 border=0 width=450>

	<tr bgColor=#888888>
		<td colspan=2>
		<font color=#ffffff>
		Caller ID Detection
		</font>
		</td>
	</tr>
	
	<tr>
	<td bgColor=#aaddff width=150>Auto Detection</td>
	<td bgColor=#aaddff> 
		<%voip_fxo_get("caller_id_auto_det");%>
	</td>
	
	<input type=hidden name=caller_id_test value=hello>
	
	</tr>	

	<tr>
	    	<td bgColor=#aaddff width=150>Caller ID Detection Mode</td>
		<td bgColor=#ddeeff>
			<select name=caller_id_det>
				"<%voip_fxo_get("caller_id_det");%>"
			</select>
		</td>
	</tr>

</table>
</p>

<table cellSpacing=1 cellPadding=2 border=0 width=375>
<tr>
   	<td colspan=3 align=center>
   		<input type="submit" value="Apply" onclick="return check_fxo()">
   		&nbsp;&nbsp;&nbsp;&nbsp;    	
   		<input type="reset" value="Reset">	
   	</td>
</tr>
</table>

</form>
</body>
</html>