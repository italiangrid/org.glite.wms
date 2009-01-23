#!/bin/sh

#This script executes the glexec test either n times or until"
#a specified date. The start is done when the word 'START' is"
#found in the specified log file"

function usage() {

  echo "Usage: ./glexec_stress_test.sh -f <log file> {-n <number>|-d <end date>} -i <index>"
  echo "      <number> an integer number to state the number of tests to be done."
  echo "      <end date> a date until when to execute the test."
  echo "                 The format must be YYYYMMDDhhmm (returned by date +%Y%m%d%H%M)."
  echo "                 E.g. 20 January 2009 at 8am => 200901200800"
  echo "      <index> an integer number to identify the node"
}

if [ $# -lt 6 ]; then
  usage
  exit 1
fi

until [ -z "$1" ]
do
  case "$1" in
     -n)
         if [ -z "$2" ]; then
           usage
           exit 1
         else
           ITERATIONS=$2
           shift 2
         fi
     ;;
     -d)
         if [ -z "$2" ]; then
           usage
           exit 1
         else
           END_DATE=$2
           shift 2
         fi
     ;;
     -f)
         if [ -z "$2" ]; then
           usage
           exit 1
         else
           LOG_FILE=$2
           shift 2
         fi
     ;;
     -i)
         if [ -z "$2" ]; then
           usage
           exit 1
         else
           INDEX=$2
           shift 2
         fi
     ;;

     *)
        usage
        exit 1
     ;;
  esac
done

if [ ! -z $END_DATE ]; then
  echo "The test will end on $END_DATE"
fi
if [ ! -z $ITERATIONS ]; then
  echo "The test will do $ITERATIONS iterations"
fi        
if [ ! -r $LOG_FILE -o  ! -w $LOG_FILE ]; then
  echo "Please specify a file for logging and starting, the file must be rw"
  echo "by the current user."
  usage
  exit 1
fi

#Set proxies for glexec
user_proxy="./x509up_u501_$INDEX"
export GLEXEC_CLIENT_CERT=$user_proxy
export GLEXEC_SOURCE_PROXY=$user_proxy
export X509_USER_PROXY=$GLEXEC_CLIENT_CERT
echo "Using proxy $user_proxy"

DATA_FILE=${LOG_FILE}_data
rm -f $DATA_FILE
touch $DATA_FILE

#Wait for the start signal
while [ ! `grep 'START' $LOG_FILE` ]
do
  sleep 1
done

if [ ! -z $END_DATE ]; then
  CURR_DATE=`date +%Y%m%d%H%M`
  while [ $CURR_DATE -lt $END_DATE ]
  do
    START_TIME=`date +%s.%N`
    $GLITE_LOCATION/sbin/glexec "/usr/bin/whoami" 2>&1 >> $LOG_FILE
#    TIME=`/usr/bin/time -f "%e" $GLITE_LOCATION/sbin/glexec "/usr/bin/whoami" >> $LOG_FILE`
#    TIME=`/usr/bin/time -f "%e" $GLITE_LOCATION/sbin/glexec "/usr/bin/whoami" >> $LOG_FILE`
    END_TIME=`date +%s.%N`
    TIME=`echo "$START_TIME $END_TIME" | awk '{print $2-$1}'`
    echo "`date +%s`,$TIME" >> $DATA_FILE
    CURR_DATE=`date +%Y%m%d%H%M`
  done
fi

count=1
if [ ! -z $ITERATIONS ]; then
  while [ $count -le $ITERATIONS ]
  do
    START_TIME=`date +%s.%N`
    $GLITE_LOCATION/sbin/glexec "/usr/bin/whoami" 2>&1 >> $LOG_FILE
#    /usr/bin/time -f "%e" $GLITE_LOCATION/sbin/glexec "/usr/bin/whoami" >> $LOG_FILE
#    TIME=`/usr/bin/time -f "%e" $GLITE_LOCATION/sbin/glexec "/usr/bin/whoami" >> $LOG_FILE`
    END_TIME=`date +%s.%N`
    TIME=`echo "$START_TIME $END_TIME" | awk '{print $2-$1}'`
    echo "`date +%s`,$TIME" >> $DATA_FILE
    count=$[count+1]
  done
fi

echo "END"

exit 0

