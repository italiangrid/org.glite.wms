#!/bin/sh

function usage() {
echo "Usage: ./setup_test.sh"
}

if [ $# -gt 0 ]; then
  usage
  exit 1
fi

#setup_node="/afs/cern.ch/user/p/pucciani/public/glitetests/scas/stress/setup_node.sh"
#log_location="/afs/cern.ch/user/p/pucciani/tmp"
#get_afstoken_script="/afs/cern.ch/user/p/pucciani/public/glitetests/scas/stress/get_afstoken.exp"
#pass_file="/afs/cern.ch/user/p/pucciani/public/glitetests/scas/stress/passwd.txt"
#afs_pass_file=/afs/cern.ch/user/p/pucciani/private/afs_magic
#afs_user="pucciani"
source setup_test.cfg

afs_pass=`cat $afs_pass_file` 


hostname=`hostname`
host1=vtb-generic-111.cern.ch
host2=lxb7606v1.cern.ch
#host1=lxb7605v1.cern.ch
#host2=lxb7605v2.cern.ch
#host3=lxb7605v3.cern.ch
#host4=lxb7605v4.cern.ch
#host5=lxb7605v5.cern.ch
#host6=lxb7606v1.cern.ch
#host7=lxb7606v2.cern.ch
#host8=lxb7606v3.cern.ch
#host9=lxb7606v4.cern.ch
#host10=lxb7606v5.cern.ch

echo "Log file location: $log_location"
echo "Removing old log files"

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
