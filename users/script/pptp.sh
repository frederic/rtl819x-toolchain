#!/bin/sh
#WAN=eth1
WAN=$1
OPTIONS=/etc/ppp/options
RPPTP=/etc/ppp/peers/rpptp
PAPFILE=/etc/ppp/pap-secrets
CHAPFILE=/etc/ppp/chap-secrets
RESOLV=/etc/ppp/resolv.conf
PPPFILE=/var/run/ppp
FIRSTFILE=/etc/ppp/firstpptp
CONNECTFILE=/etc/ppp/connectfile
eval `flash get WAN_DHCP`
eval `flash get PPTP_USER_NAME`
eval `flash get PPTP_PASSWORD`
eval `flash get PPTP_IP_ADDR`
eval `flash get PPTP_SUBNET_MASK`
eval `flash get PPTP_SERVER_IP_ADDR`
eval `flash get PPTP_MTU_SIZE`
eval `flash get PPTP_CONNECTION_TYPE`
eval `flash get PPTP_IDLE_TIME`
eval `flash get DNS_MODE`
eval `flash get DNS1`
eval `flash get DNS2`
eval `flash get DNS3`
eval `flash get WAN_DHCP`

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

if [ $1 = 'connect' ]; then
     ENABLE_CONNECT=1
     WAN=$2
else
     ENABLE_CONNECT=0
     WAN=$1
fi

route del  default dev $WAN
ifconfig $WAN $PPTP_IP_ADDR  netmask $PPTP_SUBNET_MASK

if [ -n "$PPTP_USER_NAME" ] ; then
#  echo "name \"$PPTP_USER_NAME\"" > $OPTIONS
#  echo "#################################################" > $PAPFILE  
#  echo "\"$PPTP_USER_NAME\"	*	\"$PPTP_PASSWORD\"" >> $PAPFILE
#  echo "#################################################" > $CHAPFILE
#  echo "\"$PPTP_USER_NAME\"	*	\"$PPTP_PASSWORD\"" >> $CHAPFILE
	flash gen-pptp $OPTIONS $PAPFILE $CHAPFILE
fi

echo "lock" >> $OPTIONS  
echo "noauth" >> $OPTIONS  
echo "nobsdcomp" >> $OPTIONS  
echo "nodeflate" >> $OPTIONS  
echo "usepeerdns" >> $OPTIONS  
echo "lcp-echo-interval 20" >> $OPTIONS
echo "lcp-echo-failure 3" >> $OPTIONS
echo "mtu $PPTP_MTU_SIZE" >> $OPTIONS
echo "wantype $WAN_DHCP" >> $OPTIONS
echo "holdoff 2" >> $OPTIONS

echo "remotename PPTP" > $RPPTP
echo "linkname PPTP" >> $RPPTP
echo "ipparam PPTP" >> $RPPTP
echo "pty \"pptp $PPTP_SERVER_IP_ADDR  --nolaunchpppd\"" >> $RPPTP
echo "name $PPTP_USER_NAME" >> $RPPTP

eval `flash get PPTP_SECURITY_ENABLED`
if [ $PPTP_SECURITY_ENABLED != 0 ]; then
#	echo "require-mppe" >> $RPPTP
#	echo "require-mppe-40" >> $RPPTP
#	echo "require-mppe-128" >> $RPPTP
#	echo "nomppe-stateful" >> $RPPTP
	echo "+mppe required,stateless" >> $RPPTP
fi

eval `flash get PPTP_MPPC_ENABLED`
if [ $PPTP_MPPC_ENABLED != 0 ]; then
	echo "mppc" >> $RPPTP
	echo "stateless" >> $RPPTP
else
	echo "nomppc" >> $RPPTP
fi

if [ $PPTP_SECURITY_ENABLED = 0 ] && [ $PPTP_MPPC_ENABLED = 0 ]; then
	echo "noccp" >>$RPPTP
fi
echo "persist" >> $RPPTP
echo "noauth" >>$RPPTP
echo "file /etc/ppp/options" >>$RPPTP
echo "nobsdcomp" >>$RPPTP
echo "nodetach" >>$RPPTP
echo "novj" >>$RPPTP

PID_FILE=/var/run/ppp0.pid
DNRD_PID=/var/run/dnrd.pid

if [ $PPTP_CONNECTION_TYPE = 0 ]; then
  {
  while [ true ]; do
 
  if [ $WAN_DHCP != 4 ] || [ $PPTP_CONNECTION_TYPE != 0 ]; then
    break
  fi
  if [ ! -r "$CONNECTFILE" ]  && [ $PPTP_CONNECTION_TYPE = 0 ]; then
    echo "pass" > $CONNECTFILE
 
    if [ ! -f $FIRSTFILE ]; then
      echo "pass" > $FIRSTFILE
    fi
    pppd call rpptp
  fi
  sleep 5
  done
  rm -f $FIRSTFILE
  } &
fi
 
 
if [ $PPTP_CONNECTION_TYPE = 1 ] ; then
  {
  echo "persist" >> $OPTIONS
  echo "nodetach" >>$OPTIONS
  echo "connect /etc/ppp/true" >> $OPTIONS
  echo "demand" >> $OPTIONS
  echo "idle $PPTP_IDLE_TIME" >> $OPTIONS
  echo "ktune" >> $OPTIONS
  echo "ipcp-accept-remote" >> $OPTIONS
  echo "ipcp-accept-local" >> $OPTIONS
  echo "noipdefault" >> $OPTIONS
  echo "defaultroute" >> $OPTIONS
  echo "hide-password" >> $OPTIONS
  while [ true ]; do
 
  if [ $WAN_DHCP != 4 ] || [ $PPTP_CONNECTION_TYPE != 1 ]; then
    break
  fi
 
  if [ ! -r "$CONNECTFILE" ]  && [ $PPTP_CONNECTION_TYPE = 1 ]; then
    echo "pass" > $CONNECTFILE
 
    if [ ! -f $FIRSTDEMAND ]; then
      echo "pass" > $FIRSTDEMAND
    fi
    killall -9 igmpproxy 2> /dev/null
    killall -9 dnrd 2> /dev/null
    killall -9 pppd 2> /dev/null
    dnrd --cache=off -s 168.95.1.1
    pppd call rpptp
  fi
  sleep 5
  done
  rm -f $FIRSTDEMAND
  }&
fi
 
if [ $PPTP_CONNECTION_TYPE = 2 ] ; then
   if [ $ENABLE_CONNECT = 1 ]; then
   pppd call rpptp &
   fi
fi
