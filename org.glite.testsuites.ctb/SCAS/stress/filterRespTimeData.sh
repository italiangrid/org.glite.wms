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
filtered=datafilt_${datafile}

rm -f $filtered
end=20

echo -n "zone 1 [0,2): " >> $filtered
awk -F , "BEGIN {START=0; STOP=2} {if(\$2 >= START && \$2 < STOP) print \$0}" $datafile | wc -l >> $filtered
echo -n "zone 2 [2,10): " >> $filtered
awk -F , "BEGIN {START=2} {if(\$2 >= START && \$2 < 10) print \$0}" $datafile | wc -l >> $filtered
echo -n "zone 2 [10,+inf): " >> $filtered
awk -F , "BEGIN {START=10} {if(\$2 >= START) print \$0}" $datafile | wc -l >> $filtered

echo "Data fitered file: ${dir}/${filtered}"

exit 0
