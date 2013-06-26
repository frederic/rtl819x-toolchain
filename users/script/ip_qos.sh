#!/bin/sh
#
# script file for traffic control (QoS)
#

WAN=eth1
WAN2=
BRIDGE=br0

eval `flash get WAN_DHCP`
eval `flash get OP_MODE`

# if wireless ISP mode , set WAN to wlan0
eval `flash get  WISP_WAN_ID`
if [ "$OP_MODE" = '2' ];then
	WAN=wlan$WISP_WAN_ID
fi

## DHCP_DISABLED=0, DHCP_CLIENT=1, DHCP_SERVER=2, PPPOE=3, PPTP=4, BIGPOND=5, L2TP=6
if [ $WAN_DHCP = 3 ] || [ $WAN_DHCP = 4 ]; then
  WAN=ppp0
fi

if [ $WAN_DHCP = 6 ]; then
  WAN2=ppp0
fi

eval `flash get QOS_ENABLED`
eval `flash get QOS_AUTO_UPLINK_SPEED`
eval `flash get QOS_MANUAL_UPLINK_SPEED`
eval `flash get QOS_AUTO_DOWNLINK_SPEED`
eval `flash get QOS_MANUAL_DOWNLINK_SPEED`
eval `flash get QOS_RULE_TBL_NUM`

## for "mode" field of QOS_RULE_TBL
## 		guaranteed minimum bandwidth	0x01
##		restricted maximum bandwidth	0x02
##		by IP address					0x04
##		by MAC address				0x08

iptables -F -t mangle
iptables -X -t mangle
iptables -Z -t mangle

# To avoid rule left when wan changed
tc qdisc del dev eth1 root 2> /dev/null
tc qdisc del dev ppp0 root 2> /dev/null

if [ "$WAN" != "eth1" ] && [ "$WAN" != "ppp0" ]; then
  tc qdisc del dev $WAN root 2> /dev/null
fi

tc qdisc del dev $BRIDGE root 2> /dev/null

## this line is in firewall.sh, run it again because mangle table is cleared before.
#iptables -t mangle -I PREROUTING -i $BRIDGE -j MARK --set-mark 5


PROC_QOS="0,"

UPLINK_SPEED=102400
DOWNLINK_SPEED=102400

# do not edit the next line
###CONFIG_RTL_8198_GW###
if [ "$CONFIG_RTL_8198_GW" = "y" ]; then
UPLINK_SPEED=1024000
DOWNLINK_SPEED=1024000
fi

