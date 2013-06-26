<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>TLS Certificate Import</title>
<% language=javascript %>
<SCRIPT>
function sendCAClicked1(F)
{
  if(document.TLScacert1.binary1.value == ""){
  	document.TLScacert1.binary1.focus();
  	alert('File name can not be empty !');
  	return false;
  }
  
  F.submit();
}

function sendCAClicked2(F)
{
  if(document.TLScacert2.binary2.value == ""){
  	document.TLScacert2.binary2.focus();
  	alert('File name can not be empty !');
  	return false;
  }
  
  F.submit();
}

function sendCAClicked3(F)
{
  if(document.TLScacert3.binary3.value == ""){
  	document.TLScacert3.binary3.focus();
  	alert('File name can not be empty !');
  	return false;
  }
  
  F.submit();
}

function sendCAClicked4(F)
{
  if(document.TLScacert4.binary4.value == ""){
  	document.TLScacert4.binary4.focus();
  	alert('File name can not be empty !');
  	return false;
  }
  
  F.submit();
}
</SCRIPT>
</head>
<body>
<blockquote>
<h2><font color="#0000FF">TLS Certificate Import</font></h2>
<form  onsubmit="return sendCAClicked<%voip_TLSGetCertInfo("certNum");%>(this.form);" action=/goform/voip_TLSCertUpload method=POST enctype="multipart/form-data" name="TLScacert<%voip_TLSGetCertInfo("certNum");%>">
<table cellSpacing=1 cellPadding=2 border=0>
<tr>
	<td bgColor=#070ef8 colspan=2><font color=#ffffff>TLS Certificate for FXS<%voip_TLSGetCertInfo("fxs");%>: Proxy<%voip_TLSGetCertInfo("proxy");%></font></td>
</tr>
<tr>
	<td bgColor=#aaddff width=155>Import TLS Certificate</td>
	<td bgColor=#ddeeff>
	<input type="file" name="binary<%voip_TLSGetCertInfo("certNum");%>" size=24>&nbsp;&nbsp;	
	<input type="submit" value="UPLOAD" name="submit_cert<%voip_TLSGetCertInfo("certNum");%>">
	</td>
</tr>
<tr>
	<td bgColor=#aaddff width=155>Certificate Issuer</td>
	<td bgColor=#ddeeff><%voip_TLSGetCertInfo("issuer");%></td>
</tr>
<tr>
	<td bgColor=#aaddff width=155>Certificate Issue Date</td>
	<td bgColor=#ddeeff><%voip_TLSGetCertInfo("issuerDate");%>(yy/mm/dd)</td>
</tr>
<tr>
	<td bgColor=#aaddff width=155>Certificate valid Date</td>
	<td bgColor=#ddeeff><%voip_TLSGetCertInfo("validDate");%>(yy/mm/dd)</td>
</tr>
</form>
</blockquote>
</body>
</html>