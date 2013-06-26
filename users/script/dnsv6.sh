#!/bin/sh
#dnsmasq can replace the dnrd.we should kill dnrd if needed
RM="rm -f"
KILL="kill -9"
DNSV6_PID_FILE=/var/run/dnsmasq.pid
DNRD_PID_FILE=/var/run/dnrd.pid
DNSV6_CONF_FILE=/var/dnsmasq.conf
RESOLV_CONF_FILE=/etc/resolv.conf
DNSV6_PARAM=`flash get IPV6_DNSV6_PARAM |cut -f2 -d=`
DNSV6_ENABLED=`echo $DNSV6_PARAM |cut -f1 -d,`
DNSV6_ENABLED=`echo $DNSV6_ENABLED`

rtl_create_dnsv6_conf() {
#need to create conf file according flash setting.since it it called from init
	RouterName=`echo $DNSV6_PARAM | cut -f2 -d,`
	RouterName=`echo $RouterName`
	#get RouterIPv6Addr

#	IPv6Addr=`ifconfig eth1 | grep -i "inet6 addr:"`
#	IPv6Addr=`echo $IPv6Addr | cut -f2 -d:`
	
	MAC_ADDR=`flash get ELAN_MAC_ADDR |cut -f2 -d=`
	if [ $MAC_ADDR -eq 0 ]; then
		MAC_ADDR=`flash get HW_NIC0_ADDR |cut -f2 -d=`
	fi

	IPv6AddrPart1="Fe80"
	IPv6AddrPart2=`echo $MAC_ADDR |cut -c 3-4`
	first_oct=`echo $MAC_ADDR |cut -c 1`
	second_oct=`echo $MAC_ADDR |cut -c 2`
	if [ $second_oct = '0' ]; then
		second_oct='2'
	elif [ $second_oct = '1' ]; then
		second_oct='3'
	elif [ $second_oct = '4' ]; then
		second_oct='6'
	elif [ $second_oct = '5' ]; then
		second_oct='7'
	elif [ $second_oct = '8' ]; then
		second_oct='a'
	elif [ $second_oct = '9' ]; then
		second_oct='b'
	elif [ $second_oct = 'c' ]; then
		second_oct='e'
	elif [ $second_oct = 'd' ]; then
		second_oct='f'
	fi
	IPv6AddrPart2=$first_oct$second_oct$IPv6AddrPart2
	IPv6AddrPart3=`echo $MAC_ADDR |cut -c 5-6`
	IPv6AddrPart3=$IPv6AddrPart3"FF"
	IPv6AddrPart4=`echo $MAC_ADDR |cut -c 7-8`
	IPv6AddrPart4="FE"$IPv6AddrPart4
	IPv6AddrPart5=`echo $MAC_ADDR |cut -c 9-12`
	IPv6Addr="$IPv6AddrPart1::$IPv6AddrPart2:$IPv6AddrPart3:$IPv6AddrPart4:$IPv6AddrPart5"
#create conf file
	echo "domain-needed" >> $DNSV6_CONF_FILE 
	echo "bogus-priv" >> $DNSV6_CONF_FILE 
	if [ -f $RESOLV_CONF_FILE ]; then
		echo "resolv-file=$RESOLV_CONF_FILE" >> $DNSV6_CONF_FILE 
	fi
	echo "#strict-order" >> $DNSV6_CONF_FILE 
	echo "#no-resolv" >> $DNSV6_CONF_FILE 
	echo "#no-poll" >> $DNSV6_CONF_FILE 
	echo "address=/$RouterName/$IPv6Addr" >> $DNSV6_CONF_FILE
	echo "#listen-address=" >> $DNSV6_CONF_FILE
	echo "#bind-interfaces" >> $DNSV6_CONF_FILE
	echo "#no-hosts" >> $DNSV6_CONF_FILE
}


rtl_start_dnsv6() {
	if [ "$DNSV6_ENABLED" = "1" ]; then
		if [ -f $DNRD_PID_FILE ]; then
			PID=`cat $DNRD_PID_FILE`
			$KILL $PID
			$RM $DNRD_PID_FILE
		fi
		dnsmasq -C /var/dnsmasq.conf
	fi
}


rtl_restart_dnsv6() {
	PID=`cat $DNSV6_PID_FILE`
	if [ "$DNSV6_ENABLED" = "1" ]; then
		$KILL $PID
		$RM $DNSV6_PID_FILE
		if [ -f $DNRD_PID_FILE ]; then
                        PID=`cat $DNRD_PID_FILE`
                        $KILL $PID
			$RM $DNRD_PID_FILE
                fi
		dnsmasq -C /var/dnsmasq.conf
	else
		$KILL $PID
	fi
}


rtl_dnsv6() {
	if [ ! -f $DNSV6_CONF_FILE ]; then
		rtl_create_dnsv6_conf	
	fi

	if [ ! -f $DNSV6_PID_FILE ]; then
		rtl_start_dnsv6
	else
		rtl_restart_dnsv6
	fi
}

rtl_dnsv6
	
