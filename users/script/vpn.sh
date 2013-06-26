#!/bin/sh
CFG_PATH=/etc
IPSEC_CONF_FILE=$CFG_PATH/ipsec.conf
IPSEC_SECRET_FILE=$CFG_PATH/ipsec.secrets
FLASH_TOOLS=flash
PLUTO_PID=/var/run/pluto.pid
RSATMP=/tmp/rsaFile
#DEBUG=echo
SLEEP="sleep 5"
START_ALL=0

eval `$FLASH_TOOLS get WAN_DHCP`
if [ $WAN_DHCP = 0 -o $WAN_DHCP = 1 ]; then
	eval `flash get OP_MODE`
	eval `flash get WISP_WAN_ID`
	# if wireless ISP mode , set WAN to wlan0
	if [ "$OP_MODE" = '2' ];then 
		WAN="wlan$WISP_WAN_ID"
	else
        	WAN="eth1"
        fi
else
        WAN="ppp0"
fi

EXT_IP0=`ifconfig $WAN | grep -i "addr:"`
EXT_IP1=`echo $EXT_IP0 | cut -f2 -d:`
EXT_IP=`echo $EXT_IP1 | cut -f1 -d " "`

eval `$FLASH_TOOLS  get IPSECTUNNEL_TBL_NUM`
eval `$FLASH_TOOLS  get IPSECTUNNEL_ENABLED`

#$DEBUG ipsec setup stop
# default enable ipsec passthrough , and will be disalbed when vpn enabled
if [ -f /proc/nat_ipsec ];then
	echo "1" > /proc/nat_ipsec	
fi
# not support pptp wan interface and external ip is not null
if [ "$EXT_IP" = "" -o $WAN_DHCP = 4 ]; then
	exit
fi

#echo table number = $IPSECTUNNEL_TBL_NUM
#echo enable = $IPSECTUNNEL_ENABLED

# setup ipsec.conf and ipsec.secrets

if [ "$1" = "all" ] || [ "`cat $PLUTO_PID`" = "" ] ;then
	START_ALL=1
	if [ -f $PLUTO_PID ];then
		ipsec setup stop
	fi
fi

if [ $IPSECTUNNEL_ENABLED -gt 0 ] ;
then
	#output to common part to config file ipsec.conf
	echo "config setup" > $IPSEC_CONF_FILE
	#echo "  interfaces=\"ipsec0=$WAN\"" >> $IPSEC_CONF_FILE
	echo "  interfaces=%defaultroute" >> $IPSEC_CONF_FILE
	echo "  klipsdebug=none" >> $IPSEC_CONF_FILE
	echo "  plutodebug=none" >> $IPSEC_CONF_FILE
	echo "  plutoload=%search" >> $IPSEC_CONF_FILE
	echo "  plutostart=%search" >> $IPSEC_CONF_FILE
	echo "  uniqueids=yes" >> $IPSEC_CONF_FILE
	eval `$FLASH_TOOLS  get IPSEC_NATT_ENABLED`
	if [ $IPSEC_NATT_ENABLED = 1 ];then
		echo "  nat_traversal=yes" >> $IPSEC_CONF_FILE
	fi
	echo " " >> $IPSEC_CONF_FILE
	# disable ipsec passthrough
	if [ -f /proc/nat_ipsec ];then
		echo "0" > /proc/nat_ipsec
	fi
fi

echo "" > $IPSEC_SECRET_FILE

if [ $IPSECTUNNEL_TBL_NUM -gt 0 ] && [ $IPSECTUNNEL_ENABLED -gt 0 ];
then
  num=1
  while [ $num -le $IPSECTUNNEL_TBL_NUM ];
  do
    IPSECTUNNEL_TBL=`$FLASH_TOOLS get IPSECTUNNEL_TBL | grep IPSECTUNNEL_TBL$num`
