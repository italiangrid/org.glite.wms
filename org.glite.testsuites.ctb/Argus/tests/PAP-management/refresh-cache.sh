#!/bin/sh

PAP_HOME=/opt/authz/pap
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-Refesh-Cache---"
###############################################################
echo "1) testing rc with non existing alias"
/opt/authz/pap/bin/pap-admin rc Do-Not-Exist
if [ $? -eq 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "2) testing rc with a local pap"
/opt/authz/pap/bin/pap-admin rc default
if [ $? -eq 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
if [ $failed == "yes" ]; then
  echo "---Test-Refesh-Cache: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-Refesh-Cache: TEST PASSED---"
  echo `date`
  exit 0
fi

