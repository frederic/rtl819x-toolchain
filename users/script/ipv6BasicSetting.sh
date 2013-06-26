#!/bin/sh
RM="rm -f"
KILL="kill -9"

#ENABLE
ADDR6_PARAM=`flash get IPV6_ADDR_PARAM |cut -f2 -d=`
ADDR6_ENABLED=`echo $ADDR6_PARAM |cut -f1 -d,`
ADDR6_ENABLED=`echo $ADDR6_ENABLED`

PREFIX1=`echo $ADDR6_PARAM | cut -f2 -d, `
PREFIX1=`echo $PREFIX1`
PREFIX2=`echo $ADDR6_PARAM | cut -f3 -d, `
PREFIX2=`echo $PREFIX2`
ADDR1=`echo $ADDR6_PARAM | cut -f4 -d, `
ADDR1=`echo $ADDR1`
ADDR2=`echo $ADDR6_PARAM | cut -f5 -d, `
ADDR2=`echo $ADDR2`

if [ "$ADDR6_ENABLED" = "1" ]; then
	/bin/ifconfig br0 $ADDR1/$PREFIX1
	/bin/ifconfig eth1 $ADDR2/$PREFIX2
fi
