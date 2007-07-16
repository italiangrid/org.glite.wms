#!/bin/bash
DATE=`date -Iminutes`
OUTCE="IDs/$DATE"_".CE.out
OUTBDII="IDs/$DATE"_".BDII.out

while true
do	

	./monitor_WNsge.sh >> outWN_"$DATE"
	sleep 60


done
