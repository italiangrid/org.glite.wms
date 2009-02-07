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


#TODO
#hostnames are defined in cfg file
hostname=`hostname`
host1=vtb-generic-111.cern.ch
host2=lxb7606v1.cern.ch

echo "Log file location: $log_location"
echo "Removing old log files if present"

rm -f $log_location/$hostname
for host in $host1 $host2;
do
  rm -f $log_location/$host
done

echo "Preparing the nodes"

i=0
for host in $host1 $host2;
do
  i=$[$i+1]
  echo "Starting host $host"
  hostlogfile=$host.log
  echo "Log file: $hostlogfile"
  ssh root@$host "expect $get_afstoken_script $afs_user $afs_pass; touch $log_location/$host; rm -f $hostlogfile; touch $hostlogfile; $setup_node_script $i $log_location/$host >> $hostlogfile 2>&1" &
  #ssh root@$host "expect $get_afstoken_script $afs_user $afs_pass; touch $log_location/$host; rm -f $hostlogfile; touch $hostlogfile; $setup_node_script $i $log_location/$host" &
  echo "$host started"
done

echo "Nodes started, add the string "START" to their log file to trigger the start of the stress test"
exit 0
