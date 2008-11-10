#!/bin/bash
#
# Stress tests the hydra database with multiple users and threads
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

. $(dirname $0)/functions.sh

if [ $# -ne 0 ] ; then
  usage $0
  if [ $1 = "--help" ] ; then
    my_exit $SUCCESS
  else
    my_exit $ERROR
  fi
fi

setup 

START=`date`
SECSTART=`date +%s`
echo "Started at $START"

for (( i=0 ; i<$STRESS_TEST_PROCS ; i++ )) ; do
   ./stress-exec.sh  $X509_USER_PROXY &
done

echo "Started $i processes"

wait

END=`date`
SECEND=`date +%s`
echo "Started at $START"
echo "Finished at $END"

tottime=$((SECEND-SECSTART))
totcom=$((STRESS_TEST_PROCS*STRESS_TEST_KEYS_PER_PROC*2))
echo "Handled $totcom commands in $tottime seconds"
avgtime=`echo "scale=1;$tottime/$totcom"|bc`
echo "Average time per command was $avgtime seconds"

###############################
############ Done #############
###############################

echo "Tests completed"
my_exit $SUCCESS