#    entry=`echo $IPSECTUNNEL_TBL | cut -f2 -d=`
	if [ $num -lt 10 ]; then
		entry=`echo $IPSECTUNNEL_TBL | cut -b 18-`	
	else
		entry=`echo $IPSECTUNNEL_TBL | cut -b 19-`
	fi
    id=`echo $entry | cut -f1 -d,`
    enable=`echo $entry | cut -f2 -d,`
    enable=`echo $enable`
    #if [ $enable = 1 ];then #entry enable 
	conName=`echo $entry | cut -f3 -d,`
	conName=`echo $conName`
	#echo $conName
	lcType=`echo $entry | cut -f4 -d,`
	lctype=`echo $lcType`
	#echo $lcType
	lcIp=`echo $entry | cut -f5 -d,`
	lcIp=`echo $lcIp`
	#echo $lcIp
	lcMask=`echo $entry | cut -f6 -d,`
	lcMask=`echo $lcMask`
	#echo $lcMask
	rtType=`echo $entry | cut -f7 -d,`
	rtType=`echo $rtType`
	#echo $rtType
	rtIp=`echo $entry | cut -f8 -d,`
	rtIp=`echo $rtIp`
	#echo $rtIp
	rtMask=`echo $entry | cut -f9 -d,`
	rtMask=`echo $rtMask`
	#echo $rtMask
	rtGw=`echo $entry | cut -f10 -d,`
	rtGw=`echo $rtGw`
	#echo $rtGw
	authType=`echo $entry | cut -f25 -d,`
	authType=`echo $authType`
	#echo $authType
	lcId=`echo $entry | cut -f26 -d,`
	lcId=`echo $lcId`
	#echo $lcId
	rtId=`echo $entry | cut -f27 -d,`
	rtId=`echo $rtId`
	#echo $rtId
	lcIdType=`echo $entry | cut -f28 -d,`
	lcIdType=`echo $lcIdType`
	#echo $lcIdType
	rtIdType=`echo $entry | cut -f29 -d,`
	rtIdType=`echo $rtIdType`
	#echo $rtIdType
	rsaKey=`echo $entry | cut -f30 -d,`
	rsaKey=`echo $rsaKey`
	#echo $rsaKey
	if [ $rtIdType = 1 ] ;then #DNS type
		rtId=`echo @$rtId`
	fi    
	if [ $lcIdType = 1 ] ;then #DNS type
		lcId=`echo @$lcId`
	fi	
	##################################################
	echo "conn $conName" >>  $IPSEC_CONF_FILE
	# setup  local site
	#echo "	left=$EXT_IP" >> $IPSEC_CONF_FILE
	echo "	left=%defaultroute" >> $IPSEC_CONF_FILE
	if [ $lcType = 0 ]; then # single Address
		echo "	leftsubnet=$lcIp/32" >> $IPSEC_CONF_FILE
	else
		echo "	leftsubnet=$lcIp/$lcMask" >> $IPSEC_CONF_FILE
	fi # end local 
	if [ $lcIdType -gt 0 ] ;then  # RSA
		echo "	leftid=$lcId" >> $IPSEC_CONF_FILE
	fi
	if [ $authType  = 1 ] ;then  # RSA
		$FLASH_TOOLS rsa $RSATMP
		myRsaKey=`cat $RSATMP | grep pubkey | cut -b 10-`
		echo "	leftrsasigkey=$myRsaKey" >> $IPSEC_CONF_FILE
	fi

	# setup remote site
	if [ $rtType = 0 ]; then # single Address
		echo "	right=$rtGw" >> $IPSEC_CONF_FILE
		echo "	rightsubnet=$rtIp/32" >> $IPSEC_CONF_FILE
	elif [ $rtType = 1 ]; then # subnet Address
		echo "	right=$rtGw" >> $IPSEC_CONF_FILE
		echo "	rightsubnet=$rtIp/$rtMask" >> $IPSEC_CONF_FILE
	elif [ $rtType = 2 ]; then    #Any Address
		echo "	right=%any" >> $IPSEC_CONF_FILE
	else # NATT Any address
		echo "	right=%any" >> $IPSEC_CONF_FILE
		echo "	rightsubnetwithin=$rtIp/$rtMask" >> $IPSEC_CONF_FILE

	fi # end remote site 
	if [ $rtIdType -gt 0 ] ;then  # RSA
		echo "	rightid=$rtId" >> $IPSEC_CONF_FILE
	fi
	if [ $authType  = 1 ] ;then  # RSA
		echo "	rightrsasigkey=$rsaKey" >> $IPSEC_CONF_FILE
	fi

	# Encryption Algorithm
	espEncr=`echo $entry | cut -f13 -d,`
	espEncr=`echo $espEncr`
	#echo $espEncr
	if [ $espEncr = 0 ]; then # 3DES
		ESPENCR=3des
	elif [ $espEncr = 1 ]; then # AES
		ESPENCR=aes128
	else  # NULL
		ESPENCR=null
	fi
	#Authenticaiton Algorithm
	espAuth=`echo $entry | cut -f14 -d,`
	espAuth=`echo $espAuth`

	if [ $espAuth = 0 ]; then #  MD5
		ESPAUTH=md5
	else
		ESPAUTH=sha1
	fi
	echo "	auth=esp" >> $IPSEC_CONF_FILE
	echo "	esp=${ESPENCR}-${ESPAUTH}" >> $IPSEC_CONF_FILE
	
	keyMode=`echo $entry | cut -f11 -d,`
	keyMode=`echo $keyMode`
	#echo $keyMode
	if [ $keyMode = 0 ]; then # IKE Key Mode
		if [ $authType = 0 ] ;then
			echo "	authby=secret" >>  $IPSEC_CONF_FILE
		else
			echo "	authby=rsasig" >>  $IPSEC_CONF_FILE
		fi
		if [ $rtType = 2 ] || [  $rtType = 3 ]; then
			REMOTEIP="%any"
		else
			REMOTEIP=`echo $rtGw`
		fi

		psKey=`echo $entry | cut -f15 -d,`
		psKey=`echo $psKey`
		#echo $psKey
		if [ $authType  = 0 ] ;then  # PSK
			if [ $rtIdType -gt 0 ];then
				pskrtId=`echo $rtId`
			fi
			if [ $lcIdType -gt 0 ];then
				psklcId=`echo $lcId`
			fi
			echo "$EXT_IP $REMOTEIP $pskrtId $psklcId: PSK \"${psKey}\"" >> $IPSEC_SECRET_FILE
		fi
		ikeEncr=`echo $entry | cut -f16 -d,`
		ikeEncr=`echo $ikeEncr`
		if [ $ikeEncr = 0 ]; then # 3DES
			IKEENCR=3des
		elif [ $ikeEncr = 1 ]; then # AES
			IKEENCR=aes128
		else  # NULL
			IKEENCR=null
		fi

		ikeAuth=`echo $entry | cut -f17 -d,`
		ikeAuth=`echo $ikeAuth`
		if [ $ikeAuth = 0 ]; then #  MD5
			IKEAUTH=md5
		else
			IKEAUTH=sha
		fi

		ikeKeyGroup=`echo $entry | cut -f18 -d,`
		ikeKeyGroup=`echo $ikeKeyGroup`
		if [ $ikeKeyGroup = 0 ]; then #  DH1 768 
			IKEKEYGROUP=modp768
		elif [ $ikeKeyGroup = 1 ]; then #  DH2 1024 
			IKEKEYGROUP=modp1024
		else  #DH5 1536
			IKEKEYGROUP=modp1536
		fi

		echo "	ike=$IKEENCR-$IKEAUTH-$IKEKEYGROUP" >> $IPSEC_CONF_FILE

		ikeLifeTime=`echo $entry | cut -f19 -d,`
		ikeLifeTime=`echo $ikeLifeTime`
		echo "	ikelifetime=${ikeLifeTime}s" >> $IPSEC_CONF_FILE

		ipsecLifeTime=`echo $entry | cut -f20 -d,`
		ipsecLifeTime=`echo $ipsecLifeTime`
		echo "	lifetime=${ipsecLifeTime}s" >> $IPSEC_CONF_FILE

		ipsecPfs=`echo $entry | cut -f21 -d,`
		ipsecPfs=`echo $ipsecPfs`
		if [ $ipsecPfs = 1 ]; then #  DH2
			echo "	pfs=yes" >> $IPSEC_CONF_FILE
		else
			echo "	pfs=no" >> $IPSEC_CONF_FILE
		fi

	else # Manual Key  Mode
		spi=`echo $entry | cut -f22 -d,`
		spi=`echo $spi`
		#echo $spi
		encrKey=`echo $entry | cut -f23 -d,`
		encrKey=`echo $encrKey`
		#echo "$encrKey"
		authKey=`echo $entry | cut -f24 -d,`
		authKey=`echo $authKey`
		#echo "$authKey"

		echo "	spi=0x$spi" >> $IPSEC_CONF_FILE
		echo "	espenckey=0x$encrKey" >> $IPSEC_CONF_FILE
		echo "	espauthkey=0x$authKey" >> $IPSEC_CONF_FILE
	fi # end of Key mode
    #fi # end of entry enable
    num=`expr $num + 1`
  done
