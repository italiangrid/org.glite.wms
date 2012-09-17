#!/bin/sh

function usage() {
  echo "Usage: $0 <datafile> <t1> <t2>"
  echo "  <datafile> a file with pairs (timestamp,data)"
  echo "  <t1> start time between double quotes (format as printed by date +\"%b %d %R:%S %Y\", e.g Feb 12 15:31:57 2009)" 
  echo "  <t2> end time (same format as above)"
}


if [ $# -lt 3 ]; then
  echo "Wrong arguments"
  usage
  exit 1
else
  datafile=$1
  tstart=$2
  tend=$3
fi

if [ ! -f $datafile ]; then
  echo "$1 is not a valid file"
  exit 1
fi

choppedfile="chop_`basename $datafile`"

sec1=`date -d "$tstart" +%s`
sec2=`date -d "$tend" +%s`
echo "Chopping from $sec1 to $sec2"

awk -F , "BEGIN {START=$sec1; STOP=$sec2} {if(\$1 >= START && \$1 <= STOP) print \$0}" $datafile > $choppedfile

echo "Chopped file ready: $choppedfile"
exit 0
