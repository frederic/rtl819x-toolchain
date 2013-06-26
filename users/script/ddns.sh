#!/bin/sh
FIRSTDDNS=/var/firstddns
OLDIP=/var/oldip
LINKFILE=/etc/ppp/link
ETH1_IP=/var/eth1_ip
renew=1
num=0
#echo "Re-Start DDNS"
killall -9 updatedd 2> /dev/null

###########kill sleep that ddns.sh created###############
TMPFILEDDNS=/tmp/tmpfileddns
line=0
ps | grep 'sleep 86430' > $TMPFILEDDNS
line=`cat $TMPFILEDDNS | wc -l`
num_sleep=1
while [ $num_sleep -le $line ];
do
pat0=` head -n $num_sleep $TMPFILEDDNS | tail -n 1`
pat1=`echo $pat0 | cut -f1 -dS`  
pat2=`echo $pat1 | cut -f1 -d " "`  
kill -9 $pat2 2> /dev/null
num_sleep=`expr $num_sleep + 1`
done
rm -f /var/firstddns 2> /dev/null
rm -f /tmp/tmpfileddns 2> /dev/null
###########################
eval `flash get DDNS_ENABLED`
eval `flash get WAN_DHCP`
eval `flash get DNS_MODE`
eval `flash get DDNS_TYPE`
eval `flash get DDNS_DOMAIN_NAME`
eval `flash get DDNS_USER`
eval `flash get DDNS_PASSWORD`


####
if [ ! -f $FIRSTDDNS ] || [ $1 = 'option' ]; then
{
echo "pass" > $FIRSTDDNS
while [ true ];do
  
 
  if [ ! -f $FIRSTDDNS ]; then
    break
  fi
  if [ $DDNS_ENABLED = 0 ]; then
    rm -f $FIRSTDDNS
    break
  fi
 
  if [ $WAN_DHCP = 1 ] || [ $WAN_DHCP = 0 ]; then
  if [ $WAN_DHCP = 1 ]; then
  if [ ! -f $ETH1_IP ]; then
  	 sleep 10
     continue
  fi
  fi
  
  fi
  
  #-- keith: add l2tp support. 20080515
  if [ $WAN_DHCP = 3 ] || [ $WAN_DHCP = 4 ] || [ $WAN_DHCP = 6 ]; then
   if [ ! -f $LINKFILE ]; then
   	 sleep 10
     continue
   fi
  
  fi
    if [ $DDNS_TYPE = 0 ]; then
      updatedd dyndns $DDNS_USER:$DDNS_PASSWORD $DDNS_DOMAIN_NAME
      ret=`echo $?`
    else 
      updatedd tzo $DDNS_USER:$DDNS_PASSWORD $DDNS_DOMAIN_NAME
      ret=`echo $?`
    fi

    if [ $ret = 0 ]; then
	echo "DDNS update successfully"
      num=0
      renew=0
      success=1
    else
    	success=0
    fi
    if [ $success = 1 ] ;then
    	sleep 86430
    else
    	sleep 300
    fi
done
} &
fi

