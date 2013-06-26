#!/bin/sh

#######################################################################
## Add definitions into voip_flash.h 
##

unused_func()
{

W32_DEFINE_GEN="./w32_define"
W32_DEFINE_TARGET="./WIN32/voip_flash.h"
W32_DEFINE_TARGET_BAK="./WIN32/voip_flash.h.bak"
W32_DEFINE_TARGET_TMP="./tmp.h"

W32_DEFINE_01="SLIC_NUM"
W32_DEFINE_02="CON_CH_NUM"

#####
if [ ! -f $W32_DEFINE_TARGET_BAK ]; then
	echo "Backup $W32_DEFINE_TARGET to $W32_DEFINE_TARGET_BAK"
	cp $W32_DEFINE_TARGET $W32_DEFINE_TARGET_BAK
fi

#####
if [ ! -f $W32_DEFINE_APP ]; then
	echo "'$W32_DEFINE_APP' is not found!"
	echo "Please build $W32_DEFINE_APP first."
	exit 1
fi

voip_def_01=`$W32_DEFINE_GEN $W32_DEFINE_01`
voip_def_02=`$W32_DEFINE_GEN $W32_DEFINE_02`

#echo $voip_def_01
#echo $voip_def_02

sed -e "s/#define $W32_DEFINE_01.*/${voip_def_01}/" $W32_DEFINE_TARGET |	\
sed -e "s/#define $W32_DEFINE_02.*/${voip_def_02}/" 						\
> $W32_DEFINE_TARGET_TMP

mv -f $W32_DEFINE_TARGET_TMP $W32_DEFINE_TARGET

}

#######################################################################
## Add compiler flags to VC6 
## 
#FLASH_GW_FLAGS_FILE="../../goahead-2.1.1/LINUX/flash_gw.o.flags"
FLASH_GW_FLAGS_FILE="voip_flash_gw.o.flags"
CVVOIP_PATH="./WIN32"
CVVOIP_DSP_TEMPLATE="$CVVOIP_PATH/CVVoIP-template.dsp"
CVVOIP_DSP_TARGET="$CVVOIP_PATH/CVVoIP.dsp"
CVVOIP_DSP_TARGET_BAK="$CVVOIP_PATH/CVVoIP.dsp.bak"

SYM1="__REPLACE_BY_MAKE_W32_SYM1__"

#####
if [ ! -f $CVVOIP_DSP_TARGET_BAK ]; then
	echo "Backup $CVVOIP_DSP_TARGET to $CVVOIP_DSP_TARGET_BAK"
	cp $CVVOIP_DSP_TARGET $CVVOIP_DSP_TARGET_BAK
fi

#####
if [ ! -f $FLASH_GW_FLAGS_FILE ]; then
	echo "'$FLASH_GW_FLAGS_FILE' is not found!"
	echo "Please build flash-gw first."
	exit 1
fi

#####
if [ ! -f $CVVOIP_DSP_TEMPLATE ]; then
	echo "'$CVVOIP_DSP_TEMPLATE' is not found!"
	echo "Please update new SDK."
	exit 1
fi


echo "Start to analyze $FLASH_GW_FLAGS_FILE"

LIST=`cat $FLASH_GW_FLAGS_FILE`

target_flags=""

#echo $LIST

for flag in $LIST; do
	flag=`echo $flag | grep "^-D"`
	#echo "-- $flag"
	if [ "$flag" != "" ]; then
		flag=`echo $flag | sed -e 's/^-D//g'`
		flag_name=`echo $flag | sed -e 's/=.*//g'`
		#echo "- $flag_name"
		grep -w "$flag_name" $CVVOIP_PATH/* | grep -v $FLASH_GW_FLAGS_FILE > /dev/null
		if [ $? == 0 ]; then
			target_flags="$target_flags \/D \\\"$flag\\\""
			#echo $flag
		fi
	fi
done

#echo $target_flags

sed -e "s/${SYM1}/${target_flags}/g" $CVVOIP_DSP_TEMPLATE > $CVVOIP_DSP_TARGET

echo "Output to $CVVOIP_DSP_TARGET... done"
## 
## Add compiler flags to VC6 
#######################################################################



