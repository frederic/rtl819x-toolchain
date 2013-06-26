#!/bin/sh
#
# script file to start dhcp client (udhcpc)
#



if [ $# -lt 2 ]; then echo "Usage: $0 interface {wait|no]";  exit 1 ; fi

PARAM1=$1
PARAM2=$2

GETMIB="flash get"
SCRIPTFILE_PATH=/usr/share/udhcpc
PIDFILE=/etc/udhcpc/udhcpc-$PARAM1.pid
CMD="-i $PARAM1 -p $PIDFILE -s $SCRIPTFILE_PATH/$PARAM1.sh"


rtl_set_dhcpc_no() {
# marked Auto-AP
#	CMD="$CMD -n -a"
	CMD="$CMD -n"
	eval `$GETMIB IP_ADDR`
	eval `$GETMIB SUBNET_MASK`
	eval `$GETMIB DEFAULT_GATEWAY`

	# Generate deconfig script, used when DHCP request is failed

	echo "#!/bin/sh" > $SCRIPTFILE_PATH/$PARAM1.deconfig
	echo "ifconfig $PARAM1 $IP_ADDR netmask $SUBNET_MASK" >> $SCRIPTFILE_PATH/$PARAM1.deconfig
        echo "while route del default dev $PARAM1" >> $SCRIPTFILE_PATH/$PARAM1.deconfig
        echo "do :" >> $SCRIPTFILE_PATH/$PARAM1.deconfig
        echo "done" >> $SCRIPTFILE_PATH/$PARAM1.deconfig
	echo "route add -net default gw $DEFAULT_GATEWAY dev $PARAM1" >> $SCRIPTFILE_PATH/$PARAM1.deconfig

	# added to start wlan application daemon
	echo "init.sh ap wlan_app" >> $SCRIPTFILE_PATH/$PARAM1.deconfig
}

rtl_set_dhcpc_wait() {
	CMD="$CMD -a 30"
	# Generate deconfig script
	echo "#!/bin/sh" > $SCRIPTFILE_PATH/$PARAM1.deconfig
	echo "ifconfig $PARAM1 0.0.0.0" >> $SCRIPTFILE_PATH/$PARAM1.deconfig

	# for miniigd patch	
	#echo "killall -9 upnp.sh 2> /dev/null" >> $SCRIPTFILE_PATH/$PARAM1.deconfig
	#echo "upnp.sh" >> $SCRIPTFILE_PATH/$PARAM1.deconfig
	
	eval `$GETMIB HOST_NAME`
	if [ "$HOST_NAME" != ""  ]; then	
		CMD="$CMD -h $HOST_NAME"
	fi
}


rtl_del_udhcpc_pid(){
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		if [ $PID != 0 ]; then
			kill -9 $PID
		fi
		rm -f $PIDFILE
	fi
}

rtl_dhcpc() {
	if [ $PARAM2 = 'no' ]; then
		rtl_set_dhcpc_no
	else
		rtl_set_dhcpc_wait
	fi
	
	rtl_del_udhcpc_pid

	#start dhcpc daemon
	udhcpc $CMD
}

rtl_dhcpc

