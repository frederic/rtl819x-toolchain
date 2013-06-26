#!/bin/sh
#
# script file to start wlan applications (IAPP, Auth, Autoconf) daemon
#
# Usage: wlanapp.sh [start|kill] wlan_interface...br_interface
#

if [ $# -lt 2 ] || [ $1 != 'start' -a $1 != 'kill' ] ; then 
	echo "Usage: $0 [start|kill] wlan_interface...br_interface [digest] [qfirst]"
	exit 1 
fi

ASESERVER=aseUdpServer
AECLIENT=aeUdpClient
WLAN_PREFIX=wlan
KILLALL=killall
START=1
PARAM_NUM=$#
PARAM_ALL=$*
PARAM1=$1
PARAM_BR=
WLAN_INTERFACE=
WLAN_INTERFACE_LIST=wlan0,wlan0-va0,wlan0-va1,wlan0-va2,wlan0-va3


rtl_check_wlan_if() {
	if [ $PARAM_NUM -ge 1 ]; then
		for ARG in $PARAM_ALL ; do
			NAME=`echo $ARG | cut -b -4`
			if [ "$NAME" = "$WLAN_PREFIX" ]; then
				WLAN_INTERFACE="$WLAN_INTERFACE $ARG"
			elif [ "$NAME" != "digest" -a "$NAME" != "qfirst" ]; then
				PARAM_BR=$ARG
			fi
			
		done	

	fi
}
	
_GETMIB="flash get"
EASYCONF_SCRIPT=autoconf.sh
EASYCONF_DAEMON=autoconf
EASYCONF_CONFIG=/var/autoconf.conf
#DEBUG_EASYCONF=-debug
DEBUG_EASYCONF=
VXD_INTERFACE=
BR_UTIL=brctl


## kill 802.1x, autoconf and IAPP daemon ##
rtl_kill_iwcontrol_pid() { 
	PIDFILE="/var/run/iwcontrol.pid"
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
			kill -9 $PID
		fi
		rm -f $PIDFILE
	fi
}

# =========for MESH 802.11s====================
rtl_kill_pathsel_pid() {
	PIDFILE=/var/run/pathsel.pid
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
			kill -9 $PID
		fi
		rm -f $PIDFILE
		$BR_UTIL meshsignaloff
	fi
}

#PIDFILE=/var/run/pathse1.pid
#if [ -f $PIDFILE ] ; then
#       	PID=`cat $PIDFILE`
#	if [ $PID != 0 ]; then
#		kill -9 $PID
#	fi
#       	rm -f $PIDFILE
#fi

#PIDFILE=/var/run/pat1sel.pid
#if [ -f $PIDFILE ] ; then
#       	PID=`cat $PIDFILE`
#	if [ $PID != 0 ]; then
#		kill -9 $PID
#	fi
#       	rm -f $PIDFILE
#fi
# =========end of for MESH 802.11s====================
rtl_kill_wlan_pid() {
	for WLAN in $WLAN_INTERFACE ; do
		PIDFILE=/var/run/auth-$WLAN.pid
		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			if [ $PID != 0 ]; then
				kill -9 $PID
			fi
			rm -f $PIDFILE
			
			PIDFILE=/var/run/auth-$WLAN-vxd.pid 
			if [ -f $PIDFILE ] ; then		
				PID=`cat $PIDFILE`
				if [ "$PID" != 0 ]; then
				kill -9 $PID
				fi
				rm -f $PIDFILE       		
			fi
		fi
		
	# for WPS ---------------------------------->>
		PIDFILE=/var/run/wscd-$WLAN.pid
		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			if [ $PID != 0 ]; then
				kill -9 $PID
			fi
			rm -f $PIDFILE   
		fi 
		#echo 0 > /proc/gpio 2>/dev/null
	done
	#<<----------------------------------- for WPS
}

rtl_kill_iapp_pid() {
	PIDFILE=/var/run/iapp.pid
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
			kill -9 $PID
		fi
		rm -f $PIDFILE
	fi
}

## start 802.1x, autoconf and IAPP daemon ##
WLAN_ENABLED=0
IAPP_ENABLED=0
DEAMON_CREATED=0
IS_AP=0
IS_CLIENT=0
VALID_WLAN_INTERFACE=


rtl_start_wlan() {
	for WLAN in $WLAN_INTERFACE ; do
		GETMIB="$_GETMIB $WLAN"
		_ENABLE_1X=0
		_USE_RS=0
		eval `$GETMIB ENCRYPT`
		eval `$GETMIB WLAN_DISABLED`	
		eval `$GETMIB MODE`	
		eval `$GETMIB WDS_ENABLED`
		eval `$GETMIB WDS_NUM`
		eval `$GETMIB WPA_AUTH`	

		
		#for mesh	;when mesh no define will occur out of string space ; need check
		if [ "$WLAN_DISABLED" = 0 ] && 
			[ "$MODE" = 4 -o "$MODE" = 5 -o "$MODE" = 6 -o "$MODE" = 7 ]; then
		eval `$GETMIB MESH_ENCRYPT`
		fi	

		
		VAP=`echo $WLAN | cut -b 7-8`
		VAP_AUTH_ENABLE=0
		ROOT_AUTH_ENABLE=0


		if  [ "$WLAN_DISABLED" = 0 ] && [ "$MODE" = 2 -o "$MODE" = 3 ] && 
				[ "$WDS_ENABLED" != 0 -a "$WDS_NUM" != 0 ]; then
			eval `$GETMIB WDS_ENCRYPT`
			if [ $WDS_ENCRYPT = 3 -o $WDS_ENCRYPT = 4 ]; then
				flash wpa $WLAN /var/wpa-wds-$WLAN.conf	wds
				auth $WLAN $PARAM_BR wds /var/wpa-wds-$WLAN.conf
				PIDFILE=/var/run/auth-$WLAN.pid	
				while [ -f $PIDFILE ] 
				do
					sleep 1
				done				
			fi	
		fi
		
		if [ "$ENCRYPT" -lt 2 ]; then
			eval `$GETMIB ENABLE_1X`
			eval `$GETMIB MAC_AUTH_ENABLED`
			if [ "$ENABLE_1X" != 0 ] || [ "$MAC_AUTH_ENABLED" != 0 ]; then
				_ENABLE_1X=1
				_USE_RS=1
			fi
		else
			_ENABLE_1X=1
			if  [ "$WPA_AUTH" = 1 ]; then
				_USE_RS=1
			fi		
		fi

		# =========for MESH 802.11s====================
		if   [ "$MODE" = 4 ] || [ "$MODE" = 5 ] || [ "$MODE" = 6 ] || [ "$MODE" = 7 ] ; then
			if [ "$MESH_ENCRYPT" != 0 ]; then
				flash wpa $WLAN /var/wpa-$WLAN-msh0.conf msh
	# KEY_MAP_KEY_PATCH_0223
				auth $WLAN-msh0 $PARAM_BR wds /var/wpa-$WLAN-msh0.conf	
	# KEY_MAP_KEY_PATCH_0223
				PIDFILE=/var/run/auth-$WLAN-msh0.pid	
				while [ -f $PIDFILE ] 
				do
					sleep 1
				done
			fi
		fi
		# =========for MESH 802.11s====================

		ROLE=
		if [ "$_ENABLE_1X" != 0 -a "$WLAN_DISABLED" = 0 ]; then	
			flash wpa $WLAN /var/wpa-$WLAN.conf
			if [ "$MODE" = '1' ]; then
				eval `$GETMIB NETWORK_TYPE`
				if [ "$NETWORK_TYPE" = '0' ]; then
					ROLE=client-infra
					#cert.sh $WLAN
				else
					ROLE=client-adhoc			
				fi
			else
				ROLE=auth
			fi

			VAP_NOT_IN_PURE_AP_MODE=0		
			if [ "$VAP" = "va" ]; then	
				MODE_BACKUP=$MODE
				eval `flash get wlan0 MODE`	
				if [ "$MODE" != 0 -a "$MODE" != 3 ]; then				
					VAP_NOT_IN_PURE_AP_MODE=1
				fi
				MODE=$MODE_BACKUP
			fi		
			
			if [ "$MODE" != '2' ] && [ "$VAP_NOT_IN_PURE_AP_MODE" = '0' ]; then
	## use wlan driver PSK module in AP mode instead of auth daemon								
	# button 2009.05.21 
	#if  [ "$WPA_AUTH" != 2 ] || [ "$MODE" = 1 ] || [ "$_USE_RS" != 0 ]; then
				if  [ "$WPA_AUTH" != 2 ] || [ "$_USE_RS" != 0 ]; then
					auth $WLAN $PARAM_BR $ROLE /var/wpa-$WLAN.conf
					DEAMON_CREATED=1
					if [ "$VAP" = "va" ]; then				
						VAP_AUTH_ENABLE=1			
					else
						ROOT_AUTH_ENABLE=1
					fi
				fi
	###############################################################
		
			fi
		fi
		
		if [ "$VAP" = "vx" ] && [ "$WLAN_DISABLED" = 0 ]; then	
			if [ "$ROLE" != "auth" ] || [ "$ROLE" = "auth" -a "$_USE_RS" != 0 ]; then
				VXD_INTERFACE=$WLAN
			fi
		fi
		
		if [ "$VAP" = "va" ]; then
			if  [ "$WLAN_DISABLED" = 0 ]; then
				ROOT_IF=`echo $WLAN | cut -b -5`
				eval `flash get $ROOT_IF IAPP_DISABLED`
				if [ $IAPP_DISABLED = 0 ] || [ $VAP_AUTH_ENABLE = 1 ]; then
					VALID_WLAN_INTERFACE="$VALID_WLAN_INTERFACE $WLAN"
				fi
			fi
		else
			if [ "$VAP" != "vx" ]; then
				eval `$GETMIB IAPP_DISABLED`
				eval `$GETMIB WSC_DISABLE`
				if [ $ROOT_AUTH_ENABLE = 1 ] || [ $IAPP_DISABLED = 0 ] || [ $WSC_DISABLE = 0 ]; then
					VALID_WLAN_INTERFACE="$VALID_WLAN_INTERFACE $WLAN"
				fi
			fi
		fi

		if [ "$VAP" != "va" ] &&  [ "$VAP" != "vx" ] ; then
			if [ "$WLAN_DISABLED" = 0 ]; then
				WLAN_ENABLED=1
			fi	
			
			#if [ "$MODE" = 0 -o "$MODE" = 3 ]; then
	#modify for mesh
			# (MODE 0 -> AP, MODE 3 -> AP + WDS, MODE 4 -> AP + MPP)
			if [ "$MODE" = 0 ] || [ "$MODE" = 3 ] || [ "$MODE" = 4 ] || [ "$MODE" = 6 ]; then
				IS_AP=1
			fi		
		
			eval `$GETMIB IAPP_DISABLED`
			if [ $IAPP_DISABLED = 0 ]; then
				IAPP_ENABLED=1
			fi	
		fi
	done
	
	# start IAPP
	if [ $IAPP_ENABLED = 1 ] && [ "$WLAN_ENABLED" = 1 ] && [ "$IS_AP" = 1 ] ; then
		iapp $PARAM_BR $VALID_WLAN_INTERFACE &
		DEAMON_CREATED=1
	fi

# =========for MESH 802.11s====================
	if   [ "$MODE" = 4 ] || [ "$MODE" = 5 ] || [ "$MODE" = 6 ] || [ "$MODE" = 7 ] ; then
		eval `$GETMIB MESH_ENABLE`
		if [ "$MESH_ENABLE" = '1' ]; then
			pathsel $PARAM_BR $WLAN_INTERFACE &
		fi
	fi
# when reboot or do init.sh gw all, the paraments passed to this shell is the active interface except after upload root.bin
# when mesh is enabled, the wlan0-va0~wlan0-va3 and wlan0-vxd is disabled, so the MODE and MESH_ENABLE are right.
}

#end of start wlan

#if  [ -f /bin/pathse1 ]; then
#        pathse1 br0 eth0 &
#fi
#if  [ -f /bin/pat1sel  ]; then
#        pat1sel br0 msh0 &
#fi

#if [ -f /bin/netserver ];then
#        netserver
#fi

# =========end of for MESH 802.11s====================



#if [ $IS_CLIENT = 1 ]; then
#	POLL=poll
#else
#	POLL=
#fi

# for WPS ------------------------------------------------->>
rtl_start_wps() {
	if [ ! -e /bin/wscd ]; then
		return;
	fi
	for WLAN in $VALID_WLAN_INTERFACE ; do
		if [ $WLAN = "wlan0" ]; then
			GETMIB="$_GETMIB $WLAN"
			USE_IWCONTROL=1
			DEBUG_ON=0
			_ENABLE_1X=0

			WSC=1
			eval `$GETMIB WSC_DISABLE`
			eval `$GETMIB WLAN_DISABLED`
			eval `$GETMIB MODE`
			eval `$GETMIB ENCRYPT`

			if [ "$ENCRYPT" -lt 2 ]; then
				eval `$GETMIB ENABLE_1X`
				eval `$GETMIB MAC_AUTH_ENABLED`
				if [ "$ENABLE_1X" != 0 ] || [ "$MAC_AUTH_ENABLED" != 0 ]; then
					_ENABLE_1X=1
				fi
			else
				_ENABLE_1X=1
			fi

			if [ $WSC_DISABLE != 0 ]; then
				WSC=0
			else
				if  [ "$WLAN_DISABLED" != 0 ] || [ "$MODE" = 2 ]; then
					WSC=0
				else  
					if [ $MODE = 1 ]; then	
						eval `$GETMIB NETWORK_TYPE`
						if [ "$NETWORK_TYPE" != 0 ]; then
							WSC=0
						fi
					fi
					if [ $MODE = 0 ]; then	
						if [ $ENCRYPT -lt 2 ] && [ $_ENABLE_1X != 0 ]; then
							WSC=0
						fi			
						if [ $ENCRYPT -ge 2 ] && [ $WPA_AUTH = 1 ]; then
							WSC=0
						fi			
					fi
				fi
			fi

			if [ $WSC = 1 ]; then
				if [ ! -f /var/wps/simplecfgservice.xml ]; then
					if [ -e /var/wps ]; then
						rm /var/wps -rf
					fi
					mkdir /var/wps
					cp /etc/simplecfg*.xml /var/wps
				fi

				if [ $MODE = 1 ]; then			
					UPNP=0
					_CMD="-mode 2"
				else		
					eval `$GETMIB WSC_UPNP_ENABLED`		
					UPNP=$WSC_UPNP_ENABLED
					_CMD="-start"
				fi

				if [ $UPNP = 1 ]; then
					route del -net 239.255.255.250 netmask 255.255.255.255 "$PARAM_BR"
					route add -net 239.255.255.250 netmask 255.255.255.255 "$PARAM_BR"
				fi
		
				flash upd-wsc-conf /etc/wscd.conf /var/wsc.conf
				_CMD="$_CMD -c /var/wsc.conf -w wlan0"
		
				if [ $DEBUG_ON = 1 ]; then
					_CMD="$_CMD -debug"	
				fi	
				if [ $USE_IWCONTROL = 1 ]; then
					_CMD="$_CMD -fi /var/wscd-wlan0.fifo"
					DEAMON_CREATED=1
				fi
		
				if [ -e /var/wps_start_pbc ]; then		
					_CMD="$_CMD -start_pbc"
					rm -f /var/wps_pbc
				fi
				if [ -e /var/wps_start_pin ]; then		
					_CMD="$_CMD -start"
					rm -f /var/wps_start_pin
				fi	
				if [ -e /var/wps_local_pin ]; then		
					PIN=`cat /var/wps_local_pin`		
					_CMD="$_CMD -local_pin $PIN"
					rm -f /var/wps_local_pin
				fi
				if [ -e /var/wps_peer_pin ]; then		
					PIN=`cat /var/wps_peer_pin`		
					_CMD="$_CMD -peer_pin $PIN"
					rm -f /var/wps_peer_pin
				fi				
				WSC_CMD=$_CMD	
				wscd $WSC_CMD -daemon
		
				WAIT=1
				while [ $USE_IWCONTROL != 0 -a $WAIT != 0 ]		
				do	
					if [ -e /var/wscd-wlan0.fifo ]; then
						WAIT=0
					else
						sleep 1
					fi
				done
			fi
		fi
	done
}
#<<--------------------------------------------------- for WPS
rtl_start_iwcontrol() {
	if [ $DEAMON_CREATED = 1 ]; then
		iwcontrol $VALID_WLAN_INTERFACE $VXD_INTERFACE $POLL
	fi
}

#For WAPI.now not support  VAP
rtl_start_wapi() {

###CONFIG_RTL_WAPI_SUPPORT###
###CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT###

	# first, to kill daemon related wapi-cert
	# in order to avoid multiple daemon existing
	if [ "$CONFIG_RTL_WAPI_SUPPORT" = "y" ]; then
		if [ "$CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT" = "y" ]; then
			$KILLALL $ASESERVER 2> /dev/null
		fi
		$KILLALL $AECLIENT  2> /dev/null

		GETMIB="$_GETMIB wlan0"
		eval `$GETMIB ENCRYPT`
		if [ $ENCRYPT = 7 ]; then
			eval `$GETMIB WAPI_AUTH`
			if [ $WAPI_AUTH = 1 ]; then
				eval `$GETMIB WAPI_ASIPADDR`
				if [ "$CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT" = "y" ]; then
					eval `$GETMIB IP_ADDR`
					#is local
					if [ $WAPI_ASIPADDR = $IP_ADDR ]; then
						$ASESERVER &
					fi
				fi
#				$AECLIENT $WAPI_ASIPADDR &
				$AECLIENT -d $WAPI_ASIPADDR -i $WLAN_INTERFACE_LIST &
			fi
		fi
	fi
}




rtl_wlanapp() {
	if [ $PARAM1 = 'kill' ]; then
		START=0
	fi

	rtl_check_wlan_if

	if [ -z "$WLAN_INTERFACE" ]; then
		echo 'Error in wlanapp.sh, no wlan interface is given!'
		exit 0
	fi
	
	rtl_kill_iwcontrol_pid
	rtl_kill_pathsel_pid
	rtl_kill_wlan_pid

	rtl_kill_iapp_pid

	rm -f /var/*.fifo

	if [ $START = 0 ]; then
		exit 1
	fi

	rtl_start_wlan
	rtl_start_wps
	rtl_start_iwcontrol
	rtl_start_wapi
}

rtl_wlanapp
