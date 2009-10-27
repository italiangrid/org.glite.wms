#!/bin/bash

if [ "x$1" = "x" ]; then
 echo " Usage: $0 <host name to test>"
 exit 1
fi
. ./helper.sh
myerrorlevel=0
mynode="$1"
echo
echo "Target node is $mynode. Date: `date`"
echo "Testing the default DPM services:"


for service in "5001 rfiod" "3306 mysqld" "5010 dpnsdaemon" "5015 dpm"  "2170 bdii" "22 sshd" "8443 srmv1" "8446 srmv2.2" "2811 globus-gridftp"; do
port=`echo $service | cut -d " " -f 1`
serv=`echo $service | cut -d " " -f 2`
state=`nmap -P0 -sT -p $port --max_rtt_timeout 1000 ${mynode} | grep "$port/tcp" | awk '{print $2}'`
if [ ! "x$state" = "xopen" ]; then
 echo_failure
 myerrorlevel=1
else
 echo_success
fi
echo "  Testing service $serv on port $port"
done
echo
if [ $myerrorlevel -ne 0 ]; then
 echo_failure
else
 echo_success
fi
echo "  Overall Service Ping test result: "

