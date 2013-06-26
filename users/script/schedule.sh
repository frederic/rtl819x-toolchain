#!/bin/sh
#add by cary
eval `flash get URLFILTER_ENABLED`
eval `flash get URLFILTER_TBL_NUM`
eval `flash get OP_MODE`
# do not edit next line, it will be replaced to "CONFIG_RTL_HW_NAPT=y" or "CONFIG_RTL_HW_NAPT=" by mkimg
###CONFIG_RTL_HW_NAPT###

rtl_enable_urlfilter() {
  urlnum=0
  num=1
  while [ $num -le $URLFILTER_TBL_NUM ];
  do
    str=`flash get URLFILTER_TBL | grep URLFILTER_TBL$num`
    str=`echo $str | cut -f2 -d=`
    url="$str $url"
    num=`expr $num + 1`
  done
  #url="$urlnum$url"
  #echo "$url "> /proc/url_filter
  echo "add:0#3 3 $url" > /proc/filter_table
 # echo "add:100/3$url"> /proc/filter_table
  
 ## Gateway mode, exclude WISP mode
  if [ "$CONFIG_RTL_HW_NAPT" = "y" ]; then
    if [ "$OP_MODE" = '0' ];then
      echo "0" > /proc/hw_nat
    fi
  fi
}

rtl_disable_urlfilter() {
  #echo "0 "> /proc/url_filter
  echo "flush"> /proc/filter_table
  ## Gateway mode, exclude WISP mode
  if [ "$CONFIG_RTL_HW_NAPT" = "y" ]; then
    if [ "$OP_MODE" = '0' ];then
      echo "1" > /proc/hw_nat
    fi
  fi
}


rtl_schedule() {
	echo "flush"> /proc/filter_table
	echo "init 3" > /proc/filter_table
	if [ $URLFILTER_TBL_NUM -gt 0 ] && [ $URLFILTER_ENABLED -gt 0 ];then
		rtl_enable_urlfilter
	else
		rtl_disable_urlfilter
	fi
}

rtl_schedule
