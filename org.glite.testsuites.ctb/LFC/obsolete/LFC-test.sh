#!/bin/sh

workdir=`dirname $0`
short_result="${workdir}/short_result"
full_result="${workdir}/full_result"
export SAME_OK=0
export SAME_ERROR=1
export SAME_SENSOR_HOME=$workdir
export SAME_SENSOR_WORK=$workdir

usage() {
 echo "USAGE: $0 -n <LFC hostname> -l <file with test list> -v <VO executing the test> [ -s <file with short test result> -f <file with full test result>]"
echo "If -s or -f are not specified the test results will be written to the files ./short_result and ./full_result."
}

while getopts "v:n:l:h" opt
do
 case $opt in
  n) nodename=$OPTARG
     ;;
  s) short_result=$OPTARG
     ;;
  f) full_result=$OPTARG
     ;;
  l) testlist=$OPTARG
     ;;
  v) SAME_VO=$OPTARG
     ;;
  h) usage
     exit 0
     ;;
  '?') echo "$0: Invalid option -$OPTARG" >&2
     usage
     exit 1
     ;;
 esac
done

export SAME_VO

if [ -z $nodename ] || [ -z $testlist ] || [ -z $SAME_VO ]; then
 usage
 exit 1
fi

export SAME_OK=0
export SAME_ERROR=1
export SAME_SENSOR_HOME=`pwd`
export SAME_SENSOR_WORK=`pwd`

rm -f $short_result
rm -f $full_result

for test in `cat $testlist` ; do
 echo "Running test $test"
 $SAME_SENSOR_HOME/tests/$test $nodename >> $full_result
 if [ $? -eq $SAME_OK ]; then
  echo -e "Test $test\t OK" >> $short_result
 else
  echo -e "Test $test\t FAILED" >> $short_result
 fi
done

echo "Short report:"
cat $short_result
echo "Full test results available in $full_result"
