#!/bin/sh
#WAN=eth1
WAN=$2
OPTIONS=/etc/ppp/options
PAPFILE=/etc/ppp/pap-secrets
CHAPFILE=/etc/ppp/chap-secrets
RESOLV=/etc/ppp/resolv.conf
LINKFILE=/etc/ppp/link
PPPFILE=/var/run/ppp
FIRSTFILE=/etc/ppp/first
FIRSTDEMAND=/etc/ppp/firstdemand
CONNECTFILE=/etc/ppp/connectfile
DNRDPIDFILE=/var/run/dnrd.pid
eval `flash get WAN_DHCP`
eval `flash get PPP_USER_NAME`
eval `flash get PPP_PASSWORD`
eval `flash get PPP_IDLE_TIME`
eval `flash get PPP_CONNECT_TYPE`
eval `flash get PPP_MTU_SIZE`
eval `flash get DNS_MODE`
eval `flash get DNS1`
eval `flash get DNS2`
eval `flash get DNS3`
eval `flash get PPP_SERVICE_NAME`
#MSS=`expr $PPP_MTU_SIZE - 40`
ifconfig $WAN 0.0.0.0
#route del default #gw 0.0.0.0 dev $WAN
#route add default $WAN
if [ $1 = 'connect' ]; then
     ENABLE_CONNECT=1
else
     ENABLE_CONNECT=0
fi
if [ -n "$PPP_USER_NAME" ] ; then
#  echo "name \"$PPP_USER_NAME\"" > $OPTIONS
#  echo "#################################################" > $PAPFILE  
#  echo "\"$PPP_USER_NAME\"	*	\"$PPP_PASSWORD\"" >> $PAPFILE
#  echo "#################################################" > $CHAPFILE
#  echo "\"$PPP_USER_NAME\"	*	\"$PPP_PASSWORD\"" >> $CHAPFILE
	/bin/flash gen-pppoe $OPTIONS $PAPFILE $CHAPFILE
fi

#if [ $WAN_DHCP = 4 ]; then
#  echo "1" > /proc/fast_pptp
#else
#  echo "0" > /proc/fast_pptp
#fi

#echo "sync" >> $OPTIONS
#echo "lock" >> $OPTIONS
echo "noauth" >>$OPTIONS
echo "nomppc" >>$OPTIONS
echo "noipdefault" >> $OPTIONS
echo "hide-password" >> $OPTIONS  
echo "defaultroute" >> $OPTIONS
echo "persist" >> $OPTIONS
echo "ipcp-accept-remote" >> $OPTIONS  
echo "ipcp-accept-local" >> $OPTIONS  
echo "nodetach" >>$OPTIONS
echo "usepeerdns" >> $OPTIONS
echo "mtu $PPP_MTU_SIZE" >> $OPTIONS
echo "mru $PPP_MTU_SIZE" >> $OPTIONS
echo "lcp-echo-interval 20" >> $OPTIONS  
echo "lcp-echo-failure 3" >> $OPTIONS
echo "wantype $WAN_DHCP" >> $OPTIONS
#mark_test
echo "holdoff 10" >> $OPTIONS

if [ -n "$PPP_SERVICE_NAME" ]; then
#  echo "plugin /etc/ppp/plugins/libplugin.a rp_pppoe_ac 62031090091393-Seednet_240_58 rp_pppoe_service $PPP_SERVICE_NAME $WAN" >> $OPTIONS
 echo "plugin /etc/ppp/plugins/libplugin.a rp_pppoe_service $PPP_SERVICE_NAME $WAN" >> $OPTIONS
else
 echo "plugin /etc/ppp/plugins/libplugin.a $WAN" >> $OPTIONS
fi

PID_FILE=/var/run/ppp0.pid
DNRD_PID=/var/run/dnrd.pid

if [ ! -f $DNRD_PID ]; then
   DNS="--cache=off"
   if [ $DNS_MODE != 1 ]; then
      dnrd $DNS -s 168.95.1.1
   fi
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
   fi
fi



if [ -r "$PPPFILE" ]; then
  rm $PPPFILE
fi

if [ $PPP_CONNECT_TYPE = 0 ] ; then
  {
  while [ true ]; do

  if [ $WAN_DHCP != 3 ] || [ $PPP_CONNECT_TYPE != 0 ]; then
    break
  fi
  if [ ! -r "$CONNECTFILE" ]  && [ $PPP_CONNECT_TYPE = 0 ]; then
    echo "pass" > $CONNECTFILE
    if [ ! -f $FIRSTFILE ]; then
      echo "pass" > $FIRSTFILE
    fi
    pppd 
  fi
  sleep 5
  done
  rm -f $FIRSTFILE
  } &
fi
 
if [ $PPP_CONNECT_TYPE = 1 ] ; then
  {
  echo "demand" >> $OPTIONS
  echo "idle $PPP_IDLE_TIME" >> $OPTIONS

  #hyking: when connection is dial on demand, forward all packet...
  #iptables -P INPUT ACCEPT
  #iptables -P FORWARD ACCEPT 
  #echo 1 > /proc/sys/net/ipv4/ip_forward

  while [ true ]; do

  if [ $WAN_DHCP != 3 ] || [ $PPP_CONNECT_TYPE != 1 ]; then
    break
  fi
  if [ ! -r "$CONNECTFILE" ]  && [ $PPP_CONNECT_TYPE = 1 ]; then
    echo "pass" > $CONNECTFILE
    if [ ! -f $FIRSTDEMAND ]; then
      echo "pass" > $FIRSTDEMAND
    fi
    pppd 
     if [ -f $DNRDPIDFILE ]; then
      	PID=`cat $DNRDPIDFILE`
      	kill -9 $PID 
      	rm -f $DNRDPIDFILE
    fi
    dnrd --cache=off -s 168.95.1.1
  fi
  sleep 5
  done
  rm -f $FIRSTDEMAND
  } &
fi


if [ $PPP_CONNECT_TYPE = 2 ]; then
   if [ $ENABLE_CONNECT = 1 ]; then
   #sleep 5
   pppd &
   fi
fi

