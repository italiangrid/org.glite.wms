#!/bin/sh

PAP_HOME=/opt/authz/pap
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-Set/Get-Poll-Interval---"
###############################################################
echo "1) Setting polling time"
/opt/authz/pap/bin/pap-admin spi 100
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "2) Retrieving polling time"
time=`/opt/authz/pap/bin/pap-admin gpi 100 | sed 's/Polling interval in seconds: //g'`
if [ $time -ne 100 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
if [ $failed == "yes" ]; then
  echo "---Test-Set/Get-Poll-Interval: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-Set/Get-Poll-Interval: TEST PASSED---"
  echo `date`
  exit 0
fi

