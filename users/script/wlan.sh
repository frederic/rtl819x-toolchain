#!/bin/sh
#
# script file to start WLAN
#
if [ $# -lt 1 ]; then echo "Usage: $0 wlan_interface";  exit 1 ; fi

GETMIB="flash get $1"
SET_WLAN="iwpriv $1"
SET_WLAN_PARAM="$SET_WLAN set_mib"
IFCONFIG=ifconfig
START_WLAN_APP=wlanapp.sh
MAX_WDS_NUM=8

## Disable WLAN MAC driver and shutdown interface first ##
$IFCONFIG $1 down

eval `$GETMIB HW_RF_TYPE`
eval `$GETMIB WLAN_DISABLED`
eval `$GETMIB MODE`

VAP=`echo $1 | cut -b 7-`
if [ "$VAP" != "" ]; then
	# wlan0-va? interface
	$START_WLAN_APP kill $1
	if [ "$WLAN_DISABLED" = '1' ]; then
		exit 1
	fi

else	
	# shutdown all WDS interface
	num=0
	while [ $num -lt $MAX_WDS_NUM ]
	do
		$IFCONFIG $1-wds$num down
		num=`expr $num + 1`
	done

	## kill wlan application daemon ##
	$START_WLAN_APP kill $1
	if [ "$WLAN_DISABLED" = '1' -o "$HW_RF_TYPE" = '0' ]; then
		exit 1
	fi
fi

## Set parameters to driver ##
eval `$GETMIB HW_REG_DOMAIN`
$SET_WLAN_PARAM regdomain=$HW_REG_DOMAIN

eval `$GETMIB OP_MODE`
eval `$GETMIB WLAN_MAC_ADDR`
if [ "$WLAN_MAC_ADDR" = "000000000000" ]; then
	if [ "$VAP" = "" ]; then
		eval `$GETMIB HW_WLAN_ADDR`
		WLAN_MAC_ADDR=$HW_WLAN_ADDR				
	elif [ "$VAP" = "va0" ]; then
		eval `$GETMIB HW_WLAN_ADDR1`
		WLAN_MAC_ADDR=$HW_WLAN_ADDR1	
	elif [ "$VAP" = "va1" ]; then
		eval `$GETMIB HW_WLAN_ADDR2`
		WLAN_MAC_ADDR=$HW_WLAN_ADDR2	
	elif [ "$VAP" = "va2" ]; then
		eval `$GETMIB HW_WLAN_ADDR3`
		WLAN_MAC_ADDR=$HW_WLAN_ADDR3	
	else
		eval `$GETMIB HW_WLAN_ADDR4`
		WLAN_MAC_ADDR=$HW_WLAN_ADDR4	
	fi	
fi

# ifconfig all wlan interface when not in WISP
# ifconfig wlan1 later interface when in WISP mode, the wlan0  will be setup in WAN interface
if [ "$VAP" = "" ]; then
	eval `$GETMIB WISP_WAN_ID`
	if [ "$OP_MODE" != '2' ] || [ $1 != "wlan$WISP_WAN_ID" ] ;then
		$IFCONFIG $1 hw ether $WLAN_MAC_ADDR
	fi

	if [ "$OP_MODE" = '2' ]; then
		$SET_WLAN_PARAM disable_brsc=1
	fi

	eval `$GETMIB HW_LED_TYPE`
	$SET_WLAN_PARAM led_type=$HW_LED_TYPE
	
	## set AP/client/WDS mode ##	
	if [ "$MODE" = '1' ]; then
	## client mode
		eval `$GETMIB NETWORK_TYPE`
		if  [ "$NETWORK_TYPE" = '0' ]; then
			$SET_WLAN_PARAM opmode=8
		else
			$SET_WLAN_PARAM opmode=32
			eval `$GETMIB DEFAULT_SSID`
			$SET_WLAN_PARAM defssid="$DEFAULT_SSID"
		fi
	else
	## AP mode
		$SET_WLAN_PARAM opmode=16
	fi

	if [ "$MODE" = '2' ]; then
	## WDS only
		$SET_WLAN_PARAM wds_pure=1
	else
		$SET_WLAN_PARAM wds_pure=0
	fi

#add for mesh
	# 802.11s set mesh parameters ====================
	
	if [ "$MODE" = '4' ]; then
		$SET_WLAN_PARAM mesh_enable=1
		$SET_WLAN_PARAM mesh_ap_enable=1
		$SET_WLAN_PARAM mesh_portal_enable=1
	elif [ "$MODE" = '5' ]; then
		$SET_WLAN_PARAM mesh_enable=1	
		$SET_WLAN_PARAM mesh_ap_enable=0
		$SET_WLAN_PARAM mesh_portal_enable=1
	elif [ "$MODE" = '6' ]; then
		$SET_WLAN_PARAM mesh_enable=1
		$SET_WLAN_PARAM mesh_ap_enable=1
		$SET_WLAN_PARAM mesh_portal_enable=0
	elif [ "$MODE" = '7' ]; then
		$SET_WLAN_PARAM mesh_enable=1
		$SET_WLAN_PARAM mesh_ap_enable=0
		$SET_WLAN_PARAM mesh_portal_enable=0
	else
		$SET_WLAN_PARAM mesh_enable=0
		$SET_WLAN_PARAM mesh_ap_enable=0
		$SET_WLAN_PARAM mesh_portal_enable=0
	fi

if   [ "$MODE" = 4 ] || [ "$MODE" = 5 ] || [ "$MODE" = 6 ] || [ "$MODE" = 7 ] ; then
	
	eval `$GETMIB MESH_ROOT_ENABLE`
	if [ "$MESH_ROOT_ENABLE" = 0 ]; then
		$SET_WLAN_PARAM mesh_root_enable=0
	else
		$SET_WLAN_PARAM mesh_root_enable=1
	fi

	eval `$GETMIB MESH_ID`
	$SET_WLAN_PARAM mesh_id="$MESH_ID"

	eval `$GETMIB MESH_MAX_NEIGHTBOR`
	$SET_WLAN_PARAM mesh_max_neightbor=$MESH_MAX_NEIGHTBOR
	
	eval `$GETMIB MIB_MESH_WPA_AUTH`
	$SET_WLAN_PARAM mesh_privacy=$MIB_MESH_WPA_AUTH

fi
	# ====================
#add for mesh
	# set RF parameters
	$SET_WLAN_PARAM RFChipID=$HW_RF_TYPE

	eval `$GETMIB HW_TX_POWER_CCK`
	eval `$GETMIB HW_TX_POWER_OFDM`
	$SET_WLAN_PARAM TxPowerCCK=$HW_TX_POWER_CCK
	$SET_WLAN_PARAM TxPowerOFDM=$HW_TX_POWER_OFDM

	eval `$GETMIB HW_WLAN0_11N_LOFDMPWD`
	$SET_WLAN_PARAM LOFDM_pwrdiff=$HW_WLAN0_11N_LOFDMPWD

	eval `$GETMIB HW_WLAN0_11N_ANTPWD_C`
	$SET_WLAN_PARAM antC_pwrdiff=$HW_WLAN0_11N_ANTPWD_C

	eval `$GETMIB HW_WLAN0_11N_THER_RFIC`
	$SET_WLAN_PARAM ther_rfic=$HW_WLAN0_11N_THER_RFIC

	eval `$GETMIB HW_WLAN0_11N_XCAP`
	$SET_WLAN_PARAM crystalCap=$HW_WLAN0_11N_XCAP

	eval `$GETMIB BEACON_INTERVAL`
	$SET_WLAN_PARAM bcnint=$BEACON_INTERVAL

	eval `$GETMIB CHANNEL`
	$SET_WLAN_PARAM channel=$CHANNEL

else	
	# vap, set AP mode always	
	$SET_WLAN_PARAM opmode=16
	$IFCONFIG $1 hw ether $WLAN_MAC_ADDR		
fi

eval `$GETMIB SSID`
$SET_WLAN_PARAM ssid="$SSID"

eval `$GETMIB BASIC_RATES`
$SET_WLAN_PARAM basicrates=$BASIC_RATES

eval `$GETMIB SUPPORTED_RATES`
$SET_WLAN_PARAM oprates=$SUPPORTED_RATES

eval `$GETMIB RATE_ADAPTIVE_ENABLED`
if [ "$RATE_ADAPTIVE_ENABLED" = '0' ]; then
	$SET_WLAN_PARAM autorate=0
	eval `$GETMIB FIX_RATE`
	$SET_WLAN_PARAM fixrate=$FIX_RATE
else
	$SET_WLAN_PARAM autorate=1
fi

eval `$GETMIB RTS_THRESHOLD`
$SET_WLAN_PARAM rtsthres=$RTS_THRESHOLD

eval `$GETMIB FRAG_THRESHOLD`
$SET_WLAN_PARAM fragthres=$FRAG_THRESHOLD

eval `$GETMIB INACTIVITY_TIME`
$SET_WLAN_PARAM expired_time=$INACTIVITY_TIME

eval `$GETMIB PREAMBLE_TYPE`
$SET_WLAN_PARAM preamble=$PREAMBLE_TYPE

eval `$GETMIB HIDDEN_SSID`
$SET_WLAN_PARAM hiddenAP=$HIDDEN_SSID

eval `$GETMIB DTIM_PERIOD`
$SET_WLAN_PARAM dtimperiod=$DTIM_PERIOD

# Use the below only when using specific value
# instead of default setting
#$SET_WLAN_PARAM longretry=6
#$SET_WLAN_PARAM shortretry=6


$SET_WLAN_PARAM aclnum=0
eval `$GETMIB MACAC_ENABLED`
$SET_WLAN_PARAM aclmode=$MACAC_ENABLED
if [ "$MACAC_ENABLED" != '0' ]; then
	eval `$GETMIB MACAC_NUM`
	if [ "$MACAC_NUM" != 0 ]; then
		num=1
		while [ $num -le $MACAC_NUM ]
		do
			AC_TBL=`$GETMIB MACAC_ADDR | grep MACAC_ADDR$num`
			addr_comment=`echo $AC_TBL | cut -f2 -d=`
			addr=`echo $addr_comment | cut -f1 -d,`
			$SET_WLAN_PARAM acladdr=$addr
			num=`expr $num + 1`
		done
	fi
fi

eval `$GETMIB AUTH_TYPE`
eval `$GETMIB ENCRYPT`
if [ "$AUTH_TYPE" = '1' ] && [ "$ENCRYPT" != '1' ]; then
	# shared-key and not WEP enabled, force to open-system
	AUTH_TYPE=0
fi
$SET_WLAN_PARAM authtype=$AUTH_TYPE

if [ "$ENCRYPT" = '0' ]; then
	$SET_WLAN_PARAM encmode=0
elif [ "$ENCRYPT" = '1' ]; then
	### WEP mode ##
	eval `$GETMIB WEP`
	if [ "$WEP" = '1' ]; then
		eval `$GETMIB WEP64_KEY1`
		eval `$GETMIB WEP64_KEY2`
		eval `$GETMIB WEP64_KEY3`
		eval `$GETMIB WEP64_KEY4`
		eval `$GETMIB WEP_DEFAULT_KEY`
		$SET_WLAN_PARAM encmode=1
		$SET_WLAN_PARAM wepkey1=$WEP64_KEY1
		$SET_WLAN_PARAM wepkey2=$WEP64_KEY2
		$SET_WLAN_PARAM wepkey3=$WEP64_KEY3
		$SET_WLAN_PARAM wepkey4=$WEP64_KEY4
		$SET_WLAN_PARAM wepdkeyid=$WEP_DEFAULT_KEY
	else
		eval `$GETMIB WEP128_KEY1`
		eval `$GETMIB WEP128_KEY2`
		eval `$GETMIB WEP128_KEY3`
		eval `$GETMIB WEP128_KEY4`
		eval `$GETMIB WEP_DEFAULT_KEY`
		$SET_WLAN_PARAM encmode=5
		$SET_WLAN_PARAM wepkey1=$WEP128_KEY1
		$SET_WLAN_PARAM wepkey2=$WEP128_KEY2
		$SET_WLAN_PARAM wepkey3=$WEP128_KEY3
		$SET_WLAN_PARAM wepkey4=$WEP128_KEY4
		$SET_WLAN_PARAM wepdkeyid=$WEP_DEFAULT_KEY
	fi
else
        ## WPA mode ##
	$SET_WLAN_PARAM encmode=2
fi

## Set 802.1x flag ##
_ENABLE_1X=0
if [ $ENCRYPT -lt 2 ]; then
	eval `$GETMIB ENABLE_1X`
	eval `$GETMIB MAC_AUTH_ENABLED`
	if [ "$ENABLE_1X" != 0 ] || [ "$MAC_AUTH_ENABLED" != 0 ]; then
		_ENABLE_1X=1
	fi
else
	_ENABLE_1X=1
fi
$SET_WLAN_PARAM 802_1x=$_ENABLE_1X

## set WDS ##
eval `$GETMIB WDS_ENABLED`
eval `$GETMIB WDS_NUM`
$SET_WLAN_PARAM wds_num=0
if [ "$MODE" = 2 -o "$MODE" = 3 ] && [ "$WDS_ENABLED" != 0 ] && [ "$WDS_NUM" != 0 ]; then
	num=1
	while [ $num -le $WDS_NUM ]
	do
		WDS_TBL=`$GETMIB WDS | grep WDS$num`
		addr_comment=`echo $WDS_TBL | cut -f2 -d=`
		addr=`echo $addr_comment | cut -f1 -d,`
		txrate=`echo $addr_comment | cut -f2 -d,`
		$SET_WLAN_PARAM wds_add=$addr,$txrate
		num=`expr $num - 1`
		$IFCONFIG $1-wds$num hw ether $WLAN_MAC_ADDR
		num=`expr $num + 2`
	done
	$SET_WLAN_PARAM wds_enable=$WDS_ENABLED	
else
	$SET_WLAN_PARAM wds_enable=0
fi

if [ "$MODE" = 2 -o "$MODE" = 3 ] && [ "$WDS_ENABLED" != '0' ]; then
	eval `$GETMIB WDS_ENCRYPT`
	if [ "$WDS_ENCRYPT" = '0' ]; then
		$SET_WLAN_PARAM wds_encrypt=0
	elif [ "$WDS_ENCRYPT" = '1' ]; then
		eval `$GETMIB WDS_WEP_KEY`
		$SET_WLAN_PARAM wds_encrypt=1
		$SET_WLAN_PARAM wds_wepkey=$WDS_WEP_KEY
	elif [ "$WDS_ENCRYPT" = '2' ]; then
		eval `$GETMIB WDS_WEP_KEY`
		$SET_WLAN_PARAM wds_encrypt=5
		$SET_WLAN_PARAM wds_wepkey=$WDS_WEP_KEY	
	elif [ "$WDS_ENCRYPT" = '3' ]; then		
		$SET_WLAN_PARAM wds_encrypt=2			
	else	
		$SET_WLAN_PARAM wds_encrypt=4
	fi
fi

# enable/disable the notification for IAPP
eval `$GETMIB IAPP_DISABLED`
if [ "$IAPP_DISABLED" = 0 ]; then
	$SET_WLAN_PARAM iapp_enable=1
else
	$SET_WLAN_PARAM iapp_enable=0
fi

#set band
eval `$GETMIB BAND`
eval `$GETMIB WIFI_SPECIFIC`
if [ "$MODE" != '1' ] && [ "$WIFI_SPECIFIC" = 1 ] &&  [ "$BAND" = '2' ] ; then
	BAND=3
fi
if [ "$BAND" = '8' ]; then
	BAND=11	
	$SET_WLAN_PARAM deny_legacy=3
elif [ "$BAND" = '2' ]; then
	BAND=3
	$SET_WLAN_PARAM deny_legacy=1
elif [ "$BAND" = '10' ]; then
	BAND=11
	$SET_WLAN_PARAM deny_legacy=1
else
	$SET_WLAN_PARAM deny_legacy=0
fi
$SET_WLAN_PARAM band=$BAND

###Set 11n parameter
if [ $BAND = 10 ] || [ $BAND = 11 ]; then
eval `$GETMIB CHANNEL_BONDING`
$SET_WLAN_PARAM use40M=$CHANNEL_BONDING
eval `$GETMIB CONTROL_SIDEBAND`
if [ "$CHANNEL_BONDING" = 0 ]; then
$SET_WLAN_PARAM 2ndchoffset=0
else
if [ "$CONTROL_SIDEBAND" = 0 ]; then
	 $SET_WLAN_PARAM 2ndchoffset=1
fi
if [ "$CONTROL_SIDEBAND" = 1 ]; then
	 $SET_WLAN_PARAM 2ndchoffset=2
fi
fi
eval `$GETMIB SHORT_GI`
$SET_WLAN_PARAM shortGI20M=$SHORT_GI
$SET_WLAN_PARAM shortGI40M=$SHORT_GI

eval `$GETMIB AGGREGATION`
if [ "$AGGREGATION" = 0 ]; then
	$SET_WLAN_PARAM ampdu=$AGGREGATION
	$SET_WLAN_PARAM amsdu=$AGGREGATION
elif [ "$AGGREGATION" = 1 ]; then
	$SET_WLAN_PARAM ampdu=1
	$SET_WLAN_PARAM amsdu=0
elif [ "$AGGREGATION" = 2 ]; then
	$SET_WLAN_PARAM ampdu=0
	$SET_WLAN_PARAM amsdu=1
elif [ "$AGGREGATION" = 3 ]; then
	$SET_WLAN_PARAM ampdu=1
	$SET_WLAN_PARAM amsdu=1	
fi
fi
##########





#set nat2.5 disable when client and mac clone is set
eval `$GETMIB MACCLONE_ENABLED`
if [ "$MACCLONE_ENABLED" = '1' -a "$MODE" = '1' ]; then
	$SET_WLAN_PARAM nat25_disable=1
	$SET_WLAN_PARAM macclone_enable=1
else
	$SET_WLAN_PARAM nat25_disable=0
	$SET_WLAN_PARAM macclone_enable=0
fi
# set nat2.5 disable and macclone disable when wireless isp mode
if [ "$OP_MODE" = '2' ] ;then
	$SET_WLAN_PARAM nat25_disable=1
	$SET_WLAN_PARAM macclone_enable=0
fi

# set 11g protection mode
eval `$GETMIB PROTECTION_DISABLED`
if  [ "$PROTECTION_DISABLED" = '1' ] ;then
	$SET_WLAN_PARAM disable_protection=1
else
	$SET_WLAN_PARAM disable_protection=0
fi

# set block relay
eval `$GETMIB BLOCK_RELAY`
$SET_WLAN_PARAM block_relay=$BLOCK_RELAY

# set WiFi specific mode
eval `$GETMIB WIFI_SPECIFIC`
$SET_WLAN_PARAM wifi_specific=$WIFI_SPECIFIC

# for WMM
eval `$GETMIB WMM_ENABLED`
$SET_WLAN_PARAM qos_enable=$WMM_ENABLED

# for guest access
eval `$GETMIB ACCESS`
$SET_WLAN_PARAM guest_access=$ACCESS

#
# following settings is used when driver WPA module is included
#
eval `$GETMIB WPA_AUTH`
if [ $MODE != 1 ] && [ $ENCRYPT -ge 2 ] && [ $ENCRYPT -lt 7 ] && [ $WPA_AUTH = 2 ]; then
	if [ $ENCRYPT = 2 ]; then
		ENABLE=1	
	elif [ $ENCRYPT = 4 ]; then
		ENABLE=2	
	elif [ $ENCRYPT = 6 ]; then		
		ENABLE=3		
	else
		echo "invalid ENCRYPT value!"; exit
	fi	
	$SET_WLAN_PARAM psk_enable=$ENABLE
	
	if [ $ENCRYPT = 2 ] || [ $ENCRYPT = 6 ]; then
		eval `$GETMIB WPA_CIPHER_SUITE`
		if [ $WPA_CIPHER_SUITE = 1 ]; then
			CIPHER=2
		elif [ $WPA_CIPHER_SUITE = 2 ]; then
			CIPHER=8
		elif [ $WPA_CIPHER_SUITE = 3 ]; then		
			CIPHER=10
		else
			echo "invalid WPA_CIPHER_SUITE value!"; exit 1		
		fi	
	fi
	$SET_WLAN_PARAM wpa_cipher=$CIPHER
	
	if [ $ENCRYPT = 4 ] || [ $ENCRYPT = 6 ]; then
		eval `$GETMIB WPA2_CIPHER_SUITE`
		if [ $WPA2_CIPHER_SUITE = 1 ]; then
			CIPHER=2
		elif [ $WPA2_CIPHER_SUITE = 2 ]; then
			CIPHER=8
		elif [ $WPA2_CIPHER_SUITE = 3 ]; then		
			CIPHER=10
		else
			echo "invalid WPA2_CIPHER_SUITE value!"; exit 1		
		fi	
	fi
	$SET_WLAN_PARAM wpa2_cipher=$CIPHER	
	
	eval `$GETMIB WPA_PSK`
	$SET_WLAN_PARAM passphrase=$WPA_PSK		
	
	eval `$GETMIB WPA_GROUP_REKEY_TIME`
	$SET_WLAN_PARAM gk_rekey=$WPA_GROUP_REKEY_TIME			
else
	$SET_WLAN_PARAM psk_enable=0
fi
