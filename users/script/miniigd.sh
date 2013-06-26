#!/bin/sh


if [ $# -lt 1 ]; then 
	#echo "Usage: $0 {init}"; 
	exit 1 ; 
fi

if [ $1 != 'init' ]; then 
	#echo "Usage: $0 {init}"; 
	exit 1 ; 
fi

BRIDGE_INTERFACE=br0
IGD_PID_FILE=/var/run/miniupnpd.pid
eval `flash get UPNP_ENABLED`

rtl_kill_old_miniigd() {
	killall -15 miniigd 2> /dev/null
	if [ -f "$IGD_PID_FILE" ]; then
		rm -f $IGD_PID_FILE
	fi
}



rtl_start_miniigd() {
  	route del -net 239.255.255.250 netmask 255.255.255.255 br0
  	route add -net 239.255.255.250 netmask 255.255.255.255 br0
	eval `flash get WAN_DHCP`
	eval `flash get OP_MODE`

	echo "To start miniigd"
	
#	eval `flash get PPP_SESSION_UPNP`
	PPP_SESSION_UPNP=0
	# if wireless ISP mode , set WAN to wlan0
	if [ "$OP_MODE" = '2' ];then
		eval `flash get WISP_WAN_ID`
		miniigd -e $WAN_DHCP -i $BRIDGE_INTERFACE -w $WISP_WAN_ID -s $PPP_SESSION_UPNP
	else
		miniigd -e $WAN_DHCP -i $BRIDGE_INTERFACE -s $PPP_SESSION_UPNP
	fi
}

rtl_miniigd() {
	rtl_kill_old_miniigd
	
	if [ $UPNP_ENABLED = 1 ]; then 
		rtl_start_miniigd
	fi
}

rtl_miniigd

