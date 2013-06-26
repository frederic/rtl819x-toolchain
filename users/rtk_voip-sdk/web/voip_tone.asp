<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
</head>
<body bgcolor="#ffffff" text="#000000">

<form method="get" action="/goform/voip_tone_set" name=toneform>
<b>Select Country</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>

  	<tr>
    	<td bgColor=#aaddff>Country</td>
		<td bgColor=#ddeeff>
		<select name=tone_country onchange="changeCountry();">
			 "<%voip_tone_get("tone_country");%>"
		</select>
		</td>

	</tr>	


</table>

<div id = tonetable <%voip_tone_get("display");%> >
<b>Select Tone</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155>DIAL TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=dial>
			 	<%voip_tone_get("dial");%>
			</select>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>RING TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=ring>
			 	<%voip_tone_get("ring");%>
			</select>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>BUSY TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=busy>
			 	<%voip_tone_get("busy");%>
			</select>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>WAITING TONE</td>
    	<td bgColor=#ddeeff width=170>
			<select name=waiting>
			 	<%voip_tone_get("waiting");%>
			</select>
		</td>
	</tr>
</table>
</div>
<table cellSpacing=1 cellPadding=2 border=0 width=400>

	<tr>
    	<td colspan=3 align=center>
    		<input type="submit" name="Country" value="Apply" >
   	</td>
	</tr>
</table>

<b>Select Custom Tone</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155>Custom Tone</td>
    	<td bgColor=#ddeeff width=170>
			<select name=selfItem  onChange="toneform.submit()">
			 	<%voip_tone_get("selfItem");%>
			</select>
		</td>
	</tr>
</table>

<p>
<b>Tone Parameters</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
	<tr>
	<td bgColor=#aaddff>ToneType</td>
	<td bgColor=#ddeeff>
		<select name=type>
			 <%voip_tone_get("type");%>
		</select>
	</td>
	</tr>
	
	<tr>
<!--
		<td bgColor=#aaddff rowspan=3>ToneCycle</td>
-->
		<td bgColor=#aaddff>ToneCycle</td>
		<td bgColor=#ddeeff>
			<select name=cycle>
			 	<%voip_tone_get("cycle");%>
			</select>

		</td>
	</tr>

<!--	
	<tr>
		<td bgColor=#ddeeff>cycle no: 	
		<input type=text name=Gain4 size=10 maxlength=39 value="<%voip_tone_get("Gain4");%>">
		</td>	
	</tr>

	
	<tr>
		<td bgColor=#ddeeff><font size=1>cycle no is supported only when cadence mode</font>
		</td>	
	</tr>
-->	
	
	
	<tr>	
	<td bgColor=#aaddff>ToneNum</td>	
    	<td bgColor=#ddeeff><input type=text name=ToneNUM size=1 maxlength=1 value=<%voip_tone_get("ToneNUM");%> > <font size=1>(1~3)</font>
		</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq1 size=20 maxlength=39 value="<%voip_tone_get("Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Freq2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq2 size=20 maxlength=39 value="<%voip_tone_get("Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff rowspan=2>Freq3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq3 size=20 maxlength=39 value="<%voip_tone_get("Freq3");%>"> <font size=1>(Hz)</font>
	</td>

	</tr>
	
	<tr>
		<td bgColor=#ddeeff><font size=1>Freq3 is supported when SUCC tone type</font>
		</td>	
	</tr>
<!--
   	<tr>
    	<td bgColor=#aaddff>Freq4</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Freq4 size=20 maxlength=39 value="<%voip_tone_get("Freq4");%>">	</td>
	</tr>
-->
   	<tr>
    	<td bgColor=#aaddff>Gain1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain1 size=20 maxlength=39 value="<%voip_tone_get("Gain1");%>"> <font size=1>(- dBm)(63~0)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Gain2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain2 size=20 maxlength=39 value="<%voip_tone_get("Gain2");%>"> <font size=1>(- dBm)(63~0)</font></td>
	</tr>
<!--
   	<tr>
    	<td bgColor=#aaddff>Gain3</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain3 size=20 maxlength=39 value="<%voip_tone_get("Gain3");%>">	</td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Gain4</td>
    	<td bgColor=#ddeeff>
	<input type=text name=Gain4 size=20 maxlength=39 value="<%voip_tone_get("Gain4");%>">	</td>
	</tr>
