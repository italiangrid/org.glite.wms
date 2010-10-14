#!/bin/sh

script_name=`basename $0`
failed="no"

# Make sure we do NOT have a PAP running at start

/etc/rc.d/init.d/pap-standalone stop > /dev/null 2>&1;
sleep 3;
/etc/rc.d/init.d/pap-standalone status > /dev/null 2>&1;
result=$?
if [ $? -ne 0 ]; then
  echo "pap-standalone status should return non-zero. returned ${result}."
  failed="yes"
  exit 1
else
  echo "pap-standalone status returned ${result}. OK."
fi

/etc/init.d/pdp stop > /dev/null 2>&1;
sleep 3;
/etc/init.d/pdp status > /dev/null 2>&1;
result=$?; 
if [ $? -ne 0 ]; then
  echo "pdp status should return non-zero. returned ${result}."
  failed="yes"
  exit 1
else
  echo "pdp status returned ${result}. OK."
fi

/etc/init.d/pepd stop > /dev/null 2>&1;
sleep 3;
/etc/init.d/pepd status > /dev/null 2>&1;
result=$?;
if [ $? -ne 0 ]; then
  echo "pepd status should return non-zero. returned ${result}."
  failed="yes"
  exit 1
else
  echo "pepd status returned ${result}. OK."
fi

if [ $failed == "yes" ]; then
  echo "---${script_name}: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---${script_name}: TEST PASSED---"
  echo `date`
  exit 0
fi

