#!/bin/sh
#
# script file to init system 
# running in mp mode or normal mode according to the para
#
if [ $1 = 'mp' ]; then
	/bin/nfbi_mp.sh $2	
else
	hcd -daemon
fi

