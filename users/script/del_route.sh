#!/bin/sh

eval `flash get STATICROUTE_ENABLED`
eval `flash get STATICROUTE_TBL_NUM`
if [ $STATICROUTE_TBL_NUM -gt 0 ] && [ $STATICROUTE_ENABLED -gt 0 ];
then
  num=1
  while [ $num -le $STATICROUTE_TBL_NUM ];
  do
    STATICROUTE_TBL=`flash get STATICROUTE_TBL | grep STATICROUTE_TBL$num=`
    tmp=`echo $STATICROUTE_TBL | cut -f2 -d=`
    dstIp=`echo $tmp | cut -f1 -d,`
    netmask=`echo $tmp | cut -f2 -d,`
    gateway=`echo $tmp | cut -f3 -d,`
    mr=`echo $tmp | cut -f4 -d,`
    iface=`echo $tmp | cut -f5 -d,`
    #route del -net $dstIp netmask 255.255.255.255 metric $mr
    route del -net $dstIp netmask $netmask gw $gateway 2> /dev/null
     
   
    num=`expr $num + 1`
  done
fi

