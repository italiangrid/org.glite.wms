#!/bin/sh

#This file produce the correct results only when using 
#during the test, not at the end of the test.
#It assumes that:
# - 10 machines are used
# - 10 users on each machine are used
# - all the WNs are active

function usage() {

echo "Usage: $0 <logdir>"
echo "  <logdir> the directory where the log files are stored"
}

if [ $# -ne 1 ]; then
  echo "Wrong arguments"
  usage
  exit 1
fi

if [ -d $1 ]; then
  logdir=$1
  pushd $logdir >> /dev/null
else
  echo "The argument provided is not a valid directory"
  exit 1
fi

rm -f tmpfile
touch tmpfile

for j in `seq 5  6`; do 
  for i in `seq 1  5`; do 
    cat lxb760${j}v${i}.cern.ch | sort | uniq | wc -l >> tmpfile; 
  done; 
done

#In tmpfile I should have 10 rows with '11'
errors=`egrep  '[^11]' tmpfile | wc -l`

echo "Errors: $errors"

popd >> /dev/null

exit 0
