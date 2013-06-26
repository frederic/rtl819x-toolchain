#!/bin/sh
LINKFILE=/etc/ppp/link
DNRD_PID=/var/run/dnrd.pid
TMPFILE=/tmp/tmpfile

eval `flash get UPNP_ENABLED`
eval `flash get WAN_DHCP`
eval `flash get DEVICE_NAME`
eval `flash get DNS_MODE`
eval `flash get DNS1`
eval `flash get DNS2`
eval `flash get DNS3`

eval `flash get OP_MODE`
eval `flash get  WISP_WAN_ID`

rtl_set_dnrd_dns() {
	if [ ! -f $DNRD_PID ]; then
	  if [ $DNS_MODE != 1 ]; then
	     dnrd -s 168.95.1.1
	  fi
	  if [ $DNS_MODE = 1 ]; then
	    if [ "$DNS1" != '0.0.0.0' ]; then
	       DNS="$DNS -s $DNS1"
	    fi
	    if [ "$DNS2" != '0.0.0.0' ]; then
	       DNS="$DNS -s $DNS2"
	    fi
	    if [ "$DNS3" != '0.0.0.0' ]; then
		DNS="$DNS -s $DNS3"
	    fi
	    dnrd $DNS
	  fi
	fi
}


rtl_kill_upnpd() {
	 line=0
	 ps -A | grep upnpd > $TMPFILE
	 line=`cat $TMPFILE | wc -l`
	 num=1
	 while [ $num -le $line ];
	 do
	  pat0=` head -n $num $TMPFILE | tail -n 1`
	  pat1=`echo $pat0 | cut -f2 -dS`
	  pat2=`echo $pat1 | cut -f1 -d " "`
	  if [ "$pat2" = 'upnpd' ]; then
	    pat1=`echo $pat0 | cut -f1 -dS`
	    pat2=`echo $pat1 | cut -f1 -d " "`
	    kill -9 $pat2
	  fi
	  num=`expr $num + 1`
	 done
}


 WAN_EXIST=0;

rtl_check_wan_if() { 
	# if wireless ISP mode , set WAN to wlan0
	line=0
	if [ "$OP_MODE" = '2' ];then
		WAN=wlan$WISP_WAN_ID
	else
		WAN=eth1	
	fi

	 ifconfig $WAN | grep "inet addr" > $TMPFILE
	 line=`cat $TMPFILE | wc -l`
	 num=1
	 while [ $num -le $line ];
	 do
	  pat0=` head -n $num $TMPFILE | tail -n 1`
	  pat1=`echo $pat0 | cut -f2 -dS`
	  pat2=`echo $pat1 | cut -f1 -d " "`
	  if [ "$pat2" = 'inet' ]; then
	     WAN_EXIST=1;
	  fi
	  num=`expr $num + 1`
	 done
}

rtl_check_dev_name() {
	if [ "$DEVICE_NAME" = "" ]; then
	  DEVICE_NAME="RTL8181"
	fi
}

rtl_start_upnpd() { 
  route del -net 239.255.255.250 netmask 255.255.255.255 br0
  route add -net 239.255.255.250 netmask 255.255.255.255 br0
  if [ $WAN_DHCP = 1 ] && [ $WAN_EXIST = 0 ];then 
    upnpd lo br0 "$DEVICE_NAME" &
  elif [ $WAN_DHCP = 1 ] && [ $WAN_EXIST = 1 ];then 
    upnpd $WAN br0 "$DEVICE_NAME" &
  elif [ $WAN_DHCP = 0 ];then 
    upnpd $WAN br0 "$DEVICE_NAME" &
  elif [ $WAN_DHCP = 3 ] && [ -f $LINKFILE ]; then
    upnpd ppp0 br0 "$DEVICE_NAME" &
  elif [ $WAN_DHCP = 4 ] && [ -f $LINKFILE ]; then
    upnpd ppp0 br0 "$DEVICE_NAME" &
  else
    upnpd lo br0 "$DEVICE_NAME" &
  fi
}


rtl_upnp() {
	rtl_set_dnrd_dns
	rtl_kill_upnpd
	rtl_check_dev_name
	
	if [ $UPNP_ENABLED = 1 ]; then
		rtl_start_upnpd
	fi
}

rtl_upnp

