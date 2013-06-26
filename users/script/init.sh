#!/bin/sh
#
# script file to start network
#
# Usage: init.sh {gw | ap} {all | bridge | wan}
#

if [ $# -lt 2 ]; then echo "Usage: $0 {gw | ap} {all | bridge | wan}"; exit 1 ; fi

TOOL=flash
GETMIB="$TOOL get"
LOADDEF="$TOOL default"
LOADDEFSW="$TOOL default-sw"
LOADDS="$TOOL reset"
SET_IP=fixedip.sh
START_DHCP_SERVER=dhcpd.sh
START_DHCP_CLIENT=dhcpc.sh
START_BRIDGE=bridge.sh
START_WLAN=wlan.sh
START_PPPOE=pppoe.sh
START_FIREWALL=firewall.sh
START_WLAN_APP=wlanapp.sh
START_PPTP=pptp.sh
START_L2TP=l2tp.sh
START_NTP=ntp.sh
START_DDNS=ddns.sh
START_IP_QOS=ip_qos.sh
WLAN_PREFIX=wlan
PLUTO_PID=/var/run/pluto.pid
RESOLV_CONF="/etc/resolv.conf"
START_LAN_BRIDGE=lan_bridge.sh
START_SAMBA=samba.sh
SET_TIME=/var/system/set_time
## for wapi
CA_CERT="/var/myca/CA.cert"
CA4AP_CERT="/var/myca/ca4ap.cert"
AP_CERT="/var/myca/ap.cert"

# rock: VOIP_SUPPORT is decided by user/script/Makefile
###VOIP_SUPPORT###


WLAN_INTERFACE=
NUM_INTERFACE=
VIRTUAL_NUM_INTERFACE=
VIRTUAL_WLAN_INTERFACE=

# Query number of wlan interface
rtl_query_wlan_if() {
	NUM=0
	VIRTUAL_NUM=0
	VIRTUAL_WLAN_PREFIX=
	V_DATA=
	V_LINE=
	V_NAME=
	HAS_WLAN=0

	DATA=`ifconfig -a | grep $WLAN_PREFIX`
	LINE=`echo $DATA | grep $WLAN_PREFIX$NUM`
	NAME=`echo $LINE | cut -b -5`
	if [ -n "$NAME" ]; then
		HAS_WLAN=1
	fi
	while [ -n "$NAME" ] 
	do
		WLAN_INTERFACE="$WLAN_INTERFACE $WLAN_PREFIX$NUM"

		VIRTUAL_WLAN_PREFIX="$WLAN_PREFIX$NUM-va"
		V_DATA=`ifconfig -a | grep $VIRTUAL_WLAN_PREFIX`
		V_LINE=`echo $V_DATA | grep $VIRTUAL_WLAN_PREFIX$VIRTUAL_NUM`
		V_NAME=`echo $V_LINE | cut -b -9`
		while [ -n "$V_NAME" ] 
		do
			VIRTUAL_WLAN_INTERFACE="$VIRTUAL_WLAN_INTERFACE $VIRTUAL_WLAN_PREFIX$VIRTUAL_NUM"
			VIRTUAL_NUM=`expr $VIRTUAL_NUM + 1`
			V_LINE=`echo $V_DATA | grep $VIRTUAL_WLAN_PREFIX$VIRTUAL_NUM`
			V_NAME=`echo $V_LINE | cut -b -9`
		done
		
		VXD_INTERFACE="$WLAN_PREFIX$NUM-vxd"
		VIRTUAL_WLAN_INTERFACE="$VIRTUAL_WLAN_INTERFACE $VXD_INTERFACE"
				
		NUM=`expr $NUM + 1`
		LINE=`echo $DATA | grep $WLAN_PREFIX$NUM`
		NAME=`echo $LINE | cut -b -5`
	done
	NUM_INTERFACE=$NUM
	VIRTUAL_NUM_INTERFACE=$VIRTUAL_NUM
}


# See if flash data is valid
rtl_check_flash_data() {
	$TOOL test-hwconf
	if [ $? != 0 ]; then
		echo 'HW configuration invalid, reset default!'
		$LOADDEF
	fi

	$TOOL test-dsconf
	if [ $? != 0 ]; then
		echo 'Default configuration invalid, reset default!'
		$LOADDEFSW
	fi

	$TOOL test-csconf
	if [ $? != 0 ]; then
		echo 'Current configuration invalid, reset to default configuration!'
		$LOADDS
	fi
}
#check flash data before do flash command such as "flash get"
rtl_check_flash_data

rtl_check_voip_support() {
	# voip flash check
	if [ "$VOIP_SUPPORT" != "" ]; then
	$TOOL voip check
	fi
}

rtl_set_time() {
	if [ ! -r "$SET_TIME" ]; then
		flash settime
	fi
}

# WAPI CA
# $TOOL wapi-check
rtl_check_wapi_support() {
###CONFIG_RTL_WAPI_SUPPORT###
###CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT###
	if [ "$CONFIG_RTL_WAPI_SUPPORT" = "y" ]; then
	    if [ "$CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT" = "y" ]; then
		if [ ! -f "$CA_CERT" ]; then
			loadWapiFiles >/dev/null 2>&1
		fi
	    else
		if [ ! -f "$CA4AP_CERT" ] && [ ! -f "$AP_CERT" ]; then
			loadWapiFiles >/dev/null 2>&1
		fi
	    fi
	fi

}

eval `$GETMIB OP_MODE`
eval `$GETMIB WISP_WAN_ID`
echo "$OP_MODE" > /var/sys_op


GATEWAY=
BR_INTERFACE=
BR_LAN1_INTERFACE=
BR_LAN2_INTERFACE=

PARA1=$1
rtl_check_para_1() {
	if [ $PARA1 = 'ap' ]; then
### bridge (eth0+wlan0) confiuration #########
		GATEWAY='false'
		BR_INTERFACE=br0
		BR_LAN1_INTERFACE=eth0	
		if [ "$OP_MODE" = '1' ];then
			BR_LAN2_INTERFACE=eth1
		fi	
##############################################
	fi

	if [ "$VOIP_SUPPORT" != "" ] && [ $PARA1 = 'ATA867x' ]; then
		GATEWAY='false'
		BR_INTERFACE=eth0
		BR_LAN1_INTERFACE=eth0	
		if [ "$OP_MODE" = '1' ];then
			BR_LAN2_INTERFACE=eth1
		fi
	fi

	if [ $PARA1 = 'gw' ]; then
### gateway (eth0+eth1+wlan0) configuration ##
		GATEWAY='true'
		if [ "$OP_MODE" = '2' ];then
			WAN_INTERFACE=wlan$WISP_WAN_ID
		else
			WAN_INTERFACE=eth1
		fi
		BR_INTERFACE=br0	
		BR_LAN1_INTERFACE=eth0
		if [ "$OP_MODE" = '1' ] || [ "$OP_MODE" = '2' ]; then
			BR_LAN2_INTERFACE=eth1
		fi
##############################################
	fi
}



VLAN_INTERFACE=
rtl_check_vlan_support() {
# do not edit the next line
###CONFIG_RTK_VLAN_SUPPORT###
	if [ "$CONFIG_RTK_VLAN_SUPPORT" = "y" ]; then
	eval `$GETMIB VLANCONFIG_ENABLED`

	if [ $VLANCONFIG_ENABLED != 0 ]; then
		VLAN_INTERFACE='eth2 eth3 eth4'
	else
		VLAN_INTERFACE=
	fi

	else
		VLAN_INTERFACE=
	fi
}

BR_LAN_INTERFACES=
rtl_check_multi_lan_dev_support() {
# do not edit the next line
###CONFIG_RTL_MULTI_LAN_DEV###
        if [ "$CONFIG_RTL_MULTI_LAN_DEV" = "y" ]; then
                BR_LAN_INTERFACES='eth0 eth2 eth3 eth4'
        fi
}
 

ENABLE_WAN=
ENABLE_BR=
PARA2=$2
PARA0=$0
rtl_check_para_2() {
	if [ $PARA2 = 'all' ]; then
		ENABLE_WAN=1
		ENABLE_BR=1
	elif [ $PARA2 = 'wan' ]; then
		ENABLE_WAN=1
		# for miniigd patch	
		ENABLE_BR=1
	elif [ $PARA2 = 'bridge' ]; then
		# if WISP mode , restart wan  for pppoe  ,pptp
		# if [ "$OP_MODE" = '2' ]; then 
			#ENABLE_WAN=1
		#else
			#ENABLE_WAN=0
		#fi
		# for miniigd patch	
		ENABLE_WAN=1
		ENABLE_BR=1
		elif [ $PARA2 = 'wlan_app' ]; then
		$START_WLAN_APP start $WLAN_INTERFACE $BR_INTERFACE
		exit 0	
	else
		echo "Usage: PARA0 {all | bridge | wan}"; exit 1
	fi
}

# Cleanup hardware tables
rtl_clean_hw_table() {
# do not edit next line, it will be replaced to "CONFIG_RTL_HW_NAPT=y" or "CONFIG_RTL_HW_NAPT=" by user/script/Makfile
###CONFIG_RTL_HW_NAPT###
	if [ "$GATEWAY" = 'true' ]; then
		if [ "$CONFIG_RTL_HW_NAPT" = "y" ]; then
			if [ "$OP_MODE" = '0' ]; then
				# gateway mode
				echo "1" > /proc/hw_nat
			elif [ "$OP_MODE" = '1' ]; then
				# bridge mode
				echo "2" > /proc/hw_nat
			elif [ "$OP_MODE" = '2' ]; then
				# wisp mode
				echo "3" > /proc/hw_nat		 
			elif [ "$OP_MODE" = '3' ]; then	
				# bridge mode with multiple vlan
				echo "4" > /proc/hw_nat		 
			else	
				# wisp mode with multiple vlan
				echo "5" > /proc/hw_nat		 
			fi
		else
			echo "$OP_MODE" > /proc/sw_nat
		fi
	fi	
}

# Generate WPS PIN number
rtl_generate_wps_pin() {
	eval `$GETMIB HW_WSC_PIN`
	if [ "$HW_WSC_PIN" = "00000000" ]; then
		$TOOL gen-pin
	fi
}

rtl_set_mac_addr() {
	# Set Ethernet 0 MAC address
	eval `$GETMIB ELAN_MAC_ADDR`
	if [ "$ELAN_MAC_ADDR" = "000000000000" ]; then
		eval `$GETMIB HW_NIC0_ADDR`
		ELAN_MAC_ADDR=$HW_NIC0_ADDR
	fi
	if [ "$CONFIG_RTL_MULTI_LAN_DEV" = "y" ]; then
		ifconfig eth0 hw ether $ELAN_MAC_ADDR
		ifconfig eth2 hw ether $ELAN_MAC_ADDR
		ifconfig eth3 hw ether $ELAN_MAC_ADDR
		ifconfig eth4 hw ether $ELAN_MAC_ADDR
        else
		ifconfig $BR_LAN1_INTERFACE hw ether $ELAN_MAC_ADDR
	fi

	# Set Ethernet 1 MAC Address for bridge mode and WISP
	eval `$GETMIB ELAN_MAC_ADDR`
	if [ "$OP_MODE" = '1' ]  || [ "$OP_MODE" = '2' ]; then
		if [ "$ELAN_MAC_ADDR" = "000000000000" ]; then
			eval `$GETMIB HW_NIC1_ADDR`
			ELAN_MAC_ADDR=$HW_NIC1_ADDR
		fi
	#	if [ "$OP_MODE" = '2' ]; then  #wireless ISP
		if [ "$BR_LAN2_INTERFACE" != '' ]; then
			ifconfig $BR_LAN2_INTERFACE hw ether $ELAN_MAC_ADDR
		fi
	#	fi
	fi

	# Disable DELAY_RX in Ethernet driver when do WIFI test
	#eval `$GETMIB WIFI_SPECIFIC`
	#if [  "$WIFI_SPECIFIC" = 1 ]; then
	#	echo 1 > /proc/eth_flag
	#fi

	#don't setup WAN in bridge mode
	if [ "$GATEWAY" = 'true' ] && [ "$OP_MODE" != '1' ] ; then 
		eval `$GETMIB WAN_MAC_ADDR`
		if [ "$WAN_MAC_ADDR" = "000000000000" ]; then
			if [ "$OP_MODE" = '2' ]; then  #wireless ISP, use the WLAN mac address
				eval `$GETMIB wlan$WISP_WAN_ID HW_WLAN_ADDR`
				WAN_MAC_ADDR=$HW_WLAN_ADDR		
			else
				eval `$GETMIB HW_NIC1_ADDR`
				WAN_MAC_ADDR=$HW_NIC1_ADDR
			fi
		fi
		ifconfig $WAN_INTERFACE hw ether $WAN_MAC_ADDR
	fi
}


# for miniigd patch	
rtl_patch_miniigd() {
	if [ "$GATEWAY" = 'true' ]; then
		IGD_PID_FILE=/var/run/miniupnpd.pid
		killall -15 miniigd 2> /dev/null
		if [ -f "$IGD_PID_FILE" ]; then
			rm -f $IGD_PID_FILE
		fi
		ROUTED_PID_FILE=/var/run/routed.pid
		killall -15 routed 2> /dev/null
		if [ -f "$ROUTED_PID_FILE" ]; then
			rm -f $ROUTED_PID_FILE
		fi
	fi
}

rtl_disconnect_all() {
	if [ $ENABLE_WAN = 1 -a "$GATEWAY" = 'true' ]; then  #disconnect all wan  for vpn and wisp
		# stop vpn if enabled
		if [ -f $PLUTO_PID ];then
			ipsec setup stop
		fi	
		killall -9 pptp.sh 2> /dev/null
		killall -9 pppoe.sh 2> /dev/null
		killall -9 l2tp.sh 2> /dev/null
		rm -f /etc/ppp/first*
		disconnect.sh all
	fi
}


# Start WLAN interface
rtl_start_wlan_if() {
	NUM=0
	while [ $NUM -lt $NUM_INTERFACE -a $ENABLE_BR = 1  ]
	do
		echo 'Initialize '$WLAN_PREFIX$NUM' interface'
		ifconfig $WLAN_PREFIX$NUM down	
		$TOOL set_mib $WLAN_PREFIX$NUM
		if [ $? != 0 ] ; then
			echo 'Using wlan script...'
			$START_WLAN $WLAN_PREFIX$NUM
		fi
		RP_NAME=-vxd
		ifconfig $WLAN_PREFIX$NUM$RP_NAME down
		$TOOL set_mib $WLAN_PREFIX$NUM$RP_NAME
		if [ $? != 0 ] ; then
			echo 'Using wlan script...'
			iwpriv $WLAN_PREFIX$NUM$RP_NAME copy_mib		
			$START_WLAN $WLAN_PREFIX$NUM$RP_NAME
		fi	

		# Start VIRTUAL WLAN interface
		VIRTUAL_NUM=0
		while [ $VIRTUAL_NUM -lt $VIRTUAL_NUM_INTERFACE -a $ENABLE_BR = 1  ]
		do
			echo 'Initialize '$WLAN_PREFIX$NUM-va$VIRTUAL_NUM' interface'
			ifconfig $WLAN_PREFIX$NUM-va$VIRTUAL_NUM down
			$TOOL set_mib $WLAN_PREFIX$NUM-va$VIRTUAL_NUM
			if [ $? != 0 ] ; then
				echo 'Using wlan script...'
				iwpriv $WLAN_PREFIX$NUM-va$VIRTUAL_NUM copy_mib			
				$START_WLAN $WLAN_PREFIX$NUM-va$VIRTUAL_NUM
			fi
			VIRTUAL_NUM=`expr $VIRTUAL_NUM + 1`
		done

		NUM=`expr $NUM + 1`
	done		
}


# check repeater interface for wlan0
V_WLAN_APP_ENABLE=
rtl_check_rept_if() {
	WLAN_INTERFACE_REPEATER=$WLAN_INTERFACE
	eval `$GETMIB REPEATER_ENABLED1`
	eval `$GETMIB wlan0 MODE`
	eval `$GETMIB wlan0 NETWORK_TYPE`
	ifconfig wlan0-vxd down
	if  [ "$REPEATER_ENABLED1" != 0 ] && [ "$MODE" != 2 ]; then
		if [ "$MODE" != 1 ] || [ "$MODE" = 1  -a "$NETWORK_TYPE" != 1 ]; then
			WLAN_INTERFACE_REPEATER="$WLAN_INTERFACE_REPEATER wlan0-vxd"	
		fi
	fi

	# check repeater interface for wlan1
	if [ $HAS_WLAN =  1 ]; then
		if [ $NUM_INTERFACE -lt 1  ]; then
			ifconfig wlan1-vxd down
			eval `$GETMIB wlan1 MODE`
			eval `$GETMIB wlan1 NETWORK_TYPE`	
			eval `$GETMIB REPEATER_ENABLED2`
			if  [ "$REPEATER_ENABLED2" != 0 ] && [ "$MODE" != 2 ]; then
				if [ "$MODE" != 1 ] || [ "$MODE" = 1  -a "$NETWORK_TYPE" != 1 ]; then
					WLAN_INTERFACE_REPEATER="$WLAN_INTERFACE_REPEATER wlan1-vxd"	
				fi
			fi
		fi

		V_WLAN_APP_ENABLE=1
		eval `$GETMIB wlan0 MODE`
		eval `$GETMIB REPEATER_ENABLED1`
		eval `$GETMIB REPEATER_ENABLED2`
		if  [ "$MODE" != 0 ] && [ "$MODE" != 3 ] && [ "$REPEATER_ENABLED1" = 0 ] && 
				[ "$REPEATER_ENABLED2" = 0 ]; then
			V_WLAN_APP_ENABLE=0
		fi
	fi
}


rtl_start_gw() {
#if [ "$GATEWAY" = 'true' ]; then
	if [ $ENABLE_BR = 1 ]; then
		echo 'Setup BRIDGE interface'
		PIDFILE=/etc/udhcpc/udhcpc-$BR_INTERFACE.pid
		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			if [ $PID != 0 ]; then
				kill -9 $PID
			fi
			rm -f $PIDFILE
		fi
		#kill syslogd and klogd
		killall syslogd
		killall klogd
		#Initialize bridge interface
        	if [ "$CONFIG_RTL_MULTI_LAN_DEV" = "y" ]; then
			$START_BRIDGE $BR_INTERFACE $BR_LAN_INTERFACES $WLAN_INTERFACE $BR_LAN2_INTERFACE $VIRTUAL_WLAN_INTERFACE
        	else
			#$START_BRIDGE $BR_INTERFACE $BR_LAN1_INTERFACE $VLAN_INTERFACE $WLAN_INTERFACE $BR_LAN2_INTERFACE $VIRTUAL_WLAN_INTERFACE
			$START_BRIDGE $BR_INTERFACE $BR_LAN1_INTERFACE $VLAN_INTERFACE $WLAN_INTERFACE $VIRTUAL_WLAN_INTERFACE
		fi
		eval `$GETMIB SCRLOG_ENABLED`
		if [ $SCRLOG_ENABLED != 0 -a $SCRLOG_ENABLED != 2 ] &&
			[ $SCRLOG_ENABLED != 4 -a $SCRLOG_ENABLED != 6 ] &&
			[ $SCRLOG_ENABLED != 8 -a $SCRLOG_ENABLED != 10 ] &&
			[ $SCRLOG_ENABLED != 12 -a  $SCRLOG_ENABLED != 14 ]; then			
			eval `$GETMIB REMOTELOG_ENABLED`
			eval `$GETMIB REMOTELOG_SERVER`
			if [ $REMOTELOG_ENABLED = "1" ] ;then
				SYSLOG_PARA="-R $REMOTELOG_SERVER"
			fi 
			syslogd -L $SYSLOG_PARA &
			klogd &
		fi

		# Set fixed IP or start DHCP server
		PIDFILE=/var/run/udhcpd.pid
		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			if [ $PID != 0 ]; then
				kill -16 $PID
				sleep 1
				kill -9 $PID
	        	fi
        		rm -f $PIDFILE
		fi

		eval `$GETMIB DHCP`
		if [ "$DHCP" = '0' ]; then
			eval `$GETMIB IP_ADDR`
			eval `$GETMIB SUBNET_MASK`
			eval `$GETMIB DEFAULT_GATEWAY`			
			$SET_IP $BR_INTERFACE $IP_ADDR $SUBNET_MASK $DEFAULT_GATEWAY
			if [ $HAS_WLAN = 1 ]; then
			if [ $V_WLAN_APP_ENABLE = 1 ]; then
				$START_WLAN_APP start $WLAN_INTERFACE $VIRTUAL_WLAN_INTERFACE $BR_INTERFACE
			else
				$START_WLAN_APP start $WLAN_INTERFACE $BR_INTERFACE
			fi
			fi
		elif [ "$DHCP" = '2' ]; then		
			# caculate wait time
			NUM=0
			WAIT_TIME=0
			while [ $NUM -lt $NUM_INTERFACE  ]
			do				
				eval `$GETMIB $WLAN_PREFIX$NUM WDS_ENABLED`				
				if [ "$WDS_ENABLED" != 0 ]; then
					WAIT_TIME=`expr $WAIT_TIME + 5`
				else				
					WAIT_TIME=`expr $WAIT_TIME + 1`
				fi				
				NUM=`expr $NUM + 1`
			done				
			sleep $WAIT_TIME
			
			$START_DHCP_SERVER $BR_INTERFACE gw
			if [ $HAS_WLAN = 1 ]; then
			if [ $V_WLAN_APP_ENABLE = 1 ]; then
				$START_WLAN_APP start $WLAN_INTERFACE $VIRTUAL_WLAN_INTERFACE $BR_INTERFACE
			else
				$START_WLAN_APP start $WLAN_INTERFACE $BR_INTERFACE
			fi
			fi
		fi

	#kill cwmp-tr069
	PIDFILE=/var/run/cwmp.pid
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
			kill -15 $PID
			sleep 1
	        fi
	        rm -f $PIDFILE
        fi
	fi

	# for pppoe/l2tp/pptp re-init
	iptables -F
	iptables -F -t nat
	iptables -A INPUT -j ACCEPT

	if [ $ENABLE_WAN = 1 ]; then
		if [ "$OP_MODE" != '1' ];then
			echo 'Setup WAN interface'
		fi		

		# Initialize WAN interface
		# Delete DHCP client process
		DHCPC_WAN="$WLAN_INTERFACE eth1"
		for INTF in $DHCPC_WAN ; do
			PIDFILE=/etc/udhcpc/udhcpc-$INTF.pid
			if [ -f $PIDFILE ] ; then
				PID=`cat $PIDFILE`
				if [ $PID != 0 ]; then
					kill -9 $PID
				fi
				rm -f $PIDFILE
			fi
		done

		PIDFILE=/var/run/dnrd.pid
		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			if [ $PID != 0 ]; then
				kill -9 $PID
	        	fi
        		rm -f $PIDFILE
		fi
		PIDFILE=/var/run/l2tpd.pid
		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			if [ $PID != 0 ]; then
				kill -9 $PID
	        	fi
        		rm -f $PIDFILE
		fi
		eval `$GETMIB WAN_DHCP`
		rm -f  /var/eth1_ip 2> /dev/null
		rm -f /var/ntp_run 2> /dev/null
		#echo "0 0" > /proc/pptp_src_ip
		if [ "$OP_MODE" != '1' ];then  # not bridge mode
		
################################################################		
# When WISP mode and WPA/WPA2 enabled, set keep_rsnie mib before 
# reinit wlan interface
			if [ "$OP_MODE" = '2' ];then		
				eval `$GETMIB ENCRYPT`
				if [ "$ENCRYPT" != "0" ]; then
					iwpriv $WAN_INTERFACE set_mib keep_rsnie=1
				fi
			fi
##################################################### 12-18-2006
			
			ifconfig $WAN_INTERFACE down
			ifconfig $WAN_INTERFACE up
						
			# Realtek fast pptp forwarding
			if [ $WAN_DHCP = 4 ]; then
				echo "1" > /proc/fast_pptp
				eval `$GETMIB PPTP_CONNECTION_TYPE`
				if [ $PPTP_CONNECTION_TYPE = 1 ]; then
					echo "3" > /proc/pptp_conn_ck
				else
					echo "0" > /proc/pptp_conn_ck
				fi
			else
				echo "0" > /proc/fast_pptp
			fi						
						
			# Enable Realtek fast-l2tp
			if [ $WAN_DHCP = 6 ]; then
				echo "1" > /proc/fast_l2tp
			else
				echo "0" > /proc/fast_l2tp
			fi						
						
			if [ "$WAN_DHCP" = '0' ]; then
				eval `$GETMIB WAN_IP_ADDR`
				eval `$GETMIB WAN_SUBNET_MASK`
				eval `$GETMIB WAN_DEFAULT_GATEWAY`
				eval `$GETMIB FIXED_IP_MTU_SIZE`				
				$SET_IP $WAN_INTERFACE $WAN_IP_ADDR $WAN_SUBNET_MASK $WAN_DEFAULT_GATEWAY
				ifconfig $WAN_INTERFACE mtu $FIXED_IP_MTU_SIZE					

				# start DNS relay
				eval `$GETMIB DNS1`
				DNS="--cache=off"
				if [ "$DNS1" != '0.0.0.0' ]; then
					DNS="$DNS -s $DNS1"
					echo nameserver $DNS1 > $RESOLV_CONF
				fi
				eval `$GETMIB DNS2`
				if [ "$DNS2" != '0.0.0.0' ]; then
					DNS="$DNS -s $DNS2"
					echo nameserver $DNS2 >> $RESOLV_CONF
				fi
				eval `$GETMIB DNS3`
				if [ "$DNS3" != '0.0.0.0' ]; then
					DNS="$DNS -s $DNS3"
					echo nameserver $DNS3 >> $RESOLV_CONF
				fi
				echo start DNS Relay Daemon
				dnrd $DNS

				# for miniigd patch	
				upnp.sh init
				
			elif [ "$WAN_DHCP" = '1' ]; then				
				eval `$GETMIB DHCP_MTU_SIZE`
				ifconfig $WAN_INTERFACE mtu $DHCP_MTU_SIZE
				$START_DHCP_CLIENT $WAN_INTERFACE wait&				
				
				# for miniigd patch	
				upnp.sh init
			elif [ "$WAN_DHCP" = '3' ]; then
				echo 'start PPPoE daemon'
				
				# for miniigd patch	
				upnp.sh init
				$START_PPPOE all $WAN_INTERFACE
			elif [ "$WAN_DHCP" = '4' ]; then
				echo 'start PPTP daemon'
				
				# for miniigd patch	
				upnp.sh init
				$START_PPTP $WAN_INTERFACE &
			elif [ "$WAN_DHCP" = '6' ]; then #-- keith: add l2tp support. 20080515
				echo 'start L2TP daemon'
				
				# for miniigd patch	
				upnp.sh init
				$START_L2TP $WAN_INTERFACE &	
			else
				echo 'Invalid DHCP MIB value for WAN interface!'
			fi

		fi

		# enable firewall when static ip
		if [ "$WAN_DHCP" = '0' ] || [ "$OP_MODE" = '1' ]; then
			echo 'Setup Firewall'
			#echo 2 > /proc/fast_nat 
			$START_FIREWALL
		fi
		# static ip 	
		if [ -f /bin/vpn.sh ] && [ "$WAN_DHCP" != '4' ] && [ "$WAN_DHCP" != '6' ] && 
				[ "$OP_MODE" != '1' ] &&  [ "$WAN_DHCP" = '0' ] ; then #-- keith: add l2tp support. 20080515
			echo 'Setup VPN'
			vpn.sh all
		fi

		#enable ntp daemon , not brige mode
		if [ "$OP_MODE" != '1' ] && [ "$WAN_DHCP" = 0 ]; then
			killall -9 $START_NTP 2> /dev/null
			killall -9 $START_DDNS 2> /dev/null
			$START_NTP
			$START_DDNS all &
		fi
		
		# enable the traffic control settings when OP_MODE is not equal to the bridge mode
		# do not edit next line, it will be replaced to "CONFIG_NET_SCHED=y" or "CONFIG_NET_SCHED=" by user/script/Makefile
		###CONFIG_NET_SCHED###
#		if [ "$OP_MODE" != '1' ]; then
#			if [ "$CONFIG_NET_SCHED" = "y" ]; then
#				echo "init ip_qos.sh"
#				$START_IP_QOS
#			fi
#		fi
                
	fi
	# set kthreadd high priority for performance
	renice -20 2
	# set ksoftirqd high priority for performance
	renice -20 3
	# set webs high priorit
	THREADS=`ps | grep "webs"`
	WEBSPID=`echo $THREADS | cut -f1 -d' '`
	renice -20 $WEBSPID
}
#end of rtl_start_gw

rtl_start_no_gw() {

	# Delete DHCP server/client process
	PIDFILE=/etc/udhcpc/udhcpc-$BR_INTERFACE.pid
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
			kill -9 $PID
	       	fi
      		rm -f $PIDFILE
	fi

	PIDFILE=/var/run/udhcpd.pid
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
			kill -16 $PID
				sleep 1
			kill -9 $PID
       		fi
      		rm -f $PIDFILE
	fi

	#kill syslogd and klogd
	killall syslogd
	killall klogd


	if [ "$OP_MODE" = '1' ];then
		if [ "$CONFIG_RTL_MULTI_LAN_DEV" = "y" ]; then
			$START_BRIDGE $BR_INTERFACE $BR_LAN_INTERFACES $WLAN_INTERFACE $BR_LAN2_INTERFACE $VIRTUAL_WLAN_INTERFACE
        	else
			#$START_BRIDGE $BR_INTERFACE $BR_LAN1_INTERFACE $VLAN_INTERFACE $WLAN_INTERFACE $BR_LAN2_INTERFACE $VIRTUAL_WLAN_INTERFACE
			$START_BRIDGE $BR_INTERFACE $BR_LAN1_INTERFACE $VLAN_INTERFACE $WLAN_INTERFACE $VIRTUAL_WLAN_INTERFACE
        	fi
	else	
        	if [ "$CONFIG_RTL_MULTI_LAN_DEV" = "y" ]; then
			$START_BRIDGE $BR_INTERFACE $BR_LAN_INTERFACES $WLAN_INTERFACE $VIRTUAL_WLAN_INTERFACE
		else	
			$START_BRIDGE $BR_INTERFACE $BR_LAN1_INTERFACE $VLAN_INTERFACE $WLAN_INTERFACE $VIRTUAL_WLAN_INTERFACE
		fi
	fi
	
	eval `$GETMIB SCRLOG_ENABLED`	
	if [ $SCRLOG_ENABLED != 0 -a $SCRLOG_ENABLED != 2 ] &&
			[ $SCRLOG_ENABLED != 4 -a $SCRLOG_ENABLED != 6 ] &&
			[ $SCRLOG_ENABLED != 8 -a $SCRLOG_ENABLED != 10 ] &&
			[ $SCRLOG_ENABLED != 12 -a  $SCRLOG_ENABLED != 14 ]; then			
		eval `$GETMIB REMOTELOG_ENABLED`
		eval `$GETMIB REMOTELOG_SERVER`
		if [ $REMOTELOG_ENABLED = "1" ] ;then
			SYSLOG_PARA="-R $REMOTELOG_SERVER"
		fi 
		syslogd -L $SYSLOG_PARA &
		klogd &
	fi

	eval `$GETMIB DHCP`
	if [ "$DHCP" = '2' ]; then
		sleep 1
		$START_DHCP_SERVER $BR_INTERFACE ap
	fi

	if [ "$VOIP_SUPPORT" != "" ] && [ "$WLAN_INTERFACE" = "" ]; then
	: # no wlan interface
	else
	if [ "$DHCP" = '0' ] || [ "$DHCP" = '2' ]; then
		if [ $HAS_WLAN = 1 ]; then
		if [ $V_WLAN_APP_ENABLE = 1 ]; then
			$START_WLAN_APP start $WLAN_INTERFACE $VIRTUAL_WLAN_INTERFACE $BR_INTERFACE
		else
			$START_WLAN_APP start $WLAN_INTERFACE $BR_INTERFACE
		fi
		fi
	fi
	fi

}
#end of rtl_start_no_gw

# start auto-discovery daemon

#PIDFILE=/var/run/disc_server.pid
#if [ -f $PIDFILE ] ; then
#	PID=`cat $PIDFILE`
#	if [ $PID != 0 ]; then
#		kill -9 $PID
#	fi
#	rm -f $PIDFILE
#fi

#eval `$GETMIB AUTODISCOVERY_ENABLED`
#if [ "$AUTODISCOVERY_ENABLED" != 0 ]; then
#	disc_server $BR_INTERFACE &
#fi


rtl_do_cmd() {
	if [ "$VOIP_SUPPORT" != "" ] && [ "$WLAN_INTERFACE" = "" ]; then
	# no wlan interface
	WSC_DISABLE=1
	else
	eval `$GETMIB WSC_DISABLE`
	fi

	_CMD=
	if [ $WSC_DISABLE = 0 ]; then
		_CMD="$_CMD -wsc /tmp/wscd_config"
	fi

	if [ "$GATEWAY" = 'true' ] && [ "$OP_MODE" != '1' ]; then
		eval `$GETMIB UPNP_ENABLED`
		if [ $UPNP_ENABLED != 0 ]; then
			_CMD="$_CMD -igd /tmp/igd_config"
		fi
	fi	
		
	if [ "$_CMD" != "" ]; then
		mini_upnpd $_CMD &
	fi	

	if [ "$GATEWAY" = 'true' ]; then
		if [ -f /bin/lld2d ]; then
		# for LLTD
			killall -9 lld2d 2>/dev/null
			rm /var/run/lld2d-br0.pid 2>/dev/null
			lld2d br0
		fi	
		# enable IGMP proxy daemon
		eval `$GETMIB IGMP_PROXY_DISABLED`
		if [ "$WAN_DHCP" = '0' ]; then	
			killall -9 igmpproxy 2> /dev/null
			rm -f /var/run/igmp_pid 2> /dev/null
			if [ "$IGMP_PROXY_DISABLED" = 0 ]; then
				if [ "$OP_MODE" = '0' ]; then
				# gateway mode
				igmpproxy $WAN_INTERFACE br0 &
				else
				# bridge mode or wisp mode
				igmpproxy $WAN_INTERFACE br0 &
				#igmpproxy br0 br0 &
				fi
				echo 1 > /proc/br_igmpsnoop
			else
				echo 0 > /proc/br_igmpsnoop
			fi
		fi
	fi
}

# SNMP daemon
rtl_start_snmpd() {
	if [ -f /bin/snmpd ]; then
		snmpd.sh restart
	fi
	#for mesh ; GANTOE
	# Wait for GANTOE fix netsnmp
	if [ -f /bin/snmpd  ]; then
		snmpd -c /etc/snmpd.conf -p /var/run/snmpd.pid
	fi

	if [ -f /bin/nmsd  ]; then
		nmsd
	fi
}

#echo 1 > /proc/wlan0/flow_stats
# Wait for GANTOE fix
#for mesh ; GANTOE
# start reload
rtl_reload() {
	killall -9 reload 2> /dev/null
	eval `$GETMIB WLAN_DISABLED`
	if [ "$WLAN_DISABLED" = 0 ]; then
		eval `$GETMIB SCHEDULE_ENABLED`
		if [ "$SCHEDULE_ENABLED" != 0 ]; then
			eval `$GETMIB SCHEDULE_TBL_NUM`
			if [ "$SCHEDULE_TBL_NUM" != 0 ]; then
				SCHEDULE_SELECT_IDX=0
				if [ "$SCHEDULE_SELECT_IDX" -lt "$SCHEDULE_TBL_NUM" ]; then
					SCHEDULE_SELECT_IDX=`expr $SCHEDULE_SELECT_IDX + 1`			
					SCH_TBL=`$GETMIB SCHEDULE_TBL | grep SCHEDULE_TBL$SCHEDULE_SELECT_IDX`
					tbl_comment=`echo $SCH_TBL | cut -f2 -d=`
					reload -e $tbl_comment &
				else
					reload &
				fi
			else
				reload &
			fi
		else
			reload &
		fi
	else
		reload &
	fi
}


rtl_check_ipv6() {
	#IPv6 init
	IPV6BASIC_INIT_FILE=/bin/ipv6BasicSetting.sh
	if [ -f $IPV6BASIC_INIT_FILE ]; then
		echo 'Start setting IPv6[IPv6]'
		echo "1" > /proc/sys/net/ipv6/conf/all/forwarding
		/bin/sh $IPV6BASIC_INIT_FILE
	fi
	DHCP6S_INIT_FILE=/bin/dhcp6s.sh
	if [ -f $DHCP6S_INIT_FILE ]; then
		echo 'Start dhcpv6[IPv6]' 
		/bin/sh $DHCP6S_INIT_FILE
	fi

	DNSV6_INIT_FILE=/bin/dnsv6.sh
	if [ -f $DNSV6_INIT_FILE ]; then
		echo 'Start dnsv6[IPv6]' 
		/bin/sh $DNSV6_INIT_FILE
	fi

	RADVD_INIT_FILE=/bin/radvd.sh
	if [ -f $RADVD_INIT_FILE ]; then
		echo 'Start radvd[IPv6]' 
		/bin/sh $RADVD_INIT_FILE
	fi
}

#eval `$GETMIB RTL_STP_ENABLED`
#if [ "$RTL_STP_ENABLED" = '1' ]; then
#	$START_LAN_BRIDGE
#fi

#To test wapi
#cp /certs/*.cer /var/certs/
#cp -rf /usr/local/ssl/* /var/myca/


rtl_check_samba() {
#       eval `$GETMIB SAMBA_ENABLED`
#       if [ "$SAMBA_ENABLED" = '1' ]; then
# do not edit the next line
###CONFIG_APP_SAMBA###
	if [ "$CONFIG_APP_SAMBA" = "y" ]; then
		$START_SAMBA
	fi
}

rtl_init() {

	rtl_query_wlan_if
#do check_flash_data before use flash cmd.
#	rtl_check_flash_data
	rtl_check_voip_support
	rtl_set_time
	rtl_check_wapi_support	
	rtl_check_para_1
	rtl_check_multi_lan_dev_support
	rtl_check_vlan_support
	rtl_check_para_2
	rtl_clean_hw_table
	rtl_set_mac_addr
	rtl_generate_wps_pin
	rtl_patch_miniigd
	rtl_disconnect_all
	rtl_start_wlan_if
	rtl_check_rept_if

	if [ "$GATEWAY" = 'true'  ]; then
		rtl_start_gw
	else
		rtl_start_no_gw
	fi

	killall -9 mini_upnpd 2> /dev/null


	rtl_do_cmd
	rtl_start_snmpd
	rtl_reload
	rtl_check_ipv6
	
	#Set the max connection in etc/profil
	#Set the max conneciton number 2048
	#echo "2048" > /proc/sys/net/ipv4/ip_conntrack_max
	#init gpio led
	echo "I" > /proc/gpio 2>/dev/null

	rtl_check_samba
}



rtl_init

