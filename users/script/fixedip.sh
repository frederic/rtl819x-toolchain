#!/bin/sh
#
# Script file to set fixed IP, subnet and gateway
#
# Usage: fixedip.sh interface ip netmask gateway
#

PARAM1=$1
PARAM2=$2
PARAM3=$3
PARAM4=$4

rtl_set_ip() {
	if [ -n "$PARAM3" ]; then
		ifconfig $PARAM1 $PARAM2 netmask $PARAM3
	else
		ifconfig $PARAM1 $PARAM2
	fi
}

rtl_del_route() {
	while route del  default dev $PARAM1
	do :
	done
}


rtl_add_route() {
	if [ -n "$PARAM4" ]; then
		if [ "$PARAM4" != "0.0.0.0" ]; then
			route add -net default gw $PARAM4 dev $PARAM1
		fi
	fi
}

rtl_fixedip() {
	rtl_set_ip
	rtl_del_route
	rtl_add_route
}

rtl_fixedip
