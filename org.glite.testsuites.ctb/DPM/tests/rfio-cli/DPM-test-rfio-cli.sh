#!/bin/bash

. ./helper.sh

if [ "x$1" = "x" ]; then
 if [ ! "x$DPM_HOST" = "x" ]; then
   node=$DPM_HOST
 else
  echo_failure;
  echo "  No target node defined ! Exiting."
  echo "  Usage: $0 <target node> <vo>"
  exit 1
 fi
else
 node=$1
fi

if [ "x$2" = "x" ]; then
 echo_failure;
 echo "  No vo specified ! Exiting."
 echo "  Usage: $0 <target node> <vo>"
 exit 1
else
 vo=$2
fi

echo "Target node is $node. `date`"
echo "Launching RFIO CLI tests:"

mydomain=`echo $node | cut -d "." -f 2-`
mypath=/dpm/$mydomain/home/$vo/rfioclitest$RANDOM
myfile="$mypath/testfile"
myfile2="$mypath/testfile2"

export DPM_HOST=$node
export DPNS_HOST=$node
myglobalerror=0

report() {
 if [ $ret -eq 0 ]; then
   echo_success
 else
   echo_failure
  myglobalerror=1
 fi
 echo "  $message"
}

message="Creating directory [ rfmkdir ] "
rfmkdir $mypath 2>&1 1>/dev/null
ret=$?
report


message="Copying file       [ rfcp ] "
rfcp /etc/hosts $myfile 2>&1 1>/dev/null
ret=$?
report

message="Reading file       [ rfcat ] "
rfcat $myfile 2>&1 1>/dev/null
ret=$?
report

message="Renaming file      [ rfrename ] "
rfrename $myfile ${myfile2}
ret=$?
report

message="Acessing file      [ rfstat ] "
rfstat ${myfile2} 2>&1 1>/dev/null
ret=$?
report

message="Changing permission[ rfstat ] "
rfchmod 777 ${myfile2} 2>&1 1>/dev/null
ret=$?
report

message="Removing file      [ rfrm ] "
rfrm ${myfile2}
ret=$?
report

message="Permission denied  [ rfmkdir ] "
rfmkdir /dpm/alma/korte 2>/dev/null 1>/dev/null
ret=$? 
 if [ $ret -ne 0 ]; then ret=0 
  else ret=1; fi
report


message="Removing dir       [ rfrename ] "
rfrm -f -r $mypath
ret=$?
report



echo
message="Overall RFIO CLI test result:"
ret=$myglobalerror
report
echo
