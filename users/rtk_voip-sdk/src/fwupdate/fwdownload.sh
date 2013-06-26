#!/bin/sh
# script file for fw download 
GET="flash get"
VOIP_GET="flash voip get"
TARGET=$1
FILENAME=$2

#echo $TARGET
#echo $FILENAME

CFG_MODE=`$VOIP_GET VOIP.FW_UPDATE_MODE | cut -d= -f2`

if [ "$CFG_MODE" = "0" ]; then
	exit 0
#TFTP Mode
elif [ "$CFG_MODE" = "1" ]; then
	TFTP_ADDR=`$VOIP_GET VOIP.FW_UPDATE_TFTP_ADDR | cut -d= -f2`
	FILE_PATH=`$VOIP_GET VOIP.FW_UPDATE_FILE_PATH | cut -d= -f2`
	
	#echo "Start tftp download "

	if [ "$FILE_PATH" = "/" ]; then
		tftp -g -r $TARGET -l $FILENAME $TFTP_ADDR
	elif [ "$FILE_PATH" = "" ]; then
		tftp -g -r $TARGET -l $FILENAME $TFTP_ADDR
	else
		tftp -g -r $FILE_PATH/$TARGET -l $FILENAME $TFTP_ADDR
	fi
	
	if [ $? != 0 ]; then
		#echo "TFTP download $TARGET fail"
		rm $FILENAME
	else
		echo #"success TFTP download"
	fi

#FTP Mode
elif [ "$CFG_MODE" = "2" ]; then
	
	FTP_ADDR=`$VOIP_GET VOIP.FW_UPDATE_FTP_ADDR | cut -d= -f2`
	FILE_PATH=`$VOIP_GET VOIP.FW_UPDATE_FILE_PATH | cut -d= -f2`
	FTP_USER=`$VOIP_GET VOIP.FW_UPDATE_FTP_USER | cut -d= -f2`
	FILE_PASSWD=`$VOIP_GET VOIP.FW_UPDATE_FTP_PASSWD | cut -d= -f2`
	

	if [ "$FILE_PATH" = "/" ]; then
		ftpget -u$FTP_USER -p$FILE_PASSWD $FTP_ADDR $FILENAME $TARGET
	elif [ "$FILE_PATH" = "" ]; then
		ftpget -u$FTP_USER -p$FILE_PASSWD $FTP_ADDR $FILENAME $TARGET
	else
		ftpget -u$FTP_USER -p$FILE_PASSWD $FTP_ADDR $FILENAME $FILE_PATH/$TARGET
	fi
	
	if [ $? != 0 ]; then
		echo "FTP download $TARGET fail"
	else
		echo #"success FTP download"
	fi

# HTTP Mode
elif [ "$CFG_MODE" = "3" ]; then
		
	HTTP_ADDR=`$VOIP_GET VOIP.FW_UPDATE_HTTP_ADDR | cut -d= -f2`
	HTTP_PORT=`$VOIP_GET VOIP.FW_UPDATE_HTTP_PORT | cut -d= -f2`
	FILE_PATH=`$VOIP_GET VOIP.FW_UPDATE_FILE_PATH | cut -d= -f2`
	
	HTTP_URL="http://$HTTP_ADDR:$HTTP_PORT/$FILE_PATH/$TARGET"
	
	#echo "Start http download "

	wget $HTTP_URL -O $FILENAME
	
	#echo "wget $HTTP_URL -O $FILENAME"
	
	#echo "http download end"
fi
