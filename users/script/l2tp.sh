#!/bin/sh
WAN=$1

L2TPCONF=/etc/ppp/l2tpd.conf
OPTIONS=/etc/ppp/options
RPPTP=/etc/ppp/peers/rpptp
PAPFILE=/etc/ppp/pap-secrets
CHAPFILE=/etc/ppp/chap-secrets
RESOLV=/etc/ppp/resolv.conf
PPPFILE=/var/run/ppp
FIRSTFILE=/etc/ppp/firstl2tp
FIRSTDEMAND=/etc/ppp/firstl2tp_demand
CONNECTFILE=/etc/ppp/connectfile
MCONNECTFILE=/var/ppp/mconnectfile
L2TP_IPDYN=/var/l2tp_dyn
L2TP_GW=/var/l2tp_gw
DNRD_PID=/var/run/dnrd.pid
eval `flash get L2TP_USER_NAME`
eval `flash get L2TP_PASSWORD`
eval `flash get L2TP_IP_ADDR`
eval `flash get L2TP_SUBNET_MASK`
eval `flash get L2TP_GATEWAY`
eval `flash get L2TP_SERVER_IP_ADDR`
eval `flash get L2TP_MTU_SIZE`
eval `flash get L2TP_IDLE_TIME`
eval `flash get DNS_MODE`
eval `flash get DNS1`
eval `flash get DNS2`
eval `flash get DNS3`
eval `flash get WAN_DHCP`
eval `flash get L2TP_CONNECTION_TYPE`
eval `flash get L2TP_WAN_IP_DYNAMIC`
if [ $1 = 'connect' ]; then
     ENABLE_CONNECT=1
      WAN=$2
else
     ENABLE_CONNECT=0
     WAN=$1
fi
route del default dev $WAN
ifconfig $WAN $L2TP_IP_ADDR  netmask $L2TP_SUBNET_MASK

RESOLV_CONF="/etc/resolv.conf"
DNS="--cache=off"
if [ $DNS_MODE = 1 ]; then
	if [ "$DNS1" != '0.0.0.0' ]; then
		DNS="$DNS -s $DNS1"
	fi
	if [ "$DNS2" != '0.0.0.0' ]; then
		DNS="$DNS -s $DNS2"
	fi
	if [ "$DNS3" != '0.0.0.0' ]; then
		DNS="$DNS -s $DNS3"
	fi
	dnrd $DNS
else
    dnrd $DNS -s 168.95.1.1 
fi

#generate options file even the user name is null
#if [ -n "$L2TP_USER_NAME" ] ; then
  echo "user \"$L2TP_USER_NAME\"" > $OPTIONS
  echo "#################################################" > $PAPFILE  
  echo "\"$L2TP_USER_NAME\"	*	\"$L2TP_PASSWORD\"" >> $PAPFILE
  echo "#################################################" > $CHAPFILE
  echo "\"$L2TP_USER_NAME\"	*	\"$L2TP_PASSWORD\"" >> $CHAPFILE
#fi

echo "lock" >> $OPTIONS  
echo "noauth" >> $OPTIONS  
echo "defaultroute" >> $OPTIONS  
echo "usepeerdns" >> $OPTIONS  
echo "lcp-echo-interval 0" >> $OPTIONS
echo "mtu $L2TP_MTU_SIZE" >> $OPTIONS
echo "wantype $WAN_DHCP" >> $OPTIONS
echo "name $L2TP_USER_NAME" >> $OPTIONS
echo "nodeflate" >>$OPTIONS
echo "nobsdcomp" >>$OPTIONS
echo "nodetach" >>$OPTIONS
echo "default-asyncmap" >>$OPTIONS
echo "nopcomp" >>$OPTIONS
echo "noaccomp" >>$OPTIONS
echo "noccp" >>$OPTIONS
echo "novj" >>$OPTIONS
echo "nobsdcomp" >> $OPTIONS  

len=`echo -n $L2TP_PASSWORD | wc -c`
len2=`echo $len`
if [ "$len2" -gt 35 ]; then #35=31+4
echo "-mschap" >> $OPTIONS 
echo "-mschap-v2" >> $OPTIONS 
fi

echo "[global]" > $L2TPCONF
echo "port = 1701" >> $L2TPCONF
echo "auth file = /etc/ppp/chap-secrets" >> $L2TPCONF
echo "[lac client]" >> $L2TPCONF
echo "lns=$L2TP_SERVER_IP_ADDR" >> $L2TPCONF
echo "require chap = yes" >> $L2TPCONF
echo "name = $L2TP_USER_NAME" >> $L2TPCONF
echo "pppoptfile = /etc/ppp/options" >> $L2TPCONF

#To kill l2tpd existed first
killall l2tpd 2>/dev/null
sleep 1

l2tpd&

# wating l2tpd to mkfifo var/run/l2tp-control,
# especially for sqaushfs
sleep 3
#echo "d client" > /var/run/l2tp-control 



if [ $L2TP_CONNECTION_TYPE = 2 ] && [ $ENABLE_CONNECT = 1 ]; then
    echo "c client" > /var/run/l2tp-control &
    exit    
fi

if [ $L2TP_CONNECTION_TYPE = 0 ] ; then
  {
  while [ true ]; do

  if [ $WAN_DHCP != 6 ] || [ $L2TP_CONNECTION_TYPE != 0 ]; then
    echo "d client" > /var/run/l2tp-control &
    break
  fi
  if [ ! -r "$CONNECTFILE" ]  && [ $L2TP_CONNECTION_TYPE = 0 ] && [ ! -r /var/disc ]; then
    echo "pass" > $CONNECTFILE
   
    if [ ! -f $FIRSTFILE ]; then
      echo "pass" > $FIRSTFILE
    fi
    echo "c client" > /var/run/l2tp-control &
  fi
  sleep 3
  done
  rm -f $FIRSTFILE
  } &
fi

if [ $L2TP_CONNECTION_TYPE = 1 ] ; then
  {
  echo "connect /etc/ppp/true" >> $OPTIONS
  echo "demand" >> $OPTIONS
  echo "idle $L2TP_IDLE_TIME" >> $OPTIONS  
  while [ true ]; do

  if [ $WAN_DHCP != 6 ] || [ $L2TP_CONNECTION_TYPE != 1 ]; then
    echo "d client" > /var/run/l2tp-control &
    break
  fi
  if [ ! -r "$CONNECTFILE" ]  && [ $L2TP_CONNECTION_TYPE = 1 ] && [ ! -r /var/disc ]; then
    echo "pass" > $CONNECTFILE
    
    if [ ! -f $FIRSTFILE ]; then
      echo "pass" > $FIRSTFILE
    fi
    killall -9 igmpproxy 2> /dev/null
    killall -9 pppd 2> /dev/null
    killall -9 dnrd 2> /dev/null
    
   	sleep 1
   
    echo "c client" > /var/run/l2tp-control &
    
  	dnrd --cache=off -s 168.95.1.1
  else
  	if [ ! -r /var/run/ppp0.pid ]; then
  		rm -f $CONNECTFILE 2> /dev/null
  	fi  
  fi
  sleep 3
  done
  rm -f $FIRSTFILE
  } &
fi

