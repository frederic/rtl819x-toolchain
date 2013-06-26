#!/bin/sh
#PPPDFILE=/var/run/ppp.pid
#DNRDFILE=/tmp/dnrd_cmd
#PPPOEINFO=/tmp/pppoe_info
 
#To kill pppoe_info
#rm -f $PPPOEINFO

echo "Restart pppd ..."
#To restore dnrd
#CMD=`cat $DNRDFILE`
#$CMD
 
#To restore igmpproxy
#if [ -f "$PPPDFILE1" ]; then
#       igmpproxy ppp1 br0 &
#fi

#killall pppd 2> /dev/null
#sleep 3

eval `flash get WAN_DHCP`

#pppoe
rtl_start_pppoe() {
  #killall pppd 2> /dev/null
  #killall pppoe.sh 2> /dev/null
  #rm -f /etc/ppp/connectfile 2>/dev/null

  #Not trigger to WAN automatically
  killall sleep 2>/dev/null
  killall ntp.sh 2>/dev/null
  rm -f /tmp/ntp_tmp 2>/dev/null

  sleep 3
  dnrd --cache=off -s 168.95.1.1&
  pppd&
  #/bin/pppoe.sh all eth1&
}

#pptp
rtl_start_pptp() {
  #Not trigger to WAN automatically
  killall sleep 2>/dev/null
  killall ntp.sh 2>/dev/null
  rm -f /tmp/ntp_tmp 2>/dev/null

  killall pppd 2> /dev/null
  killall pptp 2> /dev/null
  killall pptp.sh 2> /dev/null
  rm -f /etc/ppp/connectfile 2>/dev/null
  sleep 3
#  pppd call rpptp&
  /bin/pptp.sh eth1&
}

#l2tp
rtl_start_l2tp() {
  #Not trigger to WAN automatically
  killall sleep 2>/dev/null
  killall ntp.sh 2>/dev/null
  rm -f /tmp/ntp_tmp 2>/dev/null

  echo "d client" > /var/run/l2tp-control&
#  killall l2tp.sh 2>/dev/null
#  killall l2tpd 2> /dev/null
#  killall pppd 2> /dev/null
   sleep 3
   rm -f /etc/ppp/connectfile 2>/dev/null
#  sleep 5
#  /bin/l2tp.sh eth1&
}


rtl_restart_pppd() {
	if [ "$WAN_DHCP" = '3' ]; then
		rtl_start_pppoe
	elif [ "$WAN_DHCP" = '4' ]; then
		rtl_start_pptp
	elif [ "$WAN_DHCP" = '5' ]; then
		rtl_start_l2tp
	fi
}

rtl_restart_pppd

