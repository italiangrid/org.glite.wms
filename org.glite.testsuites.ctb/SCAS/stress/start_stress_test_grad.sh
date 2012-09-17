#!/bin/sh

#This file can be used to start the stress test activating
#one machine every n hours. n is passed as argument.

function usage() {
echo "Usage: ./$0 <hours>"
echo "  <hours> time in hours between two activation"
}

if [ $# -ne 1 ]; then
  echo "Bad arguments"
  usage
  exit 1
else
  hours=$1
fi

source ./setup_test.cfg

hostfile1=$log_location/$m1
hostfile2=$log_location/$m2
hostfile3=$log_location/$m3
hostfile4=$log_location/$m4
hostfile5=$log_location/$m5
hostfile6=$log_location/$m6
hostfile7=$log_location/$m7
hostfile8=$log_location/$m8
hostfile9=$log_location/$m9
hostfile10=$log_location/$m10


rm -f ./token_renewal.out
touch ./token_renewal.out
echo "Starting token renewal daemon" 
./token_renewal.sh >> ./token_renewal.out 2>&1 &
token_renewal_pid=$!
echo "Token renewal script has pid $token_renewal_pid " 


echo "-----------------------------------------"
echo "`date`: starting test"
echo "`date +%s` seconds"
echo "-----------------------------------------"

echo "Starting getHourlyErrorRate script"
./getHourlyErrorRate.sh $log_location 1 &
errorRate_pid=$!
echo "getHourlyErrorRate script pid: $errorRate_pid"
rm -f errorRate_pid.txt
touch errorRate_pid.txt
echo "$errorRate_pid" >> errorRate_pid.txt

for hostfile in $hostfile1 $hostfile2 $hostfile3 $hostfile4 $hostfile5 $hostfile6 $hostfile7 $hostfile8 $hostfile9 $hostfile10
#for hostfile in $hostfile1 $hostfile2 $hostfile3 $hostfile4 $hostfile5;
do
  echo "Starting $hostfile"
  echo "START" >> $hostfile
  sleep ${hours}h 
done

exit 0

