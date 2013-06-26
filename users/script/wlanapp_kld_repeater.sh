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

WLAN_PREFIX=wlan
START=1
if [ $# -ge 1 ]; then
	if [ $1 = 'kill' ]; then
		START=0
	fi
	shift
fi

NAME=`echo $1 | cut -b -4`
WLAN_INTERFACE=
while [ $# -ge 1 -a "$NAME" = "$WLAN_PREFIX" ]
do		
	WLAN_INTERFACE="$WLAN_INTERFACE $1" 
	shift
	NAME=`echo $1 | cut -b -4`	
done

if [ $START = 1 -a $# -lt 1 ]; then 
	echo "Usage: $0 [start|kill] wlan_interface... bridge_interface [digest] [qfirst]"
	exit 1 
fi

_GETMIB="flash get"
EASYCONF_SCRIPT=autoconf.sh
EASYCONF_DAEMON=autoconf
EASYCONF_CONFIG=/var/autoconf.conf
#DEBUG_EASYCONF=-debug
DEBUG_EASYCONF=
VXD_INTERFACE=

if [ -z "$WLAN_INTERFACE" ]; then
	echo 'Error in wlanapp.sh, no wlan interface is given!'
	exit 0
fi

## kill 802.1x, autoconf and IAPP daemon ##
PIDFILE=/var/run/iwcontrol.pid
if [ -f $PIDFILE ] ; then
       	PID=`cat $PIDFILE`
	if [ $PID != 0 ]; then
		kill -9 $PID
	fi
       	rm -f $PIDFILE
fi

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
	
#	PIDFILE=/var/run/autoconf-$WLAN.pid
#	if [ -f $PIDFILE ] ; then
#		$EASYCONF_DAEMON -w $WLAN -kill 
#		rm -f $PIDFILE
#	fi		
	
# for WPS ---------------------------------->>
	PIDFILE=/var/run/wscd-$WLAN.pid
	if [ -f $PIDFILE ] ; then
       		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
	       		kill -9 $PID
		fi
       		rm -f $PIDFILE   
	fi 

	# for WPS Repeater---------------------------------->>
	PIDFILE=/var/run/wscd-$WLAN-vxd.pid
	if [ -f $PIDFILE ] ; then
       		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
	       		kill -9 $PID
		fi
       		rm -f $PIDFILE   
	fi
	#<<----------------------------------- for WPS Repeater
	
	echo 0 > /proc/gpio
done
#<<----------------------------------- for WPS

PIDFILE=/var/run/iapp.pid
if [ -f $PIDFILE ] ; then
       	PID=`cat $PIDFILE`
	if [ $PID != 0 ]; then
		kill -9 $PID
	fi
       	rm -f $PIDFILE
fi

rm -f /var/*.fifo

if [ $START = 0 ]; then
	exit 1
fi

## start 802.1x, autoconf and IAPP daemon ##
WLAN_ENABLED=0
IAPP_ENABLED=0
DEAMON_CREATED=0
IS_AP=0
IS_CLIENT=0

for WLAN in $WLAN_INTERFACE ; do
	GETMIB="$_GETMIB $WLAN"
	_ENABLE_1X=0
	eval `$GETMIB ENCRYPT`
	eval `$GETMIB WLAN_DISABLED`	
	eval `$GETMIB MODE`	
	eval `$GETMIB WDS_ENABLED`
	eval `$GETMIB WDS_NUM`

	if  [ "$WLAN_DISABLED" = 0 ] && [ "$MODE" = 2 -o "$MODE" = 3 ] && 
			[ "$WDS_ENABLED" != 0 -a "$WDS_NUM" != 0 ]; then
		eval `$GETMIB WDS_ENCRYPT`
		if [ $WDS_ENCRYPT = 3 -o $WDS_ENCRYPT = 4 ]; then
			flash wpa $WLAN /var/wpa-wds-$WLAN.conf	wds
			auth $WLAN $1 wds /var/wpa-wds-$WLAN.conf
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
		fi
	else
		_ENABLE_1X=1
	fi

	VXD_ROLE=
	if [ "$_ENABLE_1X" != 0 -a "$WLAN_DISABLED" = 0 ]; then	
		flash wpa $WLAN /var/wpa-$WLAN.conf
		if [ "$MODE" = '1' ]; then
			eval `$GETMIB NETWORK_TYPE`
			if [ "$NETWORK_TYPE" = '0' ]; then
				ROLE=client-infra
				VXD_ROLE=auth
				#cert.sh $WLAN
			else
				ROLE=client-adhoc			
			fi
		else
			ROLE=auth
			VXD_ROLE=client-infra
		fi
		
		if [ "$MODE" != '2' ]; then
## use wlan driver PSK module in AP mode instead of auth daemon
#			eval `$GETMIB WPA_AUTH`			
#			if  [ "$WPA_AUTH" != 2 ] || [ "$MODE" = 1 ]; then
				auth $WLAN $1 $ROLE /var/wpa-$WLAN.conf
				DEAMON_CREATED=1
#			fi
###############################################################
			eval `$GETMIB REPEATER_ENABLED1`
			eval `$GETMIB WPA_AUTH`
			if [ "$ENCRYPT" -ge 2 ] && [ "$WPA_AUTH" = 2 ]; then	
				if [ "$VXD_ROLE" != "" ] && [ $WLAN = "wlan0" ]; then
					if  [ "$REPEATER_ENABLED1" != 0 ]; then			
						flash wpa $WLAN-vxd /var/wpa-$WLAN-vxd.conf
						auth $WLAN-vxd $1 $VXD_ROLE /var/wpa-$WLAN-vxd.conf					
						VXD_INTERFACE=$WLAN-vxd
					fi			
				fi				
	
				eval `$GETMIB REPEATER_ENABLED2`		
				if [ "$VXD_ROLE" != "" ] && [ $WLAN = "wlan1" ]; then
					if  [ "$REPEATER_ENABLED2" != 0 ]; then			
						flash wpa $WLAN-vxd /var/wpa-$WLAN-vxd.conf
						auth $WLAN-vxd $1 $VXD_ROLE /var/wpa-$WLAN-vxd.conf	
						VXD_INTERFACE=$VXD_INTERFACE $WLAN-vxd
					fi			
				fi			
			fi
		fi
	fi
	

	## See if auto-config is enabled ##
#	eval `$GETMIB EASYCFG_ENABLED`		
EASYCFG_ENABLED=0
	if [ "$WLAN_DISABLED" = 0 -a "$EASYCFG_ENABLED" != 0 -a "$MODE" != 2 ]; then	
		## Generate autoconf config file ##		
		eval `$GETMIB EASYCFG_MODE`
		eval `$GETMIB EASYCFG_ALG_REQ`
		eval `$GETMIB EASYCFG_ALG_SUPP`
		eval `$GETMIB NETWORK_TYPE`

		DO_QUESTION_REQUEST=0
		if [ $MODE = 0 -o $MODE = 3 ]; then
			eval `$GETMIB EASYCFG_DIGEST`
			echo "algorithm = $EASYCFG_ALG_SUPP" > $EASYCONF_CONFIG		
		else
			echo "algorithm = $EASYCFG_ALG_REQ" > $EASYCONF_CONFIG	
			if [ $NETWORK_TYPE = 0 ]; then
				if [ "$EASYCFG_MODE" = "2" -o "$EASYCFG_MODE" = "3"  ] && [ $# -lt 2 ]; then
					EASYCFG_DIGEST="00000000000000000000000000000000"	
				else
					EASYCFG_DIGEST="$2"
					DO_QUESTION_REQUEST=1
				fi	
			else	
				if [ "$EASYCFG_MODE" = "2" -o "$EASYCFG_MODE" = "3" ] && [ $# -lt 2 ]; then
					eval `$GETMIB EASYCFG_DIGEST`
				else
					EASYCFG_DIGEST="$2"
					DO_QUESTION_REQUEST=1					
				fi
			fi	
		fi
		echo "digest = $EASYCFG_DIGEST" >> $EASYCONF_CONFIG

		eval `$GETMIB EASYCFG_KEY`
		echo "key = $EASYCFG_KEY" >> $EASYCONF_CONFIG

		eval `$GETMIB EASYCFG_SSID`
		echo "ssid = $EASYCFG_SSID" >> $EASYCONF_CONFIG
		echo "wlan_fifo = /var/$EASYCONF_DAEMON-$WLAN.fifo" >> $EASYCONF_CONFIG
		eval `$GETMIB EASYCFG_SCAN_SSID`
		echo "autoconf_ssid = $EASYCFG_SCAN_SSID" >> $EASYCONF_CONFIG
		
		## Start autoconf daemon ##
		if [ $MODE = 0 -o $MODE = 3 ]; then
			CMD="-server $DEBUG_EASYCONF"
			if [ $EASYCFG_MODE = 1 ]; then
				CMD="$CMD -button"
			elif [ $EASYCFG_MODE = 2 ]; then
				CMD="$CMD -question"
			elif [ $EASYCFG_MODE = 3 ]; then
				CMD="$CMD -button -question"
			else
				echo 'Invalid EASYCFG_MODE value !'
				exit 0
			fi
		else	
			if [ $NETWORK_TYPE = 0 ]; then
				CMD="-client $DEBUG_EASYCONF"
				if [ $EASYCFG_MODE = 3 ]; then		
					EASYCFG_MODE=1
				fi		
			else
				CMD="-adhoc $DEBUG_EASYCONF"
			fi

			if [ "$EASYCFG_MODE" = "1" -o "$EASYCFG_MODE" = "3" ]; then
				CMD="$CMD -button"
			fi
			if [ "$EASYCFG_MODE" = "2" -o "$EASYCFG_MODE" = "3" ]; then
				if [ $# -ge 3 ] && [ "$3" = "qfirst" ]; then
					CMD="$CMD -question_first"
				else
					CMD="$CMD -question"
				fi
				if [ "$DO_QUESTION_REQUEST" = 1 ]; then		
					CMD="$CMD -question_request"		
				fi		
			fi
		fi
		
#		CMD="$CMD -w $WLAN"
#		$EASYCONF_DAEMON $CMD
		DEAMON_CREATED=1

		if [ "$MODE" = '1' ]; then		
			IS_CLIENT=1		
		fi
	fi	
	
#	if [ "$EASYCFG_ENABLED" = 0 ]; then
#		CMD="-w $WLAN -disable"
#		$EASYCONF_DAEMON $CMD
#	fi
		
	if [ "$WLAN_DISABLED" = 0 ]; then
		WLAN_ENABLED=1
	fi	
	
	if [ "$MODE" = 0 -o "$MODE" = 3 ]; then
		IS_AP=1
	fi		
	
	eval `$GETMIB IAPP_DISABLED`
	if [ $IAPP_DISABLED = 0 ]; then
		IAPP_ENABLED=1
	fi	
done	

# start IAPP
if [ $IAPP_ENABLED = 1 ] && [ "$WLAN_ENABLED" = 1 ] && [ "$IS_AP" = 1 ] ; then
	iapp $1 $WLAN_INTERFACE &
	DEAMON_CREATED=1
fi

#if [ $IS_CLIENT = 1 ]; then
#	POLL=poll
#else
#	POLL=
#fi

# for WPS ------------------------------------------------->>
USE_IWCONTROL=1
DEBUG_ON=0

WSC=1
eval `$GETMIB WSC_DISABLE`
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
			eval `$GETMIB WPA_AUTH`						
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
		mkdir /var/wps
		cp /etc/simplecfg*.xml /var/wps
	fi

	eval `$GETMIB WSC_UPNP_ENABLED`

	# for WPS Repeater---------------------------------->>
	eval `flash get REPEATER_ENABLED1`
	VXD_WPS_ENABLED=0
	DAEMON_FIFO_DIR=
	#<<----------------------------------- for WPS Repeater
	
	if [ $MODE = 1 ]; then
		eval `$GETMIB WSC_CONFIGURED`
		if [ $REPEATER_ENABLED1 = 1 ] && [ $WSC_CONFIGURED = 1 ]; then
			# for WPS Repeater---------------------------------->>
			UPNP=$WSC_UPNP_ENABLED
			_CMD="-start -mode 5"
			VXD_WPS_ENABLED=1
			#<<----------------------------------- for WPS Repeater
		else
			UPNP=0
			#_CMD="-mode 2"
			_CMD=
		fi
	else				
		UPNP=$WSC_UPNP_ENABLED
		_CMD="-start"
	fi

	_CMD="$_CMD -upnp $UPNP"
	if [ $UPNP = 1 ]; then
		route del -net 239.255.255.250 netmask 255.255.255.255 "$1"
		route add -net 239.255.255.250 netmask 255.255.255.255 "$1"
	fi
	
	flash upd-wsc-conf /etc/wscd.conf /var/wsc.conf

	if [ $VXD_WPS_ENABLED = 1 ]; then
		# for WPS Repeater---------------------------------->>
		_CMD="$_CMD -c /var/wsc.conf -w wlan0-vxd"
		DAEMON_FIFO_DIR=/var/wscd-wlan0-vxd.fifo
		#<<----------------------------------- for WPS Repeater
	else
		_CMD="$_CMD -c /var/wsc.conf -w wlan0"
		DAEMON_FIFO_DIR=/var/wscd-wlan0.fifo
	fi
	
	if [ $DEBUG_ON = 1 ]; then
		_CMD="$_CMD -debug"	
	fi	
	if [ $USE_IWCONTROL = 1 ]; then
		_CMD="$_CMD -fi $DAEMON_FIFO_DIR"
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
	wscd $WSC_CMD &
	
	WAIT=1
	while [ $USE_IWCONTROL != 0 -a $WAIT != 0 ]		
	do	
		if [ -e $DAEMON_FIFO_DIR ]; then
			WAIT=0
		else
			sleep 1
		fi
	done
fi
#<<--------------------------------------------------- for WPS

if [ $DEAMON_CREATED = 1 ]; then
	iwcontrol $WLAN_INTERFACE $VXD_INTERFACE $POLL
fi
