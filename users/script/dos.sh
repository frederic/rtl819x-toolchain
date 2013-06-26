#!/bin/sh
TMPFILE=/tmp/dosfile
eval `flash get OP_MODE`
eval `flash get DOS_ENABLED`
eval `flash get DOS_SYSSYN_FLOOD`
eval `flash get DOS_SYSFIN_FLOOD`
eval `flash get DOS_SYSUDP_FLOOD`
eval `flash get DOS_SYSICMP_FLOOD`
eval `flash get DOS_PIPSYN_FLOOD`
eval `flash get DOS_PIPFIN_FLOOD`
eval `flash get DOS_PIPUDP_FLOOD`
eval `flash get DOS_PIPICMP_FLOOD`
eval `flash get DOS_BLOCK_TIME`

cat /proc/net/route | grep br0 > $TMPFILE
line=`cat $TMPFILE | wc -l`
num=1
while [ $num -le $line ];
 do
  str=` head -n $num $TMPFILE | tail -n 1`
  addr=`echo $str | cut -f2 -d " "`
  if [ "$addr" != 'FFFFFFFF' ] && [ "$addr" != 'E00001B2' ] && [ "$addr" != 'EFFFFFFA' ]; then
     mask=`echo $str | cut -f8 -d " "`
     break
  fi
 num=`expr $num + 1`
 done


if [ $OP_MODE = 2 ]; then
  echo "2 $addr $mask $DOS_ENABLED $DOS_SYSSYN_FLOOD $DOS_SYSFIN_FLOOD $DOS_SYSUDP_FLOOD $DOS_SYSICMP_FLOOD $DOS_PIPSYN_FLOOD $DOS_PIPFIN_FLOOD $DOS_PIPUDP_FLOOD $DOS_PIPICMP_FLOOD $DOS_BLOCK_TIME" > /proc/enable_dos
else
  echo "0 $addr $mask $DOS_ENABLED $DOS_SYSSYN_FLOOD $DOS_SYSFIN_FLOOD $DOS_SYSUDP_FLOOD $DOS_SYSICMP_FLOOD $DOS_PIPSYN_FLOOD $DOS_PIPFIN_FLOOD $DOS_PIPUDP_FLOOD $DOS_PIPICMP_FLOOD $DOS_BLOCK_TIME" > /proc/enable_dos
fi
  
rm -f /tmp/dosfile



