#!/bin/sh
# script file for auto configuration

GET="flash get"
VOIP_GET="flash voip get"

CFG_MODE=`$VOIP_GET VOIP.AUTO_CFG_MODE | cut -d= -f2`
if [ "$CFG_MODE" = "0" ]; then
	# disable auto config
	exit 0
fi

FILE_PATH=`$VOIP_GET VOIP.AUTO_CFG_FILE_PATH | cut -d= -f2`
CFG_EXPIRE=`$VOIP_GET VOIP.AUTO_CFG_EXPIRE | cut -d= -f2`

if [ "$CFG_MODE" = "1" ]; then
	# HTTP Mode
	HTTP_ADDR=`$VOIP_GET VOIP.AUTO_CFG_HTTP_ADDR | cut -d= -f2`
	HTTP_PORT=`$VOIP_GET VOIP.AUTO_CFG_HTTP_PORT | cut -d= -f2`
elif [ "$CFG_MODE" = "2" ]; then
	TFTP_ADDR=`$VOIP_GET VOIP.AUTO_CFG_TFTP_ADDR | cut -d= -f2`
elif [ "$CFG_MODE" = "3" ]; then
	FTP_ADDR=`$VOIP_GET VOIP.AUTO_CFG_FTP_ADDR | cut -d= -f2`
	FTP_USER=`$VOIP_GET VOIP.AUTO_CFG_FTP_USER | cut -d= -f2`
	FTP_PASSWD=`$VOIP_GET VOIP.AUTO_CFG_FTP_PASSWD | cut -d= -f2`
fi

if [ "$CFG_MODE" = "1" ] || [ "$CFG_MODE" = "2" ] || [ "$CFG_MODE" = "3" ]; then
	TMPCFG="/tmp/voip_flash.dat"
	if [ "$1" = "ATA8972-8m" ]; then
		FILENAME=`$GET HW_NIC1_ADDR | cut -d= -f2`".dat"
	else
		FILENAME=`$GET HW_NIC0_ADDR | cut -d= -f2`".dat"
	fi
	if [ -z "$FILE_PATH" ]; then
		TARGET_PATH=$FILENAME
	else
		TARGET_PATH=$FILE_PATH/$FILENAME
	fi
	echo "Start Auto Config daemon"
	# start auto config
	while [ true ]; do
		if [ "$CFG_MODE" = "1" ]; then
			HTTP_URL="http://$HTTP_ADDR:$HTTP_PORT/$FILE_PATH/$FILENAME"
		# wget
			echo +++++auto configuration by http+++++
		wget $HTTP_URL -O $TMPCFG
		elif [ "$CFG_MODE" = "2" ]; then
			#tftp
			echo +++++auto configuration by tftp+++++
			echo $TARGET_PATH
			tftp -g -l $TMPCFG -r $TARGET_PATH  $TFTP_ADDR
		elif [ "$CFG_MODE" = "3" ]; then
			#ftpget
			echo $TARGET_PATH
			echo +++++auto configuration by ftpget+++++
			ftpget -v -u$FTP_USER -p$FTP_PASSWD $FTP_ADDR $TMPCFG $TARGET_PATH
		fi
		if [ $? != 0 ]; then
			if [ "$CFG_MODE" = "1" ]; then
			echo "=> auto config error: wget $HTTP_URL failed"
			elif [ "$CFG_MODE" = "2" ]; then
				echo "=> auto config error: tftp $TFTP_ADDR failed"
			elif [ "$CFG_MODE" = "3" ]; then
				echo "=> auto config error: tftp $FTP_ADDR failed"
			fi
			sleep 10
			continue
		fi
		# import
		flash config-file $TMPCFG
		case $? in
			0)  # import ok
				SLEEP_TIME=`expr $CFG_EXPIRE \* 3600 \* 24`
				RESTART=1
				;;
			1)	# the same version
				SLEEP_TIME=`expr $CFG_EXPIRE \* 3600 \* 24`
				;;
			*)	# import failed:
				echo "=> auto config error: import $TMPCFG error"
				SLEEP_TIME=300
				;;
		esac

		if [ "$RESTART" = "1" ]; then
			if [ "$1" = "ATA8972-8m" ]; then
				init.sh ATA867x_8m all
			else
				init.sh gw all
			fi
			# import ok, restart webs & solar
			killall webs
			webs &
			solar &
		fi

		echo "=> auto config info: completed"

		CFG_EXPIRE=`$VOIP_GET VOIP.AUTO_CFG_EXPIRE | cut -d= -f2`
		if [ "$CFG_EXPIRE" = "0" ]; then
			# run once only
			break
		fi

		sleep $SLEEP_TIME
	done
fi

echo "=> auto config info: exit"
