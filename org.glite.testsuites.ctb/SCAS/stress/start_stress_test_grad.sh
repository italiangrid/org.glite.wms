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


#for hostfile in $hostfile1 $hostfile2 $hostfile3 $hostfile4 $hostfile5 $hostfile6 $hostfile7 $hostfile8 $hostfile9 $hostfile10
for hostfile in $hostfile1 $hostfile2;
do
  echo "START" >> $hostfile
  sleep ${hours}h 
done

echo "`date`: test started"
echo "`date +%s` seconds"
exit 0

