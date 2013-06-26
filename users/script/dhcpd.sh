#!/bin/sh
#
# script file to start udhcpd daemon (udhcp server)
#

if [ $# -lt 2 ]; then echo "Usage: $0 interface {gw | ap} ";  exit 1 ; fi

GETMIB="flash get"
CONF_FILE=/var/udhcpd.conf
LEASE_FILE=/var/lib/misc/udhcpd.leases
PARAM1=$1
PARAM2=$2

# See if DHCP server is on
rtl_check_dhcpd_enable() {
	eval `$GETMIB DHCP`
	if [ "$DHCP" != '2' ]; then
		exit 0
	fi
}

rtl_set_dhcpd_conf() {
	echo "interface $PARAM1" > $CONF_FILE

	eval `$GETMIB DHCP_CLIENT_START`
	echo "start $DHCP_CLIENT_START" >> $CONF_FILE

	eval `$GETMIB DHCP_CLIENT_END`
	echo "end $DHCP_CLIENT_END" >> $CONF_FILE

	eval `$GETMIB SUBNET_MASK`
	echo "opt subnet $SUBNET_MASK" >> $CONF_FILE
}


rtl_set_dhcpd_ap() {
	eval `$GETMIB DEFAULT_GATEWAY`
	echo "opt router $DEFAULT_GATEWAY"  >> $CONF_FILE

	eval `$GETMIB DNS1`
	if [ "$DNS1" != "0.0.0.0" ]; then
		echo "opt dns $DNS1" >> $CONF_FILE
	fi
	eval `$GETMIB DNS2`
	if [ "$DNS2" != "0.0.0.0" ]; then
		echo "opt dns $DNS2" >> $CONF_FILE
	fi
	eval `$GETMIB DNS3`
	if [ "$DNS3" != "0.0.0.0" ]; then
		echo "opt dns $DNS3" >> $CONF_FILE
	fi
	# set default
	if [ "`cat $CONF_FILE | grep dns`" = "" ]; then
		echo "opt dns $DEFAULT_GATEWAY"  >> $CONF_FILE
	fi
	
	eval `$GETMIB DOMAIN_NAME`	
	if [ "$DOMAIN_NAME" != "" ]; then
		echo "opt domain $DOMAIN_NAME" >> $CONF_FILE
	fi	
}

rtl_set_dhcpd_gw() {
	eval `$GETMIB IP_ADDR`
	echo "opt router $IP_ADDR"  >> $CONF_FILE

	eval `$GETMIB DNS_MODE`
	if [ "$DNS_MODE" = '0' ]; then
		echo "opt dns $IP_ADDR" >> $CONF_FILE
	else
		eval `$GETMIB DNS1`
		if [ "$DNS1" != "0.0.0.0" ]; then
			echo "opt dns $DNS1" >> $CONF_FILE
		fi
		eval `$GETMIB DNS2`
		if [ "$DNS2" != "0.0.0.0" ]; then
			echo "opt dns $DNS2" >> $CONF_FILE
		fi
		eval `$GETMIB DNS3`
		if [ "$DNS3" != "0.0.0.0" ]; then
			echo "opt dns $DNS3" >> $CONF_FILE
		fi
	fi
	# set default
	if [ "`cat $CONF_FILE | grep dns`" = "" ]; then
		echo "opt dns $IP_ADDR"  >> $CONF_FILE
	fi
	
	eval `$GETMIB DOMAIN_NAME`	
	if [ "$DOMAIN_NAME" != "" ]; then
		echo "opt domain $DOMAIN_NAME" >> $CONF_FILE
	fi	
}

rtl_set_rsvdip() {
	eval `$GETMIB DHCPRSVDIP_ENABLED`
	if [ $DHCPRSVDIP_ENABLED != 0 ]; then
		eval `$GETMIB DHCPRSVDIP_TBL_NUM`
		if [ $DHCPRSVDIP_TBL_NUM -gt 0 ]; then
			num=1
			while [ $num -le $DHCPRSVDIP_TBL_NUM ];
			do
			DHCPRSVDIP_ENT=`$GETMIB DHCPRSVDIP_TBL | grep DHCPRSVDIP_TBL$num=`
			tmp=`echo $DHCPRSVDIP_ENT | cut -f2 -d=`    		
			MAC=`echo $tmp | cut -f1 -d,`
			IP=`echo $tmp | cut -f2 -d,`
			NAME=`echo $tmp | cut -f3 -d,`
	#			echo "static_lease $MAC $IP $NAME" >> $CONF_FILE 
				echo "static_lease $MAC $IP" >> $CONF_FILE 
				num=`expr $num + 1`
			done
		fi
	fi
}

#Keep the leases file
#if [ -f "$LEASE_FILE" ]; then
#	rm -f $LEASE_FILE
#fi
#echo "" > $LEASE_FILE

rtl_set_ip() {
	eval `$GETMIB IP_ADDR`
	fixedip.sh $PARAM1 $IP_ADDR $SUBNET_MASK 0.0.0.0
}


rtl_dhcpd() {
	#check wheher the dhcp server is enable	
	eval `$GETMIB DHCP`
	if [ "$DHCP" != '2' ]; then
		exit 0
	fi

	rtl_set_dhcpd_conf
	
	if [ $PARAM2 = "ap" ]; then
		rtl_set_dhcpd_ap
	else
		rtl_set_dhcpd_gw
	fi

	rtl_set_rsvdip
	rtl_set_ip

	udhcpd $CONF_FILE
}

rtl_dhcpd