fi

if  [ $IPSECTUNNEL_ENABLED -gt 0 ] ;
then
	if [ -f $RSATMP ] ;then  # RSA
		cat $RSATMP >>  $IPSEC_SECRET_FILE
		rm $RSATMP
	fi
fi


if  [ $IPSECTUNNEL_ENABLED -gt 0 ] && [ $START_ALL -eq 1 ] ;
then
	$DEBUG ipsec setup start
	$SLEEP 
fi
#reload auto script
if  [ $IPSECTUNNEL_ENABLED -gt 0 ] ;
then
	ipsec auto --rereadsecrets
fi

# add ipsec connection and up 
if [ $IPSECTUNNEL_TBL_NUM -gt 0 ] && [ $IPSECTUNNEL_ENABLED -gt 0 ];
then
  num=1
  while [ $num -le $IPSECTUNNEL_TBL_NUM ];
  do
    IPSECTUNNEL_TBL=`$FLASH_TOOLS get IPSECTUNNEL_TBL | grep IPSECTUNNEL_TBL$num`
    entry=`echo $IPSECTUNNEL_TBL | cut -f2 -d=`
    enable=`echo $entry | cut -f2 -d,`
    enable=`echo $enable`
    conName=`echo $entry | cut -f3 -d,`
    conName=`echo $conName`
    #echo $conName
    if [ $START_ALL -ne 1 ]  && [ $num -ne $IPSECTUNNEL_TBL_NUM ] ;then
    	num=`expr $num + 1`
	continue # the newest modify entry will be last entry
    fi
    if [ $enable = 1 ];then #entry enable 
	keyMode=`echo $entry | cut -f11 -d,`
	keyMode=`echo $keyMode`
	#echo $keyMode
	if [ $keyMode = 0 ]; then # IKE Key Mode
		$DEBUG ipsec auto --add $conName
		conType=`echo $entry | cut -f12 -d,`
		conType=`echo $conType`
		#echo $conType
		if  [ $conType = 0 ]; then #initiator
			$DEBUG ipsec auto --up $conName &
		fi
	else
		echo "ipsec manual --up $conName"
		$DEBUG ipsec manual --up $conName
	fi  #keymode
    fi # enable
    num=`expr $num + 1`
  done 
fi #tbl number


# add rule for ipsec to access local LAN
if  [ $IPSECTUNNEL_ENABLED -gt 0 ] ;
then
	if [ "`iptables -L -v | grep ipsec`" != "" ]; then
		iptables -D FORWARD -i ipsec+ -j ACCEPT
		iptables -D FORWARD -o ipsec+ -j ACCEPT
	fi
	iptables -A FORWARD -i ipsec+ -j ACCEPT
	iptables -A FORWARD -o ipsec+ -j ACCEPT
fi
