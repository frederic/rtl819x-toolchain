#!/bin/sh
#
# script file to start bridge
#
# Usage: bridge.sh br_interface lan1_interface wlan_interface[1]..wlan_interface[N]
#
#

##set minimium reserved memory to avoid swap due tofile system cache too much##
echo "1024" > /proc/sys/vm/min_free_kbytes

#optimize linux kernel parameter
echo "8192" > /proc/sys/net/core/netdev_max_backlog
echo "131072" > /proc/sys/net/core/optmem_max
echo "524288" >/proc/sys/net/core/rmem_default
echo "524288" >/proc/sys/net/core/rmem_max
echo "524288" >/proc/sys/net/core/wmem_default
echo "524288" > /proc/sys/net/core/wmem_max
echo "131072 262144 393216"> /proc/sys/net/ipv4/tcp_rmem
echo "131072 262144 393216"> /proc/sys/net/ipv4/tcp_wmem
echo "768 1024 1380" > /proc/sys/net/ipv4/tcp_mem

 
#config hotplug
echo /usr/hotplug > /proc/sys/kernel/hotplug
mkdir -p /tmp/usb/
DEVPATH=/sys/block/sda ACTION=add usbmount block
DEVPATH=/sys/block/sda/sda1 ACTION=add usbmount block

#auto mount partition listed in /proc/partitions
line=0
TMPPROCFILE=/proc/partitions
line=`cat $TMPPROCFILE | wc -l`
num=3
while [ $num -le $line ];
do
 pat0=` head -n $num $TMPPROCFILE | tail -n 1`
 major=`echo $pat0 | cut -f1 -d " "`
 name=`echo $pat0 | cut -f4 -d " "`
if [ $major = 8 ] ; then
  DEVPATH=/dev/$name ACTION=add usbmount block
fi
 num=`expr $num + 1`
done

#config samba
echo "start samba"
cp /etc/smb.conf /var/config/smb.conf
echo " " > /var/group
cp /etc/group /var/group
/bin/smbd -D

