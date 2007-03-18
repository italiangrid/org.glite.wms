#!/bin/bash

if [ $1 = 1 ]; then 
 echo "==================== Try to make C++ job"
 make -f makeWN 2>&1
 ret=$?

 if [ $ret != 0 ]; then
  echo "================== Warning: can not make C++ job. hostname: `hostname` :"
  echo "make exit with exit code= $ret"

  echo "================== Run job from InputSandbox"
  chmod 755 ./chkjob1
  ./chkjob1
 else
  echo "make C++ job OK"
  echo "================== Run job"
  ./chkjob
 fi
else
 echo "================== Run job from InputSandbox"
 chmod 755 ./chkjob1
 ./chkjob1
fi

exit 0


