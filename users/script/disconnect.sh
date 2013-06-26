#!/bin/sh
LINKFILE=/etc/ppp/link
PPPFILE=/var/run/ppp0.pid
CONNECTFILE=/etc/ppp/connectfile
PPPFILE1=/var/run/ppp1.pid
PPTPLINKFILE=/var/run/ppp-PPTP.pid
TMPFILE=/tmp/tmpfile
PLUTO_PID=/var/run/pluto.pid
FIRSTDDNS=/var/firstddns
TMPNTP=/tmp/ntp_tmp

eval `flash get PPP_CONNECT_TYPE`
eval `flash get WAN_DHCP`
eval `flash get L2TP_CONNECTION_TYPE`
eval `flash get PPTP_CONNECTION_TYPE`
##PPP_CONNECT_TYPE=0
##WAN_DHCP=5
##L2TP_CONNECTION_TYPE=0

# for miniigd patch
#if [ $1 = 'all' ]; then
  #killall -9 upnpd 2> /dev/null
#fi

#flush ip_conntrack & fp_path
#echo "disconnect....."
#patch for l2tp
if [ $WAN_DHCP = 6 ] && [ "$1" = 'option_l2tp' ]; then
#  echo "Do nothing for disconnect!"
  exit 0
fi

#except l2tp dial on demand
#echo "Do real disconnect!"
echo "enter" > /var/disc
#echo "1">/proc/net/flush_conntrack
#flush all conntrack
echo "2">/proc/fast_nat

killall sleep 2>/dev/null
killall -9 ntp.sh 2> /dev/null
killall date 2> /dev/null
killall ntpclient 2> /dev/null
#kill igmpproxy before kill pppd
killall -9 igmpproxy 2> /dev/null
echo 0 > /proc/br_igmpsnoop

killall -9 ddns.sh 2> /dev/null

PIDFILE=/var/run/dnrd.pid
if [ -f $PIDFILE ] ; then
	killall -9 dnrd 2> /dev/null
	rm -f $PIDFILE
fi

#killall -9 pppoe.sh 2>/dev/null
#killall -9 pptp.sh 2>/dev/null
#killall -9 l2tp.sh 2>/dev/null

#pppoe
if [ $WAN_DHCP = 3 ]; then
   killall -15 pppd 2> /dev/null
else
   if [ $WAN_DHCP != 6 ]; then
     killall -9 pppd 2> /dev/null
   fi
fi

killall -9 pppoe 2> /dev/null
killall -9 pptp 2> /dev/null

if [ -r "$PPPFILE" ]; then
  rm $PPPFILE
fi
if [ -r "$PPPFILE1" ]; then
  rm $PPPFILE1
fi

#l2tp
if [ $WAN_DHCP = 6 ]; then
  if [ "$1" != "option" ]; then
    killall l2tp.sh 2>/dev/null
    killall pppd 2> /dev/null
#    killall l2tpd 2> /dev/null
  fi
fi

if [ -f $PLUTO_PID ];then
  ipsec setup stop
fi
 
if [ -r "$FIRSTDDNS" ]; then
  rm $FIRSTDDNS
fi

if [ -r "$TMPNTP" ]; then
  rm $TMPNTP
fi

if [ "$1" = "option" ] && [ -r "$LINKFILE" ]; then
  rm -f /etc/ppp/first*
fi

if [ -r "$CONNECTFILE" ]; then
  rm -f $CONNECTFILE
fi
if [ -r "$LINKFILE" ]; then
  rm $LINKFILE
fi

if [ -r "$PPTPLINKFILE" ]; then
  rm $PPTPLINKFILE
fi

if [ $WAN_DHCP = 4 ] && [ $PPTP_CONNECTION_TYPE = 1 ]; then
  echo "3" > /proc/pptp_conn_ck
else
  echo "0" > /proc/pptp_conn_ck
fi

rm -f /var/disc 2> /dev/null
rm -f /var/disc_l2tp 2> /dev/null

#set un_flush flag for ip_conntrack & fp_path
#echo "0">/proc/net/flush_conntrack
echo "2">/proc/fast_nat
#echo "disconnect finished!!!"

