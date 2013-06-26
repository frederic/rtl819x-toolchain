#!/bin/sh
#Start BT Clint now Ctorrent's Dctcs
TOOL=flash
GETMIB="$TOOL get"
CMD_PARA="-h 127.0.0.1:15000"

eval `$GETMIB BT_ENABLED`
eval `$GETMIB BT_UPLOAD_DIR`
eval `$GETMIB BT_DOWNLOAD_DIR`
eval `$GETMIB BT_TOTAL_ULIMIT`
eval `$GETMIB BT_TOTAL_DLIMIT`
eval `$GETMIB BT_REFRESH_TIME`

if [ "$BT_ENABLED" == "0" ]; then
#if ctorrent running kill all...
	killall ctorrent
	killall dctcs
	exit 0
fi
if [  -d $BT_UPLOAD_DIR ]; then
	CMD_PARA="$CMD_PARA -t $BT_UPLOAD_DIR"
else
	echo "BT seeds directory NOT set!"
	exit
fi

if [ -d $BT_DOWNLOAD_DIR ]; then
	CMD_PARA="$CMD_PARA -d $BT_DOWNLOAD_DIR"
else
	echo "BT download directory NOT set!"
	exit
fi

# 0 means no limits
if [ "$BT_TOTAL_ULIMIT" != "0" ]; then
	CMD_PARA="$CMD_PARA -u $BT_TOTAL_ULIMIT"
fi

if [ "$BT_TOTAL_DLIMIT" != "0" ]; then
	CMD_PARA="$CMD_PARA -D $BT_TOTAL_DLIMIT"
fi

# 0 means use default 15 miniutes.
if [ "$BT_REFRESH_TIME" != "0" ]; then
	CMD_PARA="$CMD_PARA -i $BT_REFRESH_TIME"
fi


# -r means restart the torrent when dcts started
CMD_PARA="$CMD_PARA -r"

dctcs $CMD_PARA &
