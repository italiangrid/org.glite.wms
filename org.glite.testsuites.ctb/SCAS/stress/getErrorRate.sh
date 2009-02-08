#!/bin/sh

function usage() {
echo "$0 <logs_dir>"
echo "  <logs_dir> log files directory"
}

if [ $# -ne 1 ]; then
  echo "Wrong arguments"
  usage
  exit 1
else
  logs_dir=$1
fi

pushd $logs_dir
if [ $? -ne 0 ]; then
  echo "Failed to cd into logs_dir: $logs_dir"
  exit 1
fi 
totalRequests=`cat *_data | wc -l`
totalErrors=`cat *_error | wc -l`
errorRate=$(echo "$totalErrors / $totalRequests" | bc -l)
errorRate=$(echo "$errorRate * 100" | bc -l)

echo "-------------------------------"
echo "`date`"
echo "-------------------------------"
echo "Total requests: $totalRequests"
echo "Total errors: $totalErrors"
echo "Error rate: $errorRate"
echo "-------------------------------"

exit 0

