#!/bin/sh

function usage() {
echo "Usage: ./setup_test.sh"
}

if [ $# -gt 0 ]; then
  usage
  exit 1
fi

#source the main configuration file
source setup_test.cfg

afs_pass=`cat $afs_pass_file` 


echo "Log file location: $log_location"
echo "Removing old log files if present"

for host in $m1 $m2 $m3 $m4 $m5 $m6 $m7 $m8 $m9 $m10;
do
  rm -f $log_location/$host
done

echo "Preparing the nodes"

i=0
for host in $m1 $m2 $m3 $m4 $m5 $m6 $m7 $m8 $m9 $m10;
do
  i=$[$i+1]
  echo "Starting host $host"
  hostlogfile=$host.log
  echo "Log file: $hostlogfile"
  ssh root@$host "expect $get_afstoken_script $afs_user $afs_pass; touch $log_location/$host; rm -f $hostlogfile; touch $hostlogfile; $setup_node_script $i $log_location/$host >> $hostlogfile 2>&1" &
  echo "$host started"
done

echo "Nodes started, add the string "START" to their log file to trigger the start of the stress test"
exit 0
