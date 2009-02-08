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

while [ 1 ]
do
  sleep 1h
  totalRequests=`cat *_data | wc -l`
  totalErrors=`cat *_error | wc -l`
  errorRate=$(echo "$totalErrors / $totalRequests" | bc -l)
  errorRate=$(echo "$errorRate * 100" | bc -l)
  echo "`date +%s`,$errorRate" >> hourlyErrorRate.txt
done
exit 0

