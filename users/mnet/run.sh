#!/bin/sh

i=0

while [ "$i" -lt "10" ]
do
	usb_modeswitch -c /etc/usb_modeswitch.conf.1
	mdev -s
	pppd file /etc/wcdma.option &

	if [ -e "/var/run/ppp0.pid" ]; then
			i=10
	fi
	i=$(($i+1))
done
