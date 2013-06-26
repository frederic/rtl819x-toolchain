#!/bin/sh
RM="rm -f"
KILL="kill -9"
RADVD_PID_FILE=/var/run/radvd.pid
RADVD_CONF_FILE=/var/radvd.conf
RADVD_PARAM=`flash get IPV6_RADVD_PARAM |cut -f2 -d=`
RADVD_ENABLED=`echo $RADVD_PARAM |cut -f1 -d,`
RADVD_ENABLED=`echo $RADVD_ENABLED`
if [ ! -f $RADVD_CONF_FILE ]; then
#interface
	Name=`echo $RADVD_PARAM |cut -f2 -d,`
	MaxRtrAdvInterval=`echo $RADVD_PARAM |cut -f3 -d,`
        MinRtrAdvInterval=`echo $RADVD_PARAM |cut -f4 -d,`
        MinDelayBetweenRAs=`echo $RADVD_PARAM |cut -f5 -d,`
        AdvManagedFlag=`echo $RADVD_PARAM |cut -f6 -d,`
        AdvOtherConfigFlag=`echo $RADVD_PARAM |cut -f7 -d,`
        AdvLinkMTU=`echo $RADVD_PARAM |cut -f8 -d,`
        AdvReachableTime=`echo $RADVD_PARAM |cut -f9 -d,`
        AdvRetransTimer=`echo $RADVD_PARAM |cut -f10 -d,`
        AdvCurHopLimit=`echo $RADVD_PARAM |cut -f11 -d,`
        AdvDefaultLifetime=`echo $RADVD_PARAM |cut -f12 -d,`
        AdvDefaultPreference=`echo $RADVD_PARAM |cut -f13 -d,`
        AdvSourceLLAddress=`echo $RADVD_PARAM |cut -f14 -d,`
        UnicastOnly=`echo $RADVD_PARAM |cut -f15 -d,`
#get rid of blank
	Name=`echo $Name`
	MaxRtrAdvInterval=`echo $MaxRtrAdvInterval`
        MinRtrAdvInterval=`echo $MinRtrAdvInterval`
        MinDelayBetweenRAs=`echo $MinDelayBetweenRAs`
        AdvManagedFlag=`echo $AdvManagedFlag`
        AdvOtherConfigFlag=`echo $AdvOtherConfigFlag`
        AdvLinkMTU=`echo $AdvLinkMTU`
        AdvReachableTime=`echo $AdvReachableTime`
        AdvRetransTimer=`echo $AdvRetransTimer`
        AdvCurHopLimit=`echo $AdvCurHopLimit`
        AdvDefaultLifetime=`echo $AdvDefaultLifetime`
        AdvDefaultPreference=`echo $AdvDefaultPreference`
        AdvSourceLLAddress=`echo $AdvSourceLLAddress`
        UnicastOnly=`echo $UnicastOnly`
#prefix1
        Prefix=`echo $RADVD_PARAM |cut -f16 -d,`
        PrefixLen=`echo $RADVD_PARAM |cut -f17 -d,`
        AdvOnLinkFlag=`echo $RADVD_PARAM |cut -f18 -d,`
        AdvAutonomousFlag=`echo $RADVD_PARAM |cut -f19 -d,`
        AdvValidLifetime=`echo $RADVD_PARAM |cut -f20 -d,`
        AdvPreferredLifetime=`echo $RADVD_PARAM |cut -f21 -d,`
        AdvRouterAddr=`echo $RADVD_PARAM |cut -f22 -d,`
        if6to4=`echo $RADVD_PARAM |cut -f23 -d,`
        enabled=`echo $RADVD_PARAM |cut -f24 -d,`
        
	Prefix=`echo $Prefix`
        PrefixLen=`echo $PrefixLen`
        AdvOnLinkFlag=`echo $AdvOnLinkFlag`
        AdvAutonomousFlag=`echo $AdvAutonomousFlag`
        AdvValidLifetime=`echo $AdvValidLifetime`
        AdvPreferredLifetime=`echo $AdvPreferredLifetime`
        AdvRouterAddr=`echo $AdvRouterAddr`
        if6to4=`echo $if6to4`
        enabled=`echo $enabled`
#prefix 2
        Prefix_1=`echo $RADVD_PARAM |cut -f25 -d,`
        PrefixLen_1=`echo $RADVD_PARAM |cut -f26 -d,`
        AdvOnLinkFlag_1=`echo $RADVD_PARAM |cut -f27 -d,`
        AdvAutonomousFlag_1=`echo $RADVD_PARAM |cut -f28 -d,`
        AdvValidLifetime_1=`echo $RADVD_PARAM |cut -f29 -d,`
        AdvPreferredLifetime_1=`echo $RADVD_PARAM |cut -f30 -d,`
        AdvRouterAddr_1=`echo $RADVD_PARAM |cut -f31 -d,`
        if6to4_1=`echo $RADVD_PARAM |cut -f32 -d,`
        enabled_1=`echo $RADVD_PARAM |cut -f33 -d,`
        
	Prefix_1=`echo $Prefix_1`
        PrefixLen_1=`echo $PrefixLen_1`
        AdvOnLinkFlag_1=`echo $AdvOnLinkFlag_1`
        AdvAutonomousFlag_1=`echo $AdvAutonomousFlag_1`
        AdvValidLifetime_1=`echo $AdvValidLifetime_1`
        AdvPreferredLifetime_1=`echo $AdvPreferredLifetime_1`
        AdvRouterAddr_1=`echo $AdvRouterAddr_1`
        if6to4_1=`echo $if6to4_1`
        enabled_1=`echo $enabled_1`
