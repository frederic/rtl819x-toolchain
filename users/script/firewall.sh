#!/bin/sh
START_SCHEDULE=schedule.sh
START_IP_QOS=ip_qos.sh

ip=0
port1=0
port2=0
protocol=0
ZERO=0
WAN=eth1
WAN_Phy=eth1
BRIDGE=br0
eval `flash get WAN_DHCP`
eval `flash get IPFILTER_TBL_NUM`
eval `flash get PORTFILTER_TBL_NUM`
eval `flash get MACFILTER_TBL_NUM`
eval `flash get PORTFW_TBL_NUM`
eval `flash get DMZ_HOST`

eval `flash get IPFILTER_ENABLED`
eval `flash get PORTFILTER_ENABLED`
eval `flash get MACFILTER_ENABLED`
eval `flash get PORTFW_ENABLED`
eval `flash get DMZ_ENABLED`
eval `flash get OP_MODE`
eval `flash get WEB_WAN_ACCESS_ENABLED`
eval `flash get PING_WAN_ACCESS_ENABLED`

#flush fast natp table
#echo "1" > /proc/net/flush_conntrack
echo "2">/proc/fast_nat

# if wireless ISP mode , set WAN to wlan0
eval `flash get  WISP_WAN_ID`
if [ "$OP_MODE" = '2' ];then
	WAN=wlan$WISP_WAN_ID
	WAN_Phy=wlan$WISP_WAN_ID
fi

if [ $WAN_DHCP = 3 ] || [ $WAN_DHCP = 4 ] || [ $WAN_DHCP = 6 ] ; then
  WAN=ppp0
fi 



EXT_IP0=`ifconfig $WAN | grep -i "addr:"`
EXT_IP1=`echo $EXT_IP0 | cut -f2 -d:`
EXT_IP=`echo $EXT_IP1 | cut -f1 -d " "`
if [ "$EXT_IP" = '' ] ; then
        echo "Warning [firewall]: there is no WAN IP!"
        exit 1
fi
INT_IP0=`ifconfig $BRIDGE | grep -i "addr:"`
INT_IP1=`echo $INT_IP0 | cut -f2 -d:`
INT_IP=`echo $INT_IP1 | cut -f1 -d " "`

#echo "Clean conntrack table first before Re-Set Iptables Rules"
echo 2 > /proc/fast_nat
#echo "Finish clean conntrack table......................................."
iptables -F
iptables -t nat -F PREROUTING
iptables -t nat -F POSTROUTING
iptables -t mangle -F
iptables -F INPUT
iptables -F OUTPUT
iptables -F FORWARD
iptables -P OUTPUT ACCEPT

if [ $OP_MODE != 1 ] ; then
	iptables -P INPUT DROP
else
	iptables -P INPUT ACCEPT #default accept
fi

if [ $OP_MODE != 3 ] ; then
	iptables -P FORWARD DROP
else
	iptables -P FORWARD ACCEPT #default accept
fi


echo 1 > /proc/sys/net/ipv4/ip_forward

#if opmode is bridge , exit don't set firewall
#eval `flash get RIP_ENABLED`
#if [ "$OP_MODE" = '1' ] || [ "$RIP_ENABLED" = '1' ] ; then
if [ "$OP_MODE" = '1' ];then
	exit 
fi


#iptables -t nat -A POSTROUTING -o $WAN -j MASQUERADE

#eval `flash get VPN_PASSTHRU_PPTP_ENABLED`
#if [ $VPN_PASSTHRU_PPTP_ENABLED = 0 ];then
#       iptables -A FORWARD -p TCP --dport 1723 -j DROP
#fi
#eval `flash get VPN_PASSTHRU_L2TP_ENABLED`
#if [ $VPN_PASSTHRU_L2TP_ENABLED = 0 ];then
#       iptables -A FORWARD -p UDP --dport 1701 -j DROP
#fi
#eval `flash get VPN_PASSTHRU_IPSEC_ENABLED`
#if [ $VPN_PASSTHRU_IPSEC_ENABLED = 0 ];then
#       iptables -A FORWARD -p UDP --dport 500 -j DROP
#fi

