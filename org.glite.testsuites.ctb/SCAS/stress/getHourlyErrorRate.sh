#!/bin/sh

function usage() {
echo "Usage: $0 <logs_dir> <hours>"
echo "  <logs_dir> log files directory"
echo "  <hours> time interval in hours"
}

if [ $# -ne 2 ]; then
  echo "Wrong arguments"
  usage
  exit 1
else
  logs_dir=$1
  hours=$2
fi

if [ $? -ne 0 ]; then
  echo "Failed to cd into logs_dir: $logs_dir"
  exit 1
fi 
pushd $logs_dir
rm -f hourlyErrorRate.txt
touch hourlyErrorRate.txt
rm -f frequency.txt
touch frequency.txt

hours=0
while [ 1 ]
do
  sleep 1h
  let "hours++" 
  totalRequests=`cat *_error | wc -l`
  totalErrors=`grep ',1' *_error | wc -l`
  time=$(echo "60*60*$hours")
  frequency=$(echo "$totalRequests / $time" | bc -l)
  echo "`date +%s`,$frequency" >> frequency.txt

  errorRate=$(echo "$totalErrors / $totalRequests" | bc -l)
  errorRate=$(echo "$errorRate * 100" | bc -l)
  echo "`date +%s`,$errorRate" >> hourlyErrorRate.txt
done
exit 0

