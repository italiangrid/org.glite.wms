#!/bin/sh

function usage() {
  echo "Usage: $0 <input_file> <t0>"
  echo "  <input_file> data file with pairs (timestamp,data)"
  echo "  <t0> start time"
}

if [ $# -ne 2 ];then
  echo "Wrong arguments"
  usage
  exit 1
else
  datafile=$1
  t0=$2
fi

if [ ! -f $datafile ]; then
  echo "$datafile is not a valid file"
  usage
  exit 1
fi
new_file=${datafile}_norm
rm -f $new_file

awk -F , "{if(NR==1) {OFS = \",\"; START = $t0} \$1 -= START; print \$0}" $datafile > $new_file

#TOTALLY UNEFFICIENT
#while read line
#do 
#  old_timestamp=`echo $line |  cut -d, -f1`
#  data=`echo $line |  cut -d, -f2`
#  new_timestamp=`echo "$old_timestamp - $t0" | bc`
#  echo "${new_timestamp},${data}" >> $new_file
#done < $datafile

echo "$new_file ready"
exit 0


