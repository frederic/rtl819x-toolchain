#!/bin/sh
#
# script file to start bridge
#
# Usage: bridge.sh br_interface lan1_interface wlan_interface[1]..wlan_interface[N]
#

# rock: VOIP_SUPPORT is decided by mkimg
###VOIP_SUPPORT###
if [ "$VOIP_SUPPORT" != "" ]; then
# rock: gw but no wlan case
if [ $# -lt 2 ]; then echo "Usage: $0 br_interface lan1_interface wlan_interface lan2_interface...";  exit 1 ; fi
else
if [ $# -lt 3 ]; then echo "Usage: $0 br_interface lan1_interface wlan_interface lan2_interface...";  exit 1 ; fi
fi


GETMIB="flash get"
BR_UTIL=brctl
SET_IP=fixedip.sh
START_DHCP_CLIENT=dhcpc.sh
IFCONFIG=ifconfig
WLAN_PREFIX=wlan
LAN_PREFIX=eth
MAX_WDS_NUM=8
INITFILE=/tmp/bridge_init

#set PARA for $i can't pass to function
PARA1=$1
PARA2=$2
PARA3=$3
PARA_ALL=$*



# do not edit the next line
###CONFIG_RTK_VLAN_SUPPORT###

# do not edit the next line
###CONFIG_RTL_MULTI_LAN_DEV###

rtl_shutdown_lan_if() {	
	# shutdown LAN interface (ethernt, wlan, WDS, bridge)
	for ARG in $PARA_ALL ; do
		INTERFACE=`echo $ARG | cut -b -4`
		if [ $INTERFACE = $WLAN_PREFIX ]; then	
			$IFCONFIG $ARG down						
			VXD=`echo $ARG | cut -b 7-`
			VAP=`echo $ARG | cut -b 7-8`
			if [ "$VXD" != "vxd" ] && [ "$VAP" != "va" ]; then
				num=0
				while [ $num -lt $MAX_WDS_NUM ]
				do
					$IFCONFIG $ARG-wds$num down
					num=`expr $num + 1`
					$BR_UTIL delif $PARA1 $ARG-wds$num 2> /dev/null
				done			
			fi	
			if [ "$VAP" = "va" ]; then
				$BR_UTIL delif $PARA1 $ARG 2> /dev/null
			fi
		else
			$IFCONFIG $ARG down	
			if [ $ARG != $PARA1 ]; then
				$BR_UTIL delif $PARA1 $ARG 2> /dev/null
			fi		
		fi		
	done
}

rtl_shutdown_vlan_if() {	
	#shut down vlan interface:eth2~eth4
	if [ "$CONFIG_RTK_VLAN_SUPPORT" = "y" ]; then
		$IFCONFIG eth2 down
		$IFCONFIG eth3 down
		$IFCONFIG eth4 down

		$BR_UTIL delif $PARA1 eth2 2> /dev/null
		$BR_UTIL delif $PARA1 eth3 2> /dev/null
		$BR_UTIL delif $PARA1 eth4 2> /dev/null

	fi
}

rtl_shut_down_mesh_if() {
#for mesh
	if [ -f /bin/pathsel ]; then
                eval `$GETMIB MESH_ENABLE`

		#if [ $MESH_ENABLE = 1  ]; then
	$IFCONFIG wlan0-msh0 down 2> /dev/null
			$BR_UTIL delif $PARA1 wlan0-msh0 2> /dev/null
		#fi
	fi
}

rtl_del_wlan0_eth1() {
	#delete wlan0 eth1 interface first always, wlan0 eth1 will be added later if mode is opmode = bridge and gw platform
	$BR_UTIL delif $PARA1 eth1 2> /dev/null  
	$BR_UTIL delif $PARA1 wlan0 2> /dev/null  	
	#iwpriv wlan0 clear_brsc
	if [ ! -f $INITFILE ]; then
	$BR_UTIL delbr $PARA1
	fi	
}

rtl_enable_lan_if() {
	# Enable LAN interface (Ethernet, wlan, WDS, bridge)
	echo 'Setup bridge...'
	if [ ! -f $INITFILE ]; then
	$BR_UTIL addbr $PARA1
	fi

	eval `$GETMIB STP_ENABLED`
	if [ "$STP_ENABLED" = '0' ]; then
		$BR_UTIL setfd $PARA1 0
		$BR_UTIL stp $PARA1 0
	else
		$BR_UTIL setfd $PARA1 4
		$BR_UTIL stp $PARA1 1
	fi


	# do not edit the next line
	###CONFIG_RTK_VLAN_SUPPORT###
	if [ "$CONFIG_RTK_VLAN_SUPPORT" = "y" ]; then

	## Set vlan setting to driver ##	
	##ETH_VLAN_SWITCH=/proc/disable_l2_table	
	##if [ -f $ETH_VLAN_SWITCH ]; then	
		eval `$GETMIB VLANCONFIG_ENABLED`

		if [ $VLANCONFIG_ENABLED != 0 ]; then
                        echo 1 > /proc/rtk_vlan_support
                else
                        echo 0 > /proc/rtk_vlan_support
                fi
		
		eval `$GETMIB VLANCONFIG_TBL_NUM`	
		num=1
  		while [ $num -le $VLANCONFIG_TBL_NUM ];
  		do
    		VLANCONFIG_TBL=`flash get VLANCONFIG_TBL | grep VLANCONFIG_TBL$num=`
    		tmp_str=`echo $VLANCONFIG_TBL | cut -f2 -d=`
    		name=`echo $tmp_str | cut -f1 -d,`    				
   			vlan=`echo $tmp_str | cut -f2 -d,`
   			tag=`echo $tmp_str | cut -f3 -d,`
   			priority=`echo $tmp_str | cut -f4 -d,`
   			cfi=`echo $tmp_str | cut -f5 -d,`
   			vid=`echo $tmp_str | cut -f6 -d,`
   			is_lan=`echo $tmp_str | cut -f7 -d,`   			
			DRV_VLAN_FILE=/proc/$name/mib_vlan
			if [ -f $DRV_VLAN_FILE ]; then
				if [ $VLANCONFIG_ENABLED != 0 ]; then
					echo "1 $is_lan $vlan $tag $vid $priority $cfi" > $DRV_VLAN_FILE	
				else					
					echo "0 $is_lan $vlan $tag $vid $priority $cfi" > $DRV_VLAN_FILE
				fi				
			fi					
			num=`expr $num + 1`
		done
		
		#if [ $VLANCONFIG_ENABLED != 0 ]; then
		#	echo 1 > /proc/rtk_vlan_support	
		#else
		#	echo 0 > /proc/rtk_vlan_support		
		#fi		
	##fi

	else
		VLANCONFIG_ENABLED=0
	fi


	#Add lan port to bridge interface
	for ARG in $PARA_ALL ; do
		INTERFACE=`echo $ARG | cut -b -3`
		if [ $INTERFACE = $LAN_PREFIX ]; then	
			up_interface=1
			##if [ -f $ETH_VLAN_SWITCH ]; then	
			if [ "$CONFIG_RTL_MULTI_LAN_DEV" != "y" ]; then	
				eth_num=`echo $ARG | cut -b 4-4`
				if [ $VLANCONFIG_ENABLED = 0 ] && [ $eth_num -ge 2 -a $eth_num -le 4 ]; then
					up_interface=0
				fi
			fi
			##fi		
			if [ $up_interface != 0 ]; then		
			$BR_UTIL addif $PARA1 $ARG 2> /dev/null
			$SET_IP $ARG  0.0.0.0
		fi	
		fi	
	done
	
	eval `$GETMIB SCHEDULE_ENABLED`
	START_WLAN=1
#	if [ "$SCHEDULE_ENABLED" != 0 ]; then
#		eval `$GETMIB SCHEDULE_TBL_NUM`
#		if [ "$SCHEDULE_TBL_NUM" != 0 ]; then
#				START_WLAN=0
#		fi
#	fi
#	
#	if [ -r /proc/rf_switch ]; then
#		RF_SW_STATE=`cat /proc/rf_switch`
#		if [ "$RF_SW_STATE" = '0' ]; then
#				START_WLAN=0
#		fi 
#	fi
	ROOT_IF=
	HAS_WLAN=0
	for ARG in $PARA_ALL ; do
		INTERFACE=`echo $ARG | cut -b -4`
		if [ $INTERFACE = $WLAN_PREFIX ]; then
			HAS_WLAN=1
			VAP=`echo $ARG | cut -b 7-8`
			if [ "$VAP" = "va" ]; then
				ROOT_IF=`echo $ARG | cut -b -5`
				eval `$GETMIB $ROOT_IF WLAN_DISABLED`
				if [ "$WLAN_DISABLED" = 0 ]; then
					eval `$GETMIB $ROOT_IF MODE`
					# 802.11s Mesh NOTICE: If Mesh work with Multiple AP, Add "$MODE" != 4 (AP+MPP) and "$MODE" != 6 (MAP)
					if  [ "$MODE" != 0 ] && [ "$MODE" != 3 ]; then
						WLAN_DISABLED=1
					else
						eval `$GETMIB $ARG WLAN_DISABLED`
					#	if [ "$WLAN_DISABLED" = 0 ]; then
					#		WLAN_DISABLED=1
					#	else
					#		WLAN_DISABLED=0
					#	fi						
					fi
				else
					WLAN_DISABLED=1
				fi
			else
				eval `$GETMIB $ARG WLAN_DISABLED`
			fi
			
			if [ "$WLAN_DISABLED" = 0 ]; then
				eval `$GETMIB OP_MODE`
				eval `$GETMIB WISP_WAN_ID`
				# if opmode is wireless isp, don't add wlan0 to bridge
				if [ "$VAP" = "va" ] || [ "$OP_MODE" != '2' ] || [ $ARG != "wlan$WISP_WAN_ID" ] ;then
					$BR_UTIL addif $PARA1 $ARG 2> /dev/null
					if [ $START_WLAN != 0 ]; then
					$SET_IP $ARG 0.0.0.0
					fi
				else
					if [ $START_WLAN != 0 ]; then
					$IFCONFIG $ARG up
					fi		
				fi		
				eval `$GETMIB $ARG WDS_ENABLED`
				eval `$GETMIB $ARG WDS_NUM`
				eval `$GETMIB $ARG MODE`
				if [ $WDS_ENABLED != 0 ] && [ $WDS_NUM != 0 ] && [ $MODE = 2 -o $MODE = 3 ]; then
					num=0
					while [ $num -lt $WDS_NUM ]
					do
						$BR_UTIL addif $PARA1 $ARG-wds$num 2> /dev/null
						$SET_IP $ARG-wds$num 0.0.0.0		
						num=`expr $num + 1`
					done
				fi				
			fi
		fi	
	done	
#for mesh
	if [ -f /bin/pathsel ]; then
		eval `$GETMIB MESH_ENABLE`

		if  [ -n "$MODE" ] && [ $MODE = 4 -o $MODE = 5 -o $MODE = 6 -o $MODE = 7 ] && [ $MESH_ENABLE = 1 ]; then
			$BR_UTIL addif $PARA1 wlan0-msh0
			$SET_IP wlan0-msh0 0.0.0.0
		fi
	fi
#	if [ $MODE = 6 -o $MODE = 7 ]; then
#		$BR_UTIL delif $PARA1 eth0
#		$SET_IP wlan0-msh0 192.168.1.10
#	fi	
				
#for mesh
	# Set Ethernet 0 MAC address to bridge
	eval `$GETMIB STP_ENABLED`
	if [ "$STP_ENABLED" = '0' ]; then
		eval `$GETMIB ELAN_MAC_ADDR`
		if [ "$ELAN_MAC_ADDR" = "000000000000" ]; then
			eval `$GETMIB HW_NIC0_ADDR`
			ELAN_MAC_ADDR=$HW_NIC0_ADDR
		fi
		ifconfig $PARA1 hw ether $ELAN_MAC_ADDR
	fi

	if [ ! -f $INITFILE ]; then
	$SET_IP $PARA1 0.0.0.0
		echo 1 > $INITFILE 	
	fi
}
#end of rtl_enable_lan_if



# Set fixed IP or start DHCP client
rtl_set_lan_ip() {
	eval `$GETMIB DHCP`
	if [ "$DHCP" = '0' -o "$DHCP" = '2' ]; then
		eval `$GETMIB IP_ADDR`
		eval `$GETMIB SUBNET_MASK`
		eval `$GETMIB DEFAULT_GATEWAY`
		$SET_IP $PARA1 $IP_ADDR $SUBNET_MASK $DEFAULT_GATEWAY
	elif [ "$DHCP" = '1' ]; then
	  {
		eval `$GETMIB STP_ENABLED`
		if [ "$STP_ENABLED" = '1' ]; then
			echo 'waiting for bridge initialization...'
			sleep 30
		fi
		$START_DHCP_CLIENT $PARA1 no
	  }&
	fi
}

rtl_bridge() {
	if [ "$PARA3" != "null" ]; then
		rtl_shutdown_lan_if		
		if [ "$CONFIG_RTL_MULTI_LAN_DEV" != "y" ]; then		
		rtl_shutdown_vlan_if
		fi
		rtl_shut_down_mesh_if
		rtl_del_wlan0_eth1
		rtl_enable_lan_if
	fi
	
	rtl_set_lan_ip
}


rtl_bridge

