#!/bin/sh
{
while [ true ]; do
ch=`cat /proc/load_default`
if [ $ch = 1 ]; then
  echo "Going to Reload Default"
  flash reset
  sleep 3
  reboot
fi
sleep 3
done
} &



