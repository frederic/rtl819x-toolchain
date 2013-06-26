#!/bin/sh
#
# script file for wan link
# Usage: wanlink.sh {connect | disconnect}
#

TOOL=flash
GETMIB="$TOOL get"

if [ $1 = '' ]; then
	echo "Usage: wanlink.sh {connect | disconnect}"
	exit
elif [ $1 != 'connect' ] && [ $1 != 'disconnect' ]; then
	echo "Usage: wanlink.sh {connect | disconnect}"
	exit
fi

eval `$GETMIB OP_MODE`
eval `$GETMIB WAN_DHCP`
WAN_INTERFACE=eth1
if [ $OP_MODE = '0' ]; then
if [ $1 = 'connect' ]; then
	echo WAN Port Connect
	if [ "$WAN_DHCP" = '1' ]; then
		echo WAN DHCP mode
	elif [ "$WAN_DHCP" = '3' ]; then
		echo WAN PPPOE mode
	elif [ "$WAN_DHCP" = '4' ]; then
		echo WAN PPTP mode
	fi
else
	echo WAN Port Disconnect
	if [ "$WAN_DHCP" = '1' ]; then
		echo WAN DHCP mode
	elif [ "$WAN_DHCP" = '3' ]; then
		echo WAN PPPOE mode
	elif [ "$WAN_DHCP" = '4' ]; then
		echo WAN PPTP mode
	fi
fi
fi