#redefine url and schedule filter
$START_SCHEDULE

## to get br0's ip/mask in kernel
# do not edit next line, it will be replaced to "CONFIG_RTL_HW_NAPT=y" or "CONFIG_RTL_HW_NAPT=" by mkimg
###CONFIG_RTL_HW_NAPT###
##echo "9" > /proc/sw_nat
if [ "$CONFIG_RTL_HW_NAPT" = 'y' ]; then
	echo "9" > /proc/hw_nat
	my_wan_type=`expr $WAN_DHCP + 80`
	echo "$my_wan_type" > /proc/hw_nat
else
	echo "9" > /proc/sw_nat
fi

if [ "$EXT_IP" != '' ] ; then
        if [ $WEB_WAN_ACCESS_ENABLED = 0 ]; then
          iptables -A INPUT -p tcp --dport 80:80 -i $WAN -d $EXT_IP -j DROP
        else
          iptables -A INPUT -p tcp --dport 80:80 -i $WAN -d $EXT_IP -j ACCEPT
        fi
fi

if [ $IPFILTER_TBL_NUM -gt 0 ] && [ $IPFILTER_ENABLED -gt 0 ];
then
  num=1
  while [ $num -le $IPFILTER_TBL_NUM ];
  do
    IPFILTER_TBL=`flash get IPFILTER_TBL | grep IPFILTER_TBL$num`
    port_ip=`echo $IPFILTER_TBL | cut -f2 -d=`
    ip=`echo $port_ip | cut -f1 -d,`
    protocol=`echo $port_ip | cut -f2 -d,`
    if [ $protocol = 1 ]; then
      iptables -A FORWARD -p TCP -s $ip -j DROP
    fi
    if [ $protocol = 2 ]; then
      iptables -A FORWARD -p UDP -s $ip -j DROP
      iptables -A INPUT -p UDP --dport 53:53 -s $ip -j DROP
    fi
    if [ $protocol = 3 ]; then
      iptables -A FORWARD -p TCP -s $ip -j DROP
      iptables -A FORWARD -p UDP -s $ip -j DROP
      iptables -A INPUT -p UDP --dport 53:53 -s $ip -j DROP
    fi
    num=`expr $num + 1`
  done
fi

if [ $MACFILTER_TBL_NUM -gt 0 ] && [ $MACFILTER_ENABLED -gt 0 ];
then
  num=1
  while [ $num -le $MACFILTER_TBL_NUM ];
  do
    MACFILTER_TBL=`flash get MACFILTER_TBL | grep MACFILTER_TBL$num`
    tmp_addr=`echo $MACFILTER_TBL | cut -f2 -d=`
    addr=`echo $tmp_addr | cut -f1 -d,`
    iptables -A FORWARD -m mac --mac-source $addr -j DROP
    num=`expr $num + 1`
  done
fi

# default enable nat speedup
if [ -f /proc/fast_nat ];then
	echo "1" > /proc/fast_nat
	#echo "1" > /proc/br_nat_speedup
fi

