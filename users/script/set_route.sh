#!/bin/sh
routed_conf=/var/run/routed.conf
rm /var/run/routed.conf 2>/dev/null
killall -15 routed 2>/dev/null
start_routed=1
wan_if=$1
eval `flash get NAT_ENABLED`
eval `flash get RIP_ENABLED`
eval `flash get RIP_WAN_TX`
eval `flash get RIP_WAN_RX`
eval `flash get RIP_LAN_TX`
eval `flash get RIP_LAN_RX`
eval `flash get WAN_DHCP`

#RX:0-none 1-RIP1 2-RIP2 3-both
#TX:0-none 1-RIP1 2-RIP2 3-RIP1 compiable, dst is 224.0.0.9, but payload is rip 1

if [ "$NAT_ENABLED" = '0' ]; then
#nat disabled
	if [ "$RIP_LAN_TX" != '0' ] && [ "$RIP_LAN_RX" = '0' ]; then
	 #echo "enable tx only"
	  if [ "$RIP_LAN_TX" = '2' ]; then
		echo "network br0 0 2" >> $routed_conf
		echo "network $wan_if 0 2" >> $routed_conf
	  else
		echo "network br0 0 1" >> $routed_conf
		echo "network $wan_if 0 1" >> $routed_conf
	  fi
	elif [ "$RIP_LAN_TX" != '0' ] && [ "$RIP_LAN_RX" != '0' ]; then
	#echo "enable tx and rx"
			if [ "$RIP_LAN_TX" = '2' ]; then
				RIP_TX_MODE=2
			else
				RIP_TX_MODE=1
			fi
			if [ "$RIP_LAN_RX" = '2' ]; then
				RIP_RX_MODE=2
			else
				RIP_RX_MODE=1
			fi	  
	  echo "network br0 $RIP_RX_MODE $RIP_TX_MODE" >> $routed_conf
	  echo "network $wan_if $RIP_RX_MODE $RIP_TX_MODE" >> $routed_conf
	else
	#echo "enable rx only"	
		if [ "$RIP_LAN_RX" != '0' ]; then
			if [ "$RIP_LAN_RX" = '2' ]; then
				echo "network br0 2 0" >> $routed_conf
				echo "network $wan_if 2 0" >> $routed_conf
			else
				echo "network br0 1 0" >> $routed_conf
				echo "network $wan_if 1 0" >> $routed_conf
			fi
		else
			start_routed=0
		fi
	fi
else
#nat enabled
	if [ "$RIP_LAN_RX" != '0' ]; then
	  	if [ "$RIP_LAN_RX" = '2' ]; then
	    		echo "network br0 2 0" >> $routed_conf
			echo "network $wan_if 2 0" >> $routed_conf
	  	else
	  		echo "network br0 1 0" >> $routed_conf
			echo "network $wan_if 1 0" >> $routed_conf
	  	fi
	else
		start_routed=0	  	
	fi
fi
	
eval `flash get STATICROUTE_ENABLED`
eval `flash get STATICROUTE_TBL_NUM`
if [ $STATICROUTE_TBL_NUM -gt 0 ] && [ $STATICROUTE_ENABLED -gt 0 ]; then
  num=1
  while [ $num -le $STATICROUTE_TBL_NUM ];
  do
    STATICROUTE_TBL=`flash get STATICROUTE_TBL | grep STATICROUTE_TBL$num=`
    tmp=`echo $STATICROUTE_TBL | cut -f2 -d=`
    dstIp=`echo $tmp | cut -f1 -d,`
    netmask=`echo $tmp | cut -f2 -d,`
    gateway=`echo $tmp | cut -f3 -d,`
    iface=`echo $tmp | cut -f4 -d,`
    if [ $1 = 'ppp0' ]; then
	    if [ $iface = '1' ]; then
			route add -net $dstIp netmask $netmask dev $1
	    else
			route add -net $dstIp netmask $netmask gw $gateway dev br0
	    fi
		
   else
		
	    if [ $iface = '0' ]; then
	     	route add -net $dstIp netmask $netmask gw $gateway dev br0
	     elif [ $iface = '1' ]; then
	    	route add -net $dstIp netmask $netmask gw $gateway dev $1
	    fi
    fi
    #route add -net $dstIp netmask $netmask gw $gateway
    num=`expr $num + 1`
  done
fi

if [ "$RIP_ENABLED" != '0' ] && [ $start_routed = 1 ]; then
echo "Start RIP daemon"
routed -s
if [ "$NAT_ENABLED" = '0' ]; then
	if [ -f /var/run/igmp_pid ]; then
		#echo "kill igmppeoxy daemon since NAT is disabled"
		killall -9 igmpproxy 2> /dev/null
		rm -f /var/run/igmp_pid 2> /dev/null
	fi
fi
fi
