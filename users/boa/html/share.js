

function disableTextField (field) {
	if (document.all || document.getElementById)
		field.disabled = true;
	else {
		field.oldOnFocus = field.onfocus;
		field.onfocus = skip;
	}
}
function enableTextField (field) {
	if (document.all || document.getElementById)
		field.disabled = false;
	else {
		field.onfocus = field.oldOnFocus;
	}
}


function getDigit(str, num)
{
  i=1;
  if ( num != 1 ) {
  	while (i!=num && str.length!=0) {
		if ( str.charAt(0) == '.' ) {
			i++;
		}
		str = str.substring(1);
  	}
  	if ( i!=num )
  		return -1;
  }
  for (i=0; i<str.length; i++) {
  	if ( str.charAt(i) == '.' ) {
		str = str.substring(0, i);
		break;
	}
  }
  if ( str.length == 0)
  	return -1;
  d = parseInt(str, 10);
  return d;
}

function getDigitforMac(str, num)
{
  i=1;
  if ( num != 1 ) {
  	while (i!=num && str.length!=0) {
		if ( str.charAt(0) == '-' ) {
			i++;
		}
		str = str.substring(1);
  	}
  	if ( i!=num )
  		return -1;
  }
  for (i=0; i<str.length; i++) {
  	if ( str.charAt(i) == '-' ) {
		str = str.substring(0, i);
		break;
	}
  }
  if ( str.length == 0)
  	return -1;
  d = parseInt(str, 10);
  return d;
}

function validateKey(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '.' ) )
			continue;
	return 0;
  }
  return 1;
}

function validateKey2(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '-' ) || 
    		(str.charAt(i) >= 'A' && str.charAt(i) <= 'F')||
    		(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') )
			continue;
	return 0;
  }
  return 1;
}

function IsLoopBackIP(str)
{
	if(str=="127.0.0.1")
		return 1;
	return 0;
}

function checkDigitRange(str, num, min, max)
{
  d = getDigit(str,num);
  if ( d > max || d < min )
      	return false;
  return true;
}

function checkDigitRangeforMac(str, num, min, max)
{  
  d = getDigitforMac(str,num);
  if ( d > max || d < min )
      	return false;
  return true;
}

function deleteClick()
{
	if ( !confirm('Do you really want to delete the selected entry?') ) {
		return false;
	}
	else
		return true;
}
        
function deleteAllClick()
{
	if ( !confirm('Do you really want to delete the all entries?') ) {
		return false;
	}
	else
		return true;
}

function delClick(index)
{
	if ( !confirm('Are you sure you want to delete?') ) {
		return false;
	}
	
	document.actionForm.action.value=0;
	document.actionForm.idx.value=index;
	document.actionForm.submit();
	return true;
}

function editClick(index)
{
	document.actionForm.action.value=1;
	document.actionForm.idx.value=index;
	document.actionForm.submit();
	return true;
}

function verifyBrowser() {
	var ms = navigator.appVersion.indexOf("MSIE");
	ie4 = (ms>0) && (parseInt(navigator.appVersion.substring(ms+5, ms+6)) >= 4);
	var ns = navigator.appName.indexOf("Netscape");
	ns= (ns>=0) && (parseInt(navigator.appVersion.substring(0,1))>=4);
	if (ie4)
		return "ie4";
	else
		if(ns)
			return "ns";
		else
			return false;
}

function isBrowser(b,v) {
	browserOk = false;
	versionOk = false;
	
	browserOk = (navigator.appName.indexOf(b) != -1);
	if (v == 0) versionOk = true;
	else  versionOk = (v <= parseInt(navigator.appVersion));
	return browserOk && versionOk;
}

function disableButton (button) {
  if (document.all || document.getElementById)
    button.disabled = true;
  else if (button) {
    button.oldOnClick = button.onclick;
    button.onclick = null;
    button.oldValue = button.value;
    button.value = 'DISABLED';
  }
}

function disableButtonIB (button) {
	if (isBrowser('Netscape', 0))
		return;
	if (document.all || document.getElementById)
		button.disabled = true;
	else if (button) {
		button.oldOnClick = button.onclick;
		button.onclick = null;
		button.oldValue = button.value;
		button.value = 'DISABLED';
	}
}

function disableButtonVB (button) {
  if (verifyBrowser() == "ns")
  	return;
  if (document.all || document.getElementById)
    button.disabled = true;
  else if (button) {
    button.oldOnClick = button.onclick;
    button.onclick = null;
    button.oldValue = button.value;
    button.value = 'DISABLED';
  }
}

function enableButton (button) {
  if (document.all || document.getElementById)
    button.disabled = false;
  else if (button) {
    button.onclick = button.oldOnClick;
    button.value = button.oldValue;
  }
}

function enableButtonVB (button) {
  if (verifyBrowser() == "ns")
  	return;
  if (document.all || document.getElementById)
    button.disabled = false;
  else if (button) {
    button.onclick = button.oldOnClick;
    button.value = button.oldValue;
  }
}

function enableButtonIB (button) {
	if (isBrowser('Netscape', 4))
		return;
	if (document.all || document.getElementById)
		button.disabled = false;
	else if (button) {
		button.onclick = button.oldOnClick;
		button.value = button.oldValue;
	}
}

function sendCPEClicked(F)
{
  if(document.cpe_cert.binary.value == ""){
  	document.cpe_cert.binary.focus();
  	alert('File name can not be empty !');
  	return false;
  }
  
  F.submit();
}

function sendCAClicked(F)
{
  if(document.ca_cert.binary.value == ""){
  	document.ca_cert.binary.focus();
  	alert('File name can not be empty !');
  	return false;
  }
  
  F.submit();
}

