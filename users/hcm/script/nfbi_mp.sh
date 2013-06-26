#!/bin/sh
#
# script file to setup  network
#
#


#GETMIB="flash gethw"
#ELAN_MAC_ADDR="000000000000"
#eval `$GETMIB HW_NIC0_ADDR`
#if [ "$HW_NIC0_ADDR" = "000000000000" ]; then
      #  eval `$GETMIB HW_NIC0_ADDR`
        ELAN_MAC_ADDR="56aaa55a7de8"
#else
#	ELAN_MAC_ADDR=$HW_NIC0_ADDR
#fi
echo "$ELAN_MAC_ADDR"
ifconfig lo   127.0.0.1
ifconfig eth0 hw ether $ELAN_MAC_ADDR
#ifconfig eth0 192.168.1.6
ifconfig eth0 $1

iwpriv wlan0 set_mib mp_specific=1
ifconfig  wlan0 up
/bin/UDPserver &
