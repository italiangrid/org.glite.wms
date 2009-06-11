#!/bin/sh

function usage() {
  echo "Usage: $0 <CreamEndPoint> <jdl> <#jobPerUser> <user1> <userN>"
  echo "  <CreamEndPoint> Ex: lxbra1908.cern.ch:8443/cream-lsf-normal"
  echo "  <jdl> JDL file"
  echo "  <#jobPerUser> Number of Jobs to submit for each user"
  echo "  <user1> Number of the first test user"
  echo "  <userN> Number of the last test user"
}

if [ $# != 5 ]; then 
  usage
  exit 1
fi

if [ -f commonFunctions.sh ]; then
  source commonFunctions.sh
else
  echo "commonFunctions.sh file not found"
  exit 1
fi


creamEndPoint=$1
jdlFile=$2
jobPerUser=$3
user1=$4
userN=$5

VO1="CREAMA"
VO2="CREAMB"
VO3="DTEAM"
VO4="org.glite.voms-test"

firstcreamauser=$user1
lastcreamauser=$(( $firstcreamauser + 15 ))
firstcreambuser=$(( $lastcreamauser + 1 ))
lastcreambuser=$(( $firstcreambuser + 15 ))
firstdteamuser=$(( $lastcreambuser + 1 ))
lastdteamuser=$(( $firstdteamuser + 15 ))
firstogvtuser=$(( $lastdteamuser + 1 ))
lastogvtuser=$(( $firstogvtuser + 15 ))

if [ $lastogvtuser -ne $userN ]; then
  echo "Error in users setup"
  exit 1
fi

createProxies
if [ $? -eq 0 ]; then
  echo "Proxies ready in users-certs"
fi 

rm -rf jobids
rm -rf status
mkdir status
mkdir jobids

for jobindex in `seq 1 $jobPerUser`;
do
  submitJobsAllUsers $jobindex
done

echo "Jobs submitted"

echo "Resting a few minutes"
sleep 120

totalfailed=0
for jobindex in `seq 1 $jobPerUser`;
do
  checkOutput $jobindex
done

echo "Total jobs failed: $totalfailed"
echo "Output check finished: `date`"

exit 0

 
