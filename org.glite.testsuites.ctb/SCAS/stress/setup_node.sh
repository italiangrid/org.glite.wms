#!/bin/sh


#TODO
#Remove hardcoded conf file

function usage() {
echo "Usage: ./setup_node.sh <index> <log file>"
echo "    <index> integer number to identify the node."
echo "    <log file> the file that will be used to log data and start the test."
}

if [ $# -ne 2 ]; then
  usage
  exit 1
fi

INDEX=$1

if [ ! -f $2 ]; then
  echo "The second argument must be a valid log file" 
  usage
  exit 1
else
  LOG_FILE=$2
fi

#source the configuration file
source /afs/cern.ch/user/p/pucciani/public/glitetests/src/org.glite.testsuites.ctb/SCAS/stress/setup_test.cfg
if [ $? -ne 0 ]; then
  echo "Error sourcing the setup_test.cfg file" 
  exit 1
fi

rm -f ./proxy_renewal.out
touch ./proxy_renewal.out

echo "Retrieving glexec test script" 
cp -f --reply=yes $glexec_stress_script ./
echo "Retrieving proxy renewal script" 
cp -f --reply=yes $proxy_ren_script ./
echo "Starting proxy renewal daemon" 
./proxy_renewal.sh $INDEX >> ./proxy_renewal.out 2>&1 &
proxy_renewal_pid=$! 
echo "Proxy renewal script has pid $proxy_renewal_pid " 

#Give time to create the proxy
sleep 10s

#use special certificate for debugging
#cp -f /afs/cern.ch/user/p/pucciani/tmp/x509up_u501_g /home/dteampilot$INDEX/x509up_u501_$INDEX 2>&1 >> $LOG_FILE
#echo "Copying test and proxy to ~dteampilot$INDEX"
#cp -f x509up_u501_$INDEX /home/dteampilot$INDEX 2>&1 >> $LOG_FILE
#if [ $? -ne 0 ]; then
#  echo "Error copying the proxy"
#  exit 1
#fi

cp -f ./glexec_stress_test.sh /home/dteamdteampilot$INDEX/ 
chown dteamdteampilot$INDEX /home/dteamdteampilot$INDEX/glexec_stress_test.sh 
chmod u+x /home/dteamdteampilot$INDEX/glexec_stress_test.sh 
#chown dteamdteampilot$INDEX /home/dteamdteampilot$INDEX/x509up_u501_$INDEX 

hostname=`hostname`
hostlogfile=$hostname.log

echo "Starting glexec test as dteamdteampilot$INDEX" 
if [ "x$iterations" != "x" ];then
  su - -c "/home/dteamdteampilot$INDEX/glexec_stress_test.sh -f $LOG_FILE -n $iterations -i $INDEX" dteamdteampilot$INDEX
fi
if [ "x$end_date" != "x" ];then
  su - -c "/home/dteamdteampilot$INDEX/glexec_stress_test.sh -f $LOG_FILE -d $end_date -i $INDEX" dteamdteampilot$INDEX >> $hostlogfile 2>&1
fi

#kill the proxy renewal process
kill $proxy_renewal_pid
if [ $? -eq 0 ];then
  echo "Proxy renewal daemon killed" 
else
  echo "Error in killing proxy renewal daemon, pid is $proxy_renewal_pid" 
fi

exit 0

