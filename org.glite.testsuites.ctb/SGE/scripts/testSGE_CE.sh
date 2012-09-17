#!/bin/bash
DATE=`date -Iminutes`
OUTCE="IDs/$DATE"_".CE.out
OUTBDII="IDs/$DATE"_".BDII.out

while true
do	

	./monitor_CEsge.sh >> outCE_"$DATE"
	./monitor_BDIIsge.sh >> outBDII_"$DATE"
	sleep 60


done