if [ $PORTFILTER_TBL_NUM -gt 0 ] && [ $PORTFILTER_ENABLED -gt 0 ];
then
  num=1
  #if enable port filter , then disable nat speedup for work around
  if [ -f /proc/fast_nat ];then
	echo "0" > /proc/fast_nat
	#echo "0" > /proc/br_nat_speedup
  fi

  while [ $num -le $PORTFILTER_TBL_NUM ];
  do
    PORTFILTER_TBL=`flash get PORTFILTER_TBL | grep PORTFILTER_TBL$num`
    port_ip=`echo $PORTFILTER_TBL | cut -f2 -d=`
    port1=`echo $port_ip | cut -f1 -d,`
    tmp_port=`echo $port_ip | cut -f2 -d,`
    port2=`echo $tmp_port | cut -f2 -d ' '`
    protocol=`echo $port_ip | cut -f3 -d,`
    num=`expr $num + 1`
    
    if [ $protocol = 1 ]; then
       iptables -A FORWARD -p TCP --dport $port1:$port2 -j DROP
    fi
    if [ $protocol = 2 ]; then
    	if [ $port1 -le 53 ] && [ $port2 -ge 53 ]; then
       		iptables -A INPUT -p UDP --dport 53:53 -j DROP
	fi
       	iptables -A FORWARD -p UDP --dport $port1:$port2 -j DROP
    fi
    if [ $protocol = 3 ]; then
       iptables -A FORWARD -p TCP --dport $port1:$port2 -j DROP
       iptables -A FORWARD -p UDP --dport $port1:$port2 -j DROP
	if [ $port1 -le 53 ] && [ $port2 -ge 53 ]; then
        	iptables -A INPUT -p UDP --dport 53:53 -j DROP
	fi
    fi
  done
fi

PPTP_VPN=0
L2TP_VPN=0
IPSEC_VPN=0
if [ $PORTFW_TBL_NUM -gt 0 ] && [ $PORTFW_ENABLED -gt 0 ];
then
  num=1
  while [ $num -le $PORTFW_TBL_NUM ];
  do
    PORTFW_TBL=`flash get PORTFW_TBL | grep PORTFW_TBL$num`
    port_ip=`echo $PORTFW_TBL | cut -f2 -d=`
    ip=`echo $port_ip | cut -f1 -d,`
    port1=`echo $port_ip | cut -f2 -d,`
    tmp_port=`echo $port_ip | cut -f3 -d,`
    port2=`echo $tmp_port | cut -f2 -d ' '`
    protocol=`echo $port_ip | cut -f4 -d,`
    num=`expr $num + 1`

    if [ $port1 -le 80 ] && [ $port2 -ge 80 ];then
	iptables -D INPUT -p tcp --dport 80:80 -i $WAN -d $EXT_IP -j DROP
#        iptables -D PREROUTING -t nat -i $WAN -p TCP --dport 80:80 -j DROP
    fi

    if [ $protocol = 1 ]; then
       iptables -A PREROUTING -t nat -p TCP --dport $port1:$port2 -d $EXT_IP -j DNAT --to $ip 
       iptables  -A FORWARD  -i $WAN -d $ip -p TCP --dport $port1:$port2 -j ACCEPT

    fi
    if [ $protocol = 2 ]; then
       iptables -A PREROUTING -t nat -p UDP --dport $port1:$port2 -d $EXT_IP -j DNAT --to $ip 
       iptables  -A FORWARD  -i $WAN -d $ip -p UDP --dport $port1:$port2 -j ACCEPT
    fi
    if [ $protocol = 3 ]; then
       iptables -A PREROUTING -t nat -p TCP --dport $port1:$port2 -d $EXT_IP -j DNAT --to $ip 
       iptables -A PREROUTING -t nat -p UDP --dport $port1:$port2 -d $EXT_IP -j DNAT --to $ip 
       iptables  -A FORWARD  -i $WAN -d $ip -p TCP --dport $port1:$port2 -j ACCEPT
       iptables  -A FORWARD  -i $WAN -d $ip -p UDP --dport $port1:$port2 -j ACCEPT
    fi
    
    if [ $PPTP_VPN = 0 ]; then
    	if [ $port1 -le 1723 ] && [ $port2 -ge 1723 ];then
	    	 if [ $protocol = 1 ] || [ $protocol = 3 ]; then	    	 	
		     iptables -A PREROUTING -t nat -i $WAN -p gre -d $EXT_IP -j DNAT --to $ip
             	     iptables -A FORWARD -i $WAN -p gre -j ACCEPT	
	             PPTP_VPN=1
	    	 fi
    	fi
    fi
    
    if [ $L2TP_VPN = 0 ]; then
    	if [ $port1 -le 1701 ] && [ $port2 -ge 1701 ];then
	    	 if [ $protocol = 2 ] || [ $protocol = 3 ]; then	    	     
	    	 	   L2TP_VPN=1
	    	 	   echo 0 > /proc/nat_l2tp
	    	 fi
	fi
    fi
	  
    if [ $IPSEC_VPN = 0 ]; then
    	if [ $port1 -le 500 ] && [ $port2 -ge 500 ];then
	    	 if [ $protocol = 2 ] || [ $protocol = 3 ]; then
	    	     iptables -A PREROUTING -t nat -p esp -d $EXT_IP -j DNAT --to $ip 
	    	     iptables -A PREROUTING -t nat -p udp --dport 4500 -d $EXT_IP -j DNAT --to $ip 
	    	     iptables -A FORWARD -p udp --dport 4500 -j ACCEPT
	    	     iptables -A FORWARD -p esp -j ACCEPT
	    	     IPSEC_VPN=1
	    	 fi
	    fi
    fi
    
  done
