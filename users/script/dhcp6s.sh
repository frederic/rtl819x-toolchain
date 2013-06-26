#!/bin/sh
RM="rm -f"
KILL="kill -9"
DHCP6S_PID_FILE=/var/run/dhcp6s.pid
DHCP6S_CONF_FILE=/var/dhcp6s.conf


#ENABLE
DHCP6S_PARAM=`flash get IPV6_DHCPV6S_PARAM |cut -f2 -d=`
DHCP6S_ENABLED=`echo $DHCP6S_PARAM |cut -f1 -d,`
DHCP6S_ENABLED=`echo $DHCP6S_ENABLED`

#CONFIG FILE
rtl_check_dhcp6s_conf() {
	if [ ! -f $DHCP6S_CONF_FILE ]; then
	#param
		DnsName=`echo $DHCP6S_PARAM | cut -f2 -d, `
		Addr6PoolS=`echo $DHCP6S_PARAM | cut -f3 -d, `
		Addr6PoolE=`echo $DHCP6S_PARAM | cut -f4 -d, `
		InterfaceNameDS=`echo $DHCP6S_PARAM | cut -f5 -d, `
	#create file
		echo "option domain-name-servers $DnsName;" >> $DHCP6S_CONF_FILE	
		echo "interface $InterfaceNameDS {" >> $DHCP6S_CONF_FILE	
		echo "	address-pool pool1 3600;" >> $DHCP6S_CONF_FILE	
		echo "};" >> $DHCP6S_CONF_FILE	
		echo "pool pool1 {" >> $DHCP6S_CONF_FILE	
		echo "	range $Addr6PoolS to $Addr6PoolE ;" >> $DHCP6S_CONF_FILE	
		echo "};" >> $DHCP6S_CONF_FILE	
	fi
}

#START DEAMON
rtl_start_dhcp6s_daemon() {
	if [ ! -f $DHCP6S_PID_FILE ]; then
		if [ "$DHCP6S_ENABLED" = "1" ]; then
			/bin/dhcp6s $InterfaceNameDS
		fi
	else
		PID=`cat $DHCP6S_PID_FILE`
		if [ "$DHCP6S_ENABLED" = "1" ]; then
			$KILL $PID
			$RM $DHCP6S_PID_FILE
			/bin/dhcp6s $InterfaceNameDS
		else
			#kill the dhcp6 
			$KILL $PID
		fi
	fi
}

rtl_dhcp6s() {
	rtl_check_dhcp6s_conf
	rtl_start_dhcp6s_daemon
}

rtl_dhcp6s

