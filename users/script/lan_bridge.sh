#!/bin/sh

GETMIB="flash get"


rtl_start_br_ntp() {
	echo 'RTL STP configuration'
	
	brctl addbr stp_br

	eval `$GETMIB HW_NIC0_ADDR`
	eval `$GETMIB OP_MODE`

	ifconfig port0 hw ether $HW_NIC0_ADDR  
	ifconfig port1 hw ether $HW_NIC0_ADDR
	ifconfig port2 hw ether $HW_NIC0_ADDR
	ifconfig port3 hw ether $HW_NIC0_ADDR
	ifconfig port4 hw ether $HW_NIC0_ADDR 
	ifconfig port5 hw ether $HW_NIC0_ADDR
	#ifconfig stp_br hw ether $HW_NIC0_ADDR 


	#ifconfig br1 192.168.1.254 netmask 255.255.255.0 
	ifconfig port0 up
	ifconfig port1 up
	ifconfig port2 up
	ifconfig port3 up
	ifconfig port4 up
	ifconfig port5 up

	brctl addif stp_br port0
	brctl addif stp_br port1
	brctl addif stp_br port2
	brctl addif stp_br port3
	brctl addif stp_br port4
	brctl addif stp_br port5

	ifconfig stp_br up
}

rtl_start_br_nontp() {
	echo 'None RTL STP support'
        ifconfig port0 down
	ifconfig stp_br down
        ifconfig port1 down
        ifconfig port2 down
        ifconfig port3 down
        ifconfig port4 down
        ifconfig port5 down

        brctl delif stp_br port0
	brctl delif stp_br port1
	brctl delif stp_br port2
	brctl delif stp_br port3
	brctl delif stp_br port4
	brctl delif stp_br port5
	brctl delbr stp_br
}

rtl_lan_bridge() {
	eval `$GETMIB RTL_STP_ENABLED`
	if [ "$RTL_STP_ENABLED" = '1' ]; then
		rtl_start_br_ntp
	else
		rtl_start_br_nontp
	fi
	
}

rtl_lan_bridge



