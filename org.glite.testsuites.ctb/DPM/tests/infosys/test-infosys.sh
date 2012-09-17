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
echo "Testing the Information System:"

# Check resource BDII port
portstate=`nmap -P0 -sT -p 2170 --max_rtt_timeout 1000 ${mynode} | grep "2170/tcp" | awk '{print $2}'`
if [ ! "x${portstate}" = "xopen" ]; then
 myerrorlevel=1
 echo_failure
else
 echo_success; 
fi
 echo "  Resource BDII port is open:"

# Check resource BDII bind string

content=`ldapsearch -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid"`
myret=$?
if [ ! "$myret" = "0" ]; then
 myerrorlevel=1
 echo_failure;
else
 echo_success;
fi
echo "  Resource BDII bind string:"


# Check resource BDII content

for prot in rfio gsiftp; do
lines=`ldapsearch  -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid" GlueSEAccessProtocolLocalID=$prot | wc -l `
if [ $lines -lt 20 ]; then
 echo_failure
 myerrorlevel=1
else
 echo_success
fi
echo "  AccessProtocol $prot published: "
done

for prot in srmv1 srmv2 ; do
lines=`ldapsearch  -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid" GlueSEControlProtocolLocalID=${prot} | wc -l` 
if [ $lines -lt 20 ]; then
 echo_failure
 myerrorlevel=1
else
 echo_success
fi
echo "  ControlProtocol $prot published: "
done

for service in "httpg://$mynode:8443/srm/managerv1" "httpg://$mynode:8446/srm/managerv2"; do
lines=`ldapsearch  -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid" GlueServiceUniqueId=$service | wc -l`
if [ $lines -lt 20 ]; then
 echo_failure
 myerrorlevel=1
else
 echo_success
fi
echo "  Service $service published: "
done

# Checking for backward compatible AccessControlRule publishing

lines=`ldapsearch  -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid" GlueServiceAccessControlRule | grep "GlueServiceAccessControlRule: " | grep -v ": VO:" | wc -l`
lines2=`ldapsearch  -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid" GlueServiceAccessControlRule | grep "GlueServiceAccessControlRule: " | wc -l`
lines3=$((lines))

if [ "$lines3" -ne "$lines2" ]; then
 echo_failure;
 myerrorlevel=1
else
 echo_success;
fi
echo "  AcessControlRule published backward compatible way:"


# Check for default and unset values

lines=`ldapsearch  -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid" | grep 999999`
lines2=`ldapsearch  -x -h $mynode -p 2170 -b "mds-vo-name=resource,o=grid" | grep unset`

if [ ! "x$lines" = "x" ]; then
 echo_failure
 myerrorlevel=1
else
 echo_success
fi
echo "  Unconfigured or default attributes present: "
echo

if [ $myerrorlevel -ne 0 ]; then
 echo_failure
else
 echo_success
fi
echo "  Overall Information System test result: "
echo ""