if [ "$QOS_ENABLED" = '1' ];then

    ###### uplink speed
    if [ "$QOS_AUTO_UPLINK_SPEED" = '0' ];then
        UPLINK_SPEED=$QOS_MANUAL_UPLINK_SPEED
    	  if [ $UPLINK_SPEED -lt 100 ];then
	      UPLINK_SPEED=100
    	  fi
    fi

    ## Patch for uplink QoS accuracy
    if [ "$CONFIG_RTL_8198_GW" = "y" ]; then
          if [ $UPLINK_SPEED -gt 160000 ]; then
              UPLINK_SPEED=160000
          fi
    else
          if [ $UPLINK_SPEED -gt 75000 ]; then
                        UPLINK_SPEED=75000
          fi
    fi

    ###### downlink speed
    if [ "$QOS_AUTO_DOWNLINK_SPEED" = '0' ];then
        DOWNLINK_SPEED=$QOS_MANUAL_DOWNLINK_SPEED
    	  if [ $DOWNLINK_SPEED -lt 100 ];then
	      DOWNLINK_SPEED=100
    	  fi
    fi

    ## Patch for downlink QoS accuracy
    if [ "$CONFIG_RTL_8198_GW" = "y" ]; then
          if [ $DOWNLINK_SPEED -gt 130000 ]; then
              DOWNLINK_SPEED=130000
          fi
    else
          if [ $DOWNLINK_SPEED -gt 70000 ]; then
              DOWNLINK_SPEED=70000
          fi
    fi

    wan_pkt_mark=13
    lan_pkt_mark=53



    ###### total bandwidth section
    ### uplink
    tc qdisc add dev $WAN root handle 2:0 htb default 2 r2q 64                   
    TC_CMD="tc class add dev $WAN parent 2:0 classid 2:1 htb rate ${UPLINK_SPEED}kbit ceil ${UPLINK_SPEED}kbit quantum 30000"
    $TC_CMD
    TC_CMD="tc class add dev $WAN parent 2:1 classid 2:2 htb rate 1kbit ceil ${UPLINK_SPEED}kbit prio 256 quantum 30000"
    $TC_CMD
    TC_CMD="tc qdisc add dev $WAN parent 2:2 handle 102: sfq perturb 10"
    $TC_CMD

    if [ "$WAN2" = "ppp0" ]; then
    tc qdisc add dev $WAN2 root handle 3:0 htb default 2 r2q 64
    TC_CMD="tc class add dev $WAN2 parent 3:0 classid 3:1 htb rate ${UPLINK_SPEED}kbit ceil ${UPLINK_SPEED}kbit quantum 30000"
    $TC_CMD
    TC_CMD="tc class add dev $WAN2 parent 3:1 classid 3:2 htb rate 1kbit ceil ${UPLINK_SPEED}kbit prio 256 quantum 30000"
    $TC_CMD
    TC_CMD="tc qdisc add dev $WAN2 parent 3:2 handle 302: sfq perturb 10"
    $TC_CMD
    fi

    ### downlink
    tc qdisc add dev $BRIDGE root handle 5:0 htb default 2 r2q 64                   
    TC_CMD="tc class add dev $BRIDGE parent 5:0 classid 5:1 htb rate ${DOWNLINK_SPEED}kbit ceil ${DOWNLINK_SPEED}kbit quantum 30000"
    $TC_CMD
    TC_CMD="tc class add dev $BRIDGE parent 5:1 classid 5:2 htb rate 1kbit ceil ${DOWNLINK_SPEED}kbit prio 256 quantum 30000"
    $TC_CMD
    TC_CMD="tc qdisc add dev $BRIDGE parent 5:2 handle 502: sfq perturb 10"
    $TC_CMD

    PROC_QOS="1,"

    ###### QoS rules section
    if [ $QOS_RULE_TBL_NUM -gt 0 ];then

        num=1
  
        while [ $num -le $QOS_RULE_TBL_NUM ];
        do
            
            str=`flash get QOS_RULE_TBL | grep QOS_RULE_TBL$num`
            str=`echo $str | cut -f2 -d=`
            enabled=`echo $str | cut -f1 -d,`
            
            if [ $enabled -gt 0 ];then
            
                ### include leading space character
                mac_addr=`echo $str | cut -f2 -d,`
                mode=`echo $str | cut -f3 -d,`
                lo_ip_start=`echo $str | cut -f4 -d,`
                lo_ip_end=`echo $str | cut -f5 -d,`
                bandwidth=`echo $str | cut -f6 -d,`
                bandwidth_dl=`echo $str | cut -f7 -d,`

                ### remove leading space character
                mac_addr=`echo $mac_addr | cut -f2 -d" "`
                mode=`echo $mode | cut -f2 -d" "`
                lo_ip_start=`echo $lo_ip_start | cut -f2 -d" "`
                lo_ip_end=`echo $lo_ip_end | cut -f2 -d" "`
                bandwidth=`echo $bandwidth | cut -f2 -d" "`
                bandwidth_dl=`echo $bandwidth_dl | cut -f2 -d" "`


                if [ $bandwidth -gt  0 ]; then
                
	                if [ "$mode" = '5' ] || [ "$mode" = '6' ]; then
	
	                    ## this qos rule is set by IP address            	   	
	                    IPT_CMD="iptables -A PREROUTING -t mangle -m iprange --src-range ${lo_ip_start}-${lo_ip_end} -j MARK --set-mark $wan_pkt_mark"  
	                    $IPT_CMD
	                    
	                else
	
	                    mac_1=`echo $mac_addr | cut -b 1-2`
	                    mac_2=`echo $mac_addr | cut -b 3-4`
	                    mac_3=`echo $mac_addr | cut -b 5-6`
	                    mac_4=`echo $mac_addr | cut -b 7-8`
	                    mac_5=`echo $mac_addr | cut -b 9-10`
	                    mac_6=`echo $mac_addr | cut -b 11-12`
	                    IPT_CMD="iptables -A PREROUTING -t mangle -m mac --mac-source ${mac_1}:${mac_2}:${mac_3}:${mac_4}:${mac_5}:${mac_6} -j MARK --set-mark $wan_pkt_mark"
	                    $IPT_CMD
	                fi
	
	                if [ "$mode" = '5' ] || [ "$mode" = '9' ]; then
	                    ## "guaranteed minimum bandwidth" mode
	                    TC_CMD="tc class add dev $WAN parent 2:1 classid 2:$wan_pkt_mark htb rate ${bandwidth}kbit ceil ${UPLINK_SPEED}kbit prio 2 quantum 30000"
	                else
	                    TC_CMD="tc class add dev $WAN parent 2:1 classid 2:$wan_pkt_mark htb rate 1kbit ceil ${bandwidth}kbit prio 2 quantum 30000"
	                fi
	                $TC_CMD
	
	                TC_CMD="tc qdisc add dev $WAN parent 2:$wan_pkt_mark handle 1$wan_pkt_mark: sfq perturb 10"
	                $TC_CMD
	
	                TC_CMD="tc filter add dev $WAN parent 2:0 protocol ip prio 100 handle $wan_pkt_mark fw classid 2:$wan_pkt_mark"
	                $TC_CMD

			
			if [ "$WAN2" = "ppp0" ]; then
			if [ "$mode" = '5' ] || [ "$mode" = '9' ]; then
                            ## "guaranteed minimum bandwidth" mode
                            TC_CMD="tc class add dev $WAN2 parent 3:1 classid 3:$wan_pkt_mark htb rate ${bandwidth}kbit ceil ${UPLINK_SPEED}kbit prio 2 quantum 30000"
                        else
                            TC_CMD="tc class add dev $WAN2 parent 3:1 classid 3:$wan_pkt_mark htb rate 1kbit ceil ${bandwidth}kbit prio 2 quantum 30000"
                        fi
                        $TC_CMD
 
                        TC_CMD="tc qdisc add dev $WAN2 parent 3:$wan_pkt_mark handle 3$wan_pkt_mark: sfq perturb 10"
                        $TC_CMD
 
                        TC_CMD="tc filter add dev $WAN2 parent 3:0 protocol ip prio 100 handle $wan_pkt_mark fw classid 3:$wan_pkt_mark"
                        $TC_CMD
			fi
	            
	                wan_pkt_mark=`expr $wan_pkt_mark + 1`

                fi


                if [ $bandwidth_dl -gt  0 ]; then
                
	                if [ "$mode" = '5' ] || [ "$mode" = '6' ]; then
	
	                    ## this qos rule is set by IP address            	   	
	                    IPT_CMD="iptables -A POSTROUTING -t mangle -m iprange --dst-range ${lo_ip_start}-${lo_ip_end} -j MARK --set-mark $lan_pkt_mark"  
	                    $IPT_CMD
	                    
	                else

	                    mac_1=`echo $mac_addr | cut -b 1-2`
	                    mac_2=`echo $mac_addr | cut -b 3-4`
	                    mac_3=`echo $mac_addr | cut -b 5-6`
	                    mac_4=`echo $mac_addr | cut -b 7-8`
	                    mac_5=`echo $mac_addr | cut -b 9-10`
	                    mac_6=`echo $mac_addr | cut -b 11-12`
	                    IPT_CMD="iptables -A POSTROUTING -t mangle -m mac --mac-destination ${mac_1}:${mac_2}:${mac_3}:${mac_4}:${mac_5}:${mac_6} -j MARK --set-mark $lan_pkt_mark"
	                    $IPT_CMD
	                fi

	                if [ "$mode" = '5' ] || [ "$mode" = '9' ]; then
	                    ## "guaranteed minimum bandwidth" mode
	                    TC_CMD="tc class add dev $BRIDGE parent 5:1 classid 5:$lan_pkt_mark htb rate ${bandwidth_dl}kbit ceil ${DOWNLINK_SPEED}kbit prio 2 quantum 30000"
	                else
	                    TC_CMD="tc class add dev $BRIDGE parent 5:1 classid 5:$lan_pkt_mark htb rate 1kbit ceil ${bandwidth_dl}kbit prio 2 quantum 30000"
	                fi
	                $TC_CMD
	
	                TC_CMD="tc qdisc add dev $BRIDGE parent 5:$lan_pkt_mark handle 5$lan_pkt_mark: sfq perturb 10"
	                $TC_CMD
	
	                TC_CMD="tc filter add dev $BRIDGE parent 5:0 protocol ip prio 100 handle $lan_pkt_mark fw classid 5:$lan_pkt_mark"
	                $TC_CMD
	            
	                lan_pkt_mark=`expr $lan_pkt_mark + 1`

                fi

            fi

            num=`expr $num + 1`
        done    
    fi
fi

echo "$PROC_QOS" > /proc/qos