#create file
	echo "interface $Name" >> $RADVD_CONF_FILE
	echo "{" >> $RADVD_CONF_FILE
	echo "AdvSendAdvert on;" >> $RADVD_CONF_FILE
	echo "MaxRtrAdvInterval $MaxRtrAdvInterval;" >> $RADVD_CONF_FILE
	echo "MinRtrAdvInterval $MinRtrAdvInterval;" >> $RADVD_CONF_FILE
	echo "MinDelayBetweenRAs $MinDelayBetweenRAs;" >> $RADVD_CONF_FILE
	if [ $AdvManagedFlag -gt 0 ]; then 
		echo "AdvManagedFlag on;" >> $RADVD_CONF_FILE
	fi
	if [ $AdvOtherConfigFlag -gt 0 ]; then
		echo "AdvOtherConfigFlag on;" >> $RADVD_CONF_FILE
	fi
	echo "AdvLinkMTU $AdvLinkMTU;" >> $RADVD_CONF_FILE
	echo "AdvReachableTime $AdvReachableTime;" >> $RADVD_CONF_FILE
	echo "AdvRetransTimer $AdvRetransTimer;" >> $RADVD_CONF_FILE
	echo "AdvCurHopLimit $AdvCurHopLimit;" >> $RADVD_CONF_FILE
	echo "AdvDefaultLifetime $AdvDefaultLifetime;" >> $RADVD_CONF_FILE
	echo "AdvDefaultPreference $AdvDefaultPreference;" >> $RADVD_CONF_FILE
	if [ $AdvSourceLLAddress -gt 0 ]; then
		echo "AdvSourceLLAddress on;" >> $RADVD_CONF_FILE
	fi
	if [ $UnicastOnly -gt 0 ]; then
		echo "UnicastOnly on;" >> $RADVD_CONF_FILE
	fi
#prefix 1
	if [ $enabled -gt 0 ]; then
	echo "prefix $Prefix/$PrefixLen" >> $RADVD_CONF_FILE
	echo "{" >> $RADVD_CONF_FILE
	if [ $AdvOnLinkFlag -gt 0 ]; then
		echo "AdvOnLink on;" >> $RADVD_CONF_FILE
	fi
	if [ $AdvAutonomousFlag -gt 0 ]; then
		echo "AdvAutonomous on;" >> $RADVD_CONF_FILE
	fi
	echo "AdvValidLifetime $AdvValidLifetime;" >> $RADVD_CONF_FILE
	echo "AdvPreferredLifetime $AdvPreferredLifetime;" >> $RADVD_CONF_FILE
	if [ $AdvRouterAddr -gt 0 ]; then
		echo "AdvRouterAddr on;" >> $RADVD_CONF_FILE
	fi
	
	if [ "$if6to4" != "" ]; then
		echo "Base6to4Interface $if6to4;" >> $RADVD_CONF_FILE
	fi
	echo "};" >> $RADVD_CONF_FILE
	fi
#prefix 2
	if [ $enabled_1 -gt 0 ]; then
	echo "prefix $Prefix_1/$PrefixLen_1" >> $RADVD_CONF_FILE
	echo "{" >> $RADVD_CONF_FILE
	if [ $AdvOnLinkFlag_1 -gt 0 ]; then
		echo "AdvOnLink on;" >> $RADVD_CONF_FILE
	fi
	if [ $AdvAutonomousFlag_1 -gt 0 ]; then
		echo "AdvAutonomous on;" >> $RADVD_CONF_FILE
	fi
	echo "AdvValidLifetime $AdvValidLifetime_1;" >> $RADVD_CONF_FILE
	echo "AdvPreferredLifetime $AdvPreferredLifetime_1;" >> $RADVD_CONF_FILE
	if [ $AdvRouterAddr_1 -gt 0 ]; then
		echo "AdvRouterAddr on;" >> $RADVD_CONF_FILE
	fi
	if [ "$if6to4_1" != "" ]; then
		echo "Base6to4Interface $if6to4_1;" >> $RADVD_CONF_FILE
	fi
	echo "};" >> $RADVD_CONF_FILE
	fi
#end
	echo "};"  >> $RADVD_CONF_FILE
fi

if [ ! -f $RADVD_PID_FILE ]; then
	if [ "$RADVD_ENABLED" = "1" ]; then
		echo "1" > /proc/sys/net/ipv6/conf/all/forwarding
		/bin/radvd -C /var/radvd.conf
	fi
else
	PID=`cat $RADVD_PID_FILE`
	if [ "$RADVD_ENABLED" = "1" ]; then
		$KILL $PID
		$RM $RADVD_PID_FILE
		echo "1" > /proc/sys/net/ipv6/conf/all/forwarding
		/bin/radvd -C /var/radvd.conf
	else
		#kill the radvd
		$KILL $PID
	fi
fi
