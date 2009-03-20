#!/bin/sh

#Levels are for now hardcoded

function usage() {
  echo "Usage $0 <input file>"
  echo "  <input file> a file *_data with response time data"
}

if [ $# -ne 1 ]; then
  echo "Wrong arguments"
  usage
  exit 1
fi


dir=`dirname $1`

pushd $dir >> /dev/null

datafile=`basename $1`
filtered=filt_${datafile}

rm -f $filtered
end=20

for i in `seq 1 0.1 $end `; do

start=`echo "$i-1" | bc`
stop=$i
echo -n "$stop " >> $filtered
awk -F , "BEGIN {START=$start; STOP=$stop} {if(\$2 >= START && \$2 < STOP) print \$0}" $datafile | wc -l >> $filtered

done

#last interval
last=`echo "$end + 1" | bc`
echo -n "$last " >> $filtered
awk -F , "BEGIN {START=$last} {if(\$2 >= START) print \$0}" $datafile | wc -l >> $filtered


echo "Fitered file: ${dir}/${filtered}"

exit 0
