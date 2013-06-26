#!/bin/sh  
#                                 
# script file to start ntp client

TOOL=flash
GETMIB="$TOOL get"
NTPTMP=/tmp/ntp_tmp
LINKFILE=/etc/ppp/link
ETH1_IP=/var/eth1_ip
WAIT_TIME=0
success=0
eval `flash get WAN_DHCP`
eval `$GETMIB NTP_ENABLED`


killall date 2> /dev/null
killall ntpclient 2> /dev/null
###########kill sleep that ntp.sh created###############
TMPFILEDL=/tmp/tmpfiledl
line=0
ps | grep 'sleep 86400' > $TMPFILEDL
line=`cat $TMPFILEDL | wc -l`
num=1
while [ $num -le $line ];
do
	pat0=` head -n $num $TMPFILEDL | tail -n 1`
	pat1=`echo $pat0 | cut -f1 -dS`  
	pat2=`echo $pat1 | cut -f1 -d " "`  
	kill -9 $pat2 2> /dev/null
	num=`expr $num + 1`
done
rm -f /tmp/tmpfiledl 2> /dev/null

TMPFILEDL1=/tmp/tmpfiledl1
line1=0
ps | grep 'sleep 300' > $TMPFILEDL1
line1=`cat $TMPFILEDL1 | wc -l`
num1=1
while [ $num1 -le $line1 ];
do
	dl_pat0=` head -n $num1 $TMPFILEDL1 | tail -n 1`
	dl_pat1=`echo $dl_pat0 | cut -f1 -dS`  
	dl_pat2=`echo $dl_pat1 | cut -f1 -d " "`  
	kill -9 $dl_pat2 2> /dev/null
	num1=`expr $num1 + 1`
done
rm -f /tmp/tmpfiledl1 2> /dev/null
###########################

