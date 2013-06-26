#!/bin/sh
eval `flash get DNS_MODE`
eval `flash get WAN_DHCP`
eval `flash get PPP_MTU_SIZE`
eval `flash get PPTP_MTU_SIZE`
eval `flash get L2TP_MTU_SIZE`
eval `flash get PPTP_CONNECTION_TYPE`
RESOLV=/etc/ppp/resolv.conf
PIDFILE=/var/run/dnrd.pid
CONNECTFILE=/etc/ppp/connectfile

echo "pass" > $CONNECTFILE

rtl_add_ppp_defgw() {
	if [ $WAN_DHCP = 4 -o $WAN_DHCP = 3 ]; then
	  ptpgw0=`ifconfig ppp0 | grep -i "P-t-P:"`
	  ptpgw1=`echo $ptpgw0 | cut -f3 -d:`
	  ptpgw=`echo $ptpgw1 | cut -f1 -d " "`
	  sleep 2
	  route del default
	  route add -net default gw $ptpgw dev ppp0
	fi
	if [ $WAN_DHCP = 4 ]; then
	  if [ $PPTP_CONNECTION_TYPE = 1 ]; then
	    echo "5" > /proc/pptp_conn_ck
	  else
	    echo "0" > /proc/pptp_conn_ck
	  fi
	fi
}

rtl_set_dns() {
	if [ $DNS_MODE != 1 ]; then
	  if [ -r "$RESOLV" ] ; then
	    if [ -f $PIDFILE ]; then
	      PID=`cat $PIDFILE`
	      kill -9 $PID 
	      rm -f $PIDFILE
	    fi
	    line=0
	    cat $RESOLV | grep nameserver > /tmp/ddfile 
	    line=`cat /tmp/ddfile | wc -l`
	    num=1
	    while [ $num -le $line ];
	    do
	      pat0=` head -n $num /tmp/ddfile | tail -n 1`
	      pat1=`echo $pat0 | cut -f2 -d " "`
	      DNS="$DNS -s $pat1"
	      num=`expr $num + 1`
	    done
	    num=1
	    while [ $num -le 5 ];
	    do
	      dnrd --cache=off $DNS
	      if [ -f $PIDFILE ]; then
		break
	      else
		sleep 1
		num=`expr $num + 1`
	      fi
	    done
	    
	    cp $RESOLV /var
	  fi
	else
		DNS="--cache=off"
		if [ -f $PIDFILE ]; then
		PID=`cat $PIDFILE`
		kill -9 $PID 
		rm -f $PIDFILE
	  fi
		eval `flash get DNS1`
		if [ "$DNS1" != '0.0.0.0' ]; then
			DNS="$DNS -s $DNS1"
			echo nameserver $DNS1 > $RESOLV
		fi
		eval `flash get DNS2`
		if [ "$DNS2" != '0.0.0.0' ]; then
			DNS="$DNS -s $DNS2"
			echo nameserver $DNS2 >> $RESOLV
	  fi
		eval `flash get DNS3`
		if [ "$DNS3" != '0.0.0.0' ]; then
			DNS="$DNS -s $DNS3"
			echo nameserver $DNS3 >> $RESOLV
	  fi
		dnrd $DNS
		cp $RESOLV /var/resolv.conf
	fi
}

rtl_set_mtu() { 
	if [ $WAN_DHCP = 4 ]; then
	  ifconfig ppp0 mtu $PPTP_MTU_SIZE txqueuelen 25
	elif [ $WAN_DHCP = 6 ]; then
	  ifconfig ppp0 mtu $L2TP_MTU_SIZE txqueuelen 25
	else
	  ifconfig ppp0 mtu $PPP_MTU_SIZE txqueuelen 25
	fi
}
#upnp.sh
rtl_check_vpn() {
	if [ -f /bin/vpn.sh ]; then
	      echo 'Setup VPN'
	      vpn.sh all
	fi
}

#restart igmpproxy
rtl_restart_igmpproxy() {
	eval `flash get IGMP_PROXY_DISABLED`
	killall -9 igmpproxy 2> /dev/null
	if [ $IGMP_PROXY_DISABLED = 0 ]; then
		igmpproxy ppp0 br0 &
		echo 1 > /proc/br_igmpsnoop
	else
		echo 0 > /proc/br_igmpsnoop
	fi	
}

rtl_check_voip_dns() {
# rock: feature is decided by mkimg
###VOIP_SUPPORT###
	if [ "$VOIP_SUPPORT" != "" ]; then
		if [ -f  /etc/ppp/resolv.conf ]; then
		   # rock: enable dns client if pppoe
		   cat /etc/ppp/resolv.conf > /etc/resolv.conf
		fi
	fi
}

#restart DDNS and ntp while that is killed in disconnect.sh
rtl_check_ddns() {
	eval `flash get DDNS_ENABLED`
	if [ $DDNS_ENABLED = 1 ]; then
		killall -9 ddns.sh 2> /dev/null
		rm -f /var/firstddns 2> /dev/null
		ddns.sh option
	fi
}

rtl_chect_ntp() {
	eval `flash get NTP_ENABLED`
	if [ $NTP_ENABLED = 1 ]; then
	killall -9 ntp.sh 2> /dev/null 
	killall ntpclient 2> /dev/null
	###########kill sleep that ntp.sh created###############
		TMPFILEDL=/tmp/tmpfiledl
		line=0
		ps | grep 'sleep 86400' > $TMPFILEDL
		line=`cat $TMPFILEDL | wc -l`
		num=1
		while [ $num -le $line ];
		do
			pat0=` head -n $num $TMPFILEDL | tail -n 1`
			pat1=`echo $pat0 | cut -f1 -dS`  
			pat2=`echo $pat1 | cut -f1 -d " "`  
			kill -9 $pat2 2> /dev/null
			num=`expr $num + 1`
		done
		rm -f /tmp/tmpfiledl 2> /dev/null
	############################################
	  ntp.sh
	fi
}

rtl_connect() {
        rtl_add_ppp_defgw
        rtl_set_dns
        rtl_set_mtu
        rtl_check_vpn
        rtl_restart_igmpproxy
        rtl_check_voip_dns
        rtl_check_ddns
        rtl_check_ntp
}
 
rtl_connect
