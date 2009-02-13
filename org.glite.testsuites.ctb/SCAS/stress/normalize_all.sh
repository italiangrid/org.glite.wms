#!/bin/sh

function usage() {
  echo "Usage $0 <t0> <log_location>"
  echo "  <t0> start time"
  echo "  <log_location> directory where all data files are stored"
}

if [ $# -ne 2 ]; then
  echo "Wrong arguments"
  usage
  exit 1
else
  t0=$1
  log_location=$2
fi

if [ ! -d $log_location ]; then
  echo "$log_location is not a valid directory"
  exit 1
fi

for file in `ls $log_location/*_data $log_location/*_error`
do
  ./normalize_time.sh $file $t0
done

#mv -f *_norm $log_location >> /dev/null

echo "Time normalized files are in $log_location"
exit 0