if [ $NTP_ENABLED = 1 ]; then
echo Start NTP daemon
	while [ true ];
	do
	eval `$GETMIB NTP_SERVER_IP1`
	eval `$GETMIB NTP_SERVER_IP2`
	eval `$GETMIB DAYLIGHT_SAVE`
	eval `$GETMIB NTP_TIMEZONE`
	eval `$GETMIB NTP_SERVER_ID`
	echo "run" > /var/ntp_run
	# rock: VOIP_SUPPORT is decided by mkimg
	###VOIP_SUPPORT###
	if [ "$VOIP_SUPPORT" != "" ] && [ -f "/bin/ash" ]; then
	TZ1=`echo $NTP_TIMEZONE | cut -d"\\\\" -f1`
	else
	TZ1=`echo $NTP_TIMEZONE | cut -d" " -f1`
	fi
	TZ2=`echo $NTP_TIMEZONE | cut -d" " -f2`
                DAYLIGHT=""
                if [ $DAYLIGHT_SAVE = 0 ];then
                	DAYLIGHT=""
                elif [ "$TZ1" = '9' ] && [ "$TZ2" = '1' ]; then
                        DAYLIGHT="PDT,M4.1.0/02:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '8' ] && [ "$TZ2" = '1' ]; then
                        DAYLIGHT="PDT,M4.1.0/02:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '7' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M4.1.0/02:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '6' ] && [ "$TZ2" = '1' ]; then
                        DAYLIGHT="PDT,M4.1.0/02:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '6' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M4.1.0/02:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '5' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M4.1.0/02:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '5' ] && [ "$TZ2" = '3' ]; then
                        DAYLIGHT="PDT,M4.1.0/02:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '4' ] && [ "$TZ2" = '3' ]; then
                        DAYLIGHT="PDT,M10.2.0/00:00:00,M3.2.0/00:00:00"
                elif [ "$TZ1" = '3' ] && [ "$TZ2" = '1' ]; then
                        DAYLIGHT="PDT,M4.1.0/00:00:00,M10.5.0/00:00:00"
                elif [ "$TZ1" = '3' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M2.2.0/00:00:00,M10.2.0/00:00:00"
                elif [ "$TZ1" = '1' ] && [ "$TZ2" = '1' ]; then
                        DAYLIGHT="PDT,M3.5.0/00:00:00,M10.5.0/01:00:00"
                elif [ "$TZ1" = '0' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M3.5.0/01:00:00,M10.5.0/02:00:00"
                elif [ "$TZ1" = '-1' ]; then
                        DAYLIGHT="PDT,M3.5.0/02:00:00,M10.5.0/03:00:00"
                elif [ "$TZ1" = '-2' ] && [ "$TZ2" = '1' ]; then
                        DAYLIGHT="PDT,M3.5.0/02:00:00,M10.5.0/03:00:00"
                elif [ "$TZ1" = '-2' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M3.5.0/03:00:00,M10.5.0/04:00:00"
                elif [ "$TZ1" = '-2' ] && [ "$TZ2" = '3' ]; then
                        DAYLIGHT="PDT,M4.5.5/00:00:00,M9.5.5/00:00:00"
                elif [ "$TZ1" = '-2' ] && [ "$TZ2" = '5' ]; then
                        DAYLIGHT="PDT,M3.5.0/03:00:00,M10.5.5/04:00:00"
                elif [ "$TZ1" = '-2' ] && [ "$TZ2" = '6' ]; then
                        DAYLIGHT="PDT,M3.5.5/02:00:00,M10.1.0/02:00:00"
                elif [ "$TZ1" = '-3' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M3.5.0/02:00:00,M10.5.0/03:00:00"
                elif [ "$TZ1" = '-4' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M3.5.0/04:00:00,M10.5.0/05:00:00"
                elif [ "$TZ1" = '-9' ] && [ "$TZ2" = '4' ]; then
                        DAYLIGHT="PDT,M10.5.0/02:00:00,M4.1.0/03:00:00"
                elif [ "$TZ1" = '-10' ] && [ "$TZ2" = '2' ]; then
                        DAYLIGHT="PDT,M10.5.0/02:00:00,M4.1.0/03:00:00"
                elif [ "$TZ1" = '-10' ] && [ "$TZ2" = '4' ]; then
                        DAYLIGHT="PDT,M10.1.0/02:00:00,M4.1.0/03:00:00"
                elif [ "$TZ1" = '-10' ] && [ "$TZ2" = '5' ]; then
                        DAYLIGHT="PDT,M3.5.0/02:00:00,M10.5.0/03:00:00"
                elif [ "$TZ1" = '-12' ] && [ "$TZ2" = '1' ]; then
                        DAYLIGHT="PDT,M3.2.0/03:00:00,M10.1.0/02:00:00"
                else
                        DAYLIGHT=""

                fi


                TZ_HALF="0"

                if [ "$TZ1" = '3' ] && [ "$TZ2" = '1' ]; then
                       TZ_HALF="-1"
                fi
                if [ "$TZ1" = '-3' ] && [ "$TZ2" = '4' ]; then
                       TZ_HALF="1"
                fi
                if [ "$TZ1" = '-4' ] && [ "$TZ2" = '3' ]; then
                       TZ_HALF="1"
                fi
                if [ "$TZ1" = '-5' ] && [ "$TZ2" = '3' ]; then
                       TZ_HALF="1"
                fi
                if [ "$TZ1" = '-9' ] && [ "$TZ2" = '4' ]; then
                       TZ_HALF="1"
                fi
                if [ "$TZ1" = '-9' ] && [ "$TZ2" = '5' ]; then
                       TZ_HALF="1"
                fi

                if [ "$TZ_HALF" = "1" ]; then
					COMMAND="GMT$TZ1:30$DAYLIGHT"                  
                else
					COMMAND="GMT$TZ1$DAYLIGHT"
                fi

		if [ $NTP_SERVER_ID = 0 ];then
			ntpserver=$NTP_SERVER_IP1
		else
			ntpserver=$NTP_SERVER_IP2
		fi
		
		
		if [ $success = 0 ]; then
			#ntpdate  $NTP_SERVER_IP 
	                echo "" > $NTPTMP
			ntpclient -s -h $ntpserver -i 5 > $NTPTMP
			if [ $? = 0 ]; then
				if [ -n "`cat $NTPTMP`" ];then
					echo ntp client success
					success=1
				else
					success=0
				fi
			else
				success=0
			fi
		
			if [ $success = 1 ] ;then
				echo $COMMAND > /etc/TZ
				if [ $DAYLIGHT_SAVE = 1 ]; then
		                       	echo "" > $NTPTMP
		                       	date > $NTPTMP
				fi
			else
				sleep 300
			fi
		fi
		
		if [ $success = 1 ] ;then
			if [ -n "`cat $NTPTMP`" ];then
				sleep 86400
				success=0
			fi
		fi
		
		
		
	done &
fi # NTP Enabled