fi

eval `flash get VPN_PASSTHRU_PPTP_ENABLED`
if [ $PPTP_VPN = 0 ] && [ $VPN_PASSTHRU_PPTP_ENABLED = 0 ];then
       iptables -A FORWARD -p TCP --dport 1723 -j DROP
fi
eval `flash get VPN_PASSTHRU_L2TP_ENABLED`
if [ $L2TP_VPN = 0 ] && [ $VPN_PASSTHRU_L2TP_ENABLED = 0 ];then
       iptables -A FORWARD -p UDP --dport 1701 -j DROP
fi
eval `flash get VPN_PASSTHRU_IPSEC_ENABLED`
if [ $IPSEC_VPN = 0 ] && [ $VPN_PASSTHRU_IPSEC_ENABLED = 0 ];then
       iptables -A FORWARD -p UDP --dport 500 -j DROP
else
                iptables -A FORWARD -p udp --dport 500 -i $WAN -o br0 -j ACCEPT
fi

if [ "$DMZ_HOST" != '0.0.0.0' ] && [ $DMZ_ENABLED -gt 0 ];
then
  iptables -D INPUT -p tcp --dport 80:80 -i $WAN -d $EXT_IP -j DROP
#  iptables -D PREROUTING -t nat -i $WAN -p TCP --dport 80:80 -j DROP
  iptables -A PREROUTING -t nat -p ALL -d $EXT_IP -j DNAT --to $DMZ_HOST 
  iptables  -A FORWARD  -i $WAN -d $DMZ_HOST -p all -j ACCEPT
fi


#deny the ping request from WAN interface to bridge interface
if [ "$EXT_IP" != '' ] ; then
  if [ $PING_WAN_ACCESS_ENABLED = 0 ]; then
   iptables -A INPUT -p icmp --icmp-type echo-request -i $WAN -d $EXT_IP -j DROP
  else
   iptables -A INPUT -p icmp --icmp-type echo-request -i $WAN -d $EXT_IP -j ACCEPT
  fi
fi

# voip: allow SNMP udp port 
if [ "$EXT_IP" != '' ] && [ -f /bin/snmpd ]; then
  iptables -A INPUT -p udp --dport 161:161 -i $WAN -d $EXT_IP -j ACCEPT
fi

# voip: allow SIP udp port 
if [ "$EXT_IP" != '' ] && [ -f /bin/solar ]; then
  iptables -A INPUT -p udp --dport 5060:5060 -i $WAN -d $EXT_IP -j ACCEPT
fi

# let ipsec packet come in
if [ -f /bin/vpn.sh ] && [ $WAN_DHCP != 4 ] && [ $OP_MODE != 1 ] && [ $WAN_DHCP != 6 ]; then 
	eval `flash get IPSECTUNNEL_ENABLED`
	if [ $IPSECTUNNEL_ENABLED -gt 0 ]; then
		iptables -A INPUT -p 50 -i $WAN -j ACCEPT
		iptables -A INPUT -p 51 -i $WAN -j ACCEPT
		iptables -A INPUT -p udp --sport 500 --dport 500 -i $WAN -j ACCEPT
	fi
fi

iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
if [ $WAN_DHCP = 4 ]; then
  iptables -A FORWARD -i $WAN_Phy -m state --state ESTABLISHED,RELATED -j ACCEPT
fi

#if [ $WAN_DHCP = 0 -o $WAN_DHCP = 5 ]; then
#	eval `flash get DHCP_MTU_SIZE`
#	MTU=$DHCP_MTU_SIZE
#elif [ $WAN_DHCP = 1 ]; then
#	eval `flash get FIXED_IP_MTU_SIZE`
#	MTU=$FIXED_IP_MTU_SIZE
#elif [ $WAN_DHCP = 3 ]; then
#	eval `flash get PPP_MTU_SIZE`
#	MTU=`expr $PPP_MTU_SIZE - 40`
#elif [ $WAN_DHCP = 4 ]; then
#	eval `flash get PPTP_MTU_SIZE`
#	MTU=`expr $PPTP_MTU_SIZE - 48`	
#else
#	MTU=1460
#fi
###########################################################################
# For kernel-2.6, there is no table/chain named TCPMSS
#iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss $MTU

#iptables -A FORWARD -p udp -i $WAN -o $BRIDGE -j ACCEPT
#iptables -A FORWARD -p tcp -i $WAN -o $BRIDGE -j ACCEPT

# for igmp proxy,let WAN accept igmp query
eval `flash get IGMP_PROXY_DISABLED`
if [ $IGMP_PROXY_DISABLED = 0 ]; then
iptables -A FORWARD -i $WAN -p udp -m udp --destination  224.0.0.0/4 -j ACCEPT
#iptables -A INPUT -i $WAN -p IGMP -j ACCEPT
iptables -A INPUT -i $WAN -p 2 -j ACCEPT
fi

iptables -A FORWARD -p 50 -i $WAN -o $BRIDGE -j ACCEPT
iptables  -A FORWARD -i $WAN -m state --state ESTABLISHED,RELATED -j ACCEPT
#iptables -t mangle -I PREROUTING -i $BRIDGE -j MARK --set-mark 5
iptables  -A FORWARD -i $BRIDGE -j ACCEPT
#hyking:when layered driver enable, permit all icmp packet but icmp request...
iptables -A INPUT -p icmp -i $WAN -j ACCEPT

# for miniigd patch	
#Set a flag to allow mini-igd to append firewall rule
echo 1 > /tmp/firewall_igd

eval `flash get DOS_ENABLED`
if [ "$DOS_ENABLED" != 0 ]; then
 if [ "$OP_MODE" = '0' ];then
    echo "0" > /proc/hw_nat
  fi
  dos.sh &
fi

# WAN_DHCP=4 means pptp wan type
if [ $WAN_DHCP = 4 ]; then	
  iptables -t nat -A POSTROUTING -o $WAN_Phy -j MASQUERADE
fi
iptables -t nat -A POSTROUTING -o $WAN -j MASQUERADE

#add the default forward rule after filter
# set all the following value in init.d/rcS
##echo "1280" > /proc/sys/net/ipv4/ip_conntrack_max
##echo "600" > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established
##echo "60" > /proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout
##echo "5" > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_time_wait
##echo "5" > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_close

if [ "$CONFIG_RTL_HW_NAPT" = "y" ]; then
  iptables -A INPUT -j ACCEPT
else
  iptables -A INPUT ! -i $WAN -j ACCEPT
fi

#reply only if the target IP address is local address configured on the incoming interface
echo 1 > /proc/sys/net/ipv4/conf/eth1/arp_ignore

##enable the traffic control settings when OP_MODE is not equal to the bridge mode
# do not edit next line, it will be replaced to "CONFIG_NET_SCHED=y" or "CONFIG_NET_SCHED=" by user/script/Makefile
###CONFIG_NET_SCHED###
if [ "$OP_MODE" != '1' ]; then
	if [ "$CONFIG_NET_SCHED" = "y" ]; then
		echo "init ip_qos.sh"
		$START_IP_QOS
	fi
fi