-->	

	<tr>
		<td bgColor=#aaddff>CadNum</td>	
    	<td bgColor=#ddeeff>
		<input type=text name=cadNUM size=1 maxlength=1 value=<%voip_tone_get("cadNUM");%>> <font size=1>(1~4)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOn0</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn0 size=20 maxlength=39 value="<%voip_tone_get("CadOn0");%>"> <font size=1>(msec)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOn1</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn1 size=20 maxlength=39 value="<%voip_tone_get("CadOn1");%>"> <font size=1>(msec)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn2</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn2 size=20 maxlength=39 value="<%voip_tone_get("CadOn2");%>"> <font size=1>(msec)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOn3</td>
    	<td bgColor=#ddeeff><input type=text name=CadOn3 size=20 maxlength=39 value="<%voip_tone_get("CadOn3");%>"> <font size=1>(msec)</font></td>
	</tr>

   <tr>
		<td  bgColor=#aaddff>CadOff0</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff0 size=20 maxlength=39 value="<%voip_tone_get("CadOff0");%>"> <font size=1>(msec)</font></td>
	</tr>
	
   <tr>
		<td  bgColor=#aaddff>CadOff1</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff1 size=20 maxlength=39 value="<%voip_tone_get("CadOff1");%>"> <font size=1>(msec)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff2</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff2 size=20 maxlength=39 value="<%voip_tone_get("CadOff2");%>"> <font size=1>(msec)</font></td>
	</tr>
   <tr>
		<td  bgColor=#aaddff>CadOff3</td>
    	<td bgColor=#ddeeff><input type=text name=CadOff3 size=20 maxlength=39 value="<%voip_tone_get("CadOff3");%>"> <font size=1>(msec)</font></td>
	</tr>
<!--
   	<tr>
    	<td bgColor=#aaddff>PatternOff</td>
    	<td bgColor=#ddeeff><input type=text name=PatternOff size=20 maxlength=39 value="<%voip_tone_get("PatternOff"); %>">	</td>
	</tr>
	<tr>
-->

	<tr>
    	<td colspan=3 align=center>
    		<input type="submit" name="Tone" value="Apply" >
    		&nbsp;&nbsp;&nbsp;&nbsp;    	
    		<input type="reset" value="Reset">	
    	</td>
	</tr>
</table>
</p>

<p <%voip_tone_get("display_distone");%>>
<b>Select Disconnect Tone Number</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>

  	<tr>
    	<td bgColor=#aaddff>Number</td>
		<td bgColor=#ddeeff>
		<select name=distone_num onchange="changeNum();">
			 "<%voip_tone_get("distone_num");%>"
		</select>
		</td>

	</tr>	


</table>


<div id = distone1tab <%voip_tone_get("d1display");%> >
<b>Disconnect Tone 1 Parameters</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155>FreqNum</td>
    	<td bgColor=#ddeeff width=170>
			<select name=d1freqnum>
			 	<%voip_tone_get("d1freqnum");%>
			</select>
		</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Freq1 size=20 maxlength=39 value="<%voip_tone_get("d1Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Freq2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Freq2 size=20 maxlength=39 value="<%voip_tone_get("d1Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq Accuracy</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Accur size=20 maxlength=39 value="<%voip_tone_get("d1Accur");%>"> <font size=1>(Default:16)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Level</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1Level size=20 maxlength=39 value="<%voip_tone_get("d1Level");%>"> <font size=1>(Default:800)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone ON up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1ONup size=20 maxlength=39 value="<%voip_tone_get("d1ONup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone ON low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1ONlow size=20 maxlength=39 value="<%voip_tone_get("d1ONlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone OFF up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1OFFup size=20 maxlength=39 value="<%voip_tone_get("d1OFFup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone OFF low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d1OFFlow size=20 maxlength=39 value="<%voip_tone_get("d1OFFlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

</table>
</div>

<div id = distone2tab <%voip_tone_get("d2display");%> >
<b>Disconnect Tone 2 Parameters</b>
<table cellSpacing=1 cellPadding=2 border=0 width=400>
  	<tr>
    	<td bgColor=#aaddff width=155>FreqNum</td>
    	<td bgColor=#ddeeff width=170>
			<select name=d2freqnum>
			 	<%voip_tone_get("d2freqnum");%>
			</select>
		</td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq1</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Freq1 size=20 maxlength=39 value="<%voip_tone_get("d2Freq1");%>"> <font size=1>(Hz)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Freq2</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Freq2 size=20 maxlength=39 value="<%voip_tone_get("d2Freq2");%>"> <font size=1>(Hz)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Freq Accuracy</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Accur size=20 maxlength=39 value="<%voip_tone_get("d2Accur");%>"> <font size=1>(Default:16)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Level</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2Level size=20 maxlength=39 value="<%voip_tone_get("d2Level");%>"> <font size=1>(Default:800)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone ON up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2ONup size=20 maxlength=39 value="<%voip_tone_get("d2ONup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone ON low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2ONlow size=20 maxlength=39 value="<%voip_tone_get("d2ONlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

   	<tr>
    	<td bgColor=#aaddff>Tone OFF up limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2OFFup size=20 maxlength=39 value="<%voip_tone_get("d2OFFup");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>
   	<tr>
    	<td bgColor=#aaddff>Tone OFF low limit</td>
    	<td bgColor=#ddeeff>
	<input type=text name=d2OFFlow size=20 maxlength=39 value="<%voip_tone_get("d2OFFlow");%>"> <font size=1>(Unit:10ms)</font></td>
	</tr>

</table>
</div>

<table cellSpacing=1 cellPadding=2 border=0 width=400>

	<tr>
    	<td colspan=3 align=center>
    		<input type="submit" name="Distone" value="Apply" >
   	</td>
	</tr>
</table>

</p>

</form>
</body>
</html>