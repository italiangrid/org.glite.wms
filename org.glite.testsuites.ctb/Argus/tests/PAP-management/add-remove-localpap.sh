#!/bin/sh

PAP_HOME=/opt/authz/pap
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Add/Remove-local-PAP---"
###############################################################
echo "1) testing apap with existing alias"
/opt/authz/pap/bin/pap-admin apap default
if [ $? -eq 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "2) testing apap with wrong endpoint"
/opt/authz/pap/bin/pap-admin apap NewPAP --url "https://localhost:8555/pap/services/"
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "3) testing apap local"
/opt/authz/pap/bin/pap-admin apap NewPAP 
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  /opt/authz/pap/bin/pap-admin list-paps | grep -q 'NewPAP'
  if [ $? -ne 0 ]; then
    echo "Failed"
    failed="yes"
  else
    echo "OK"
  fi
fi

###############################################################
echo "3) test removing local pap"
/opt/authz/pap/bin/pap-admin remove-pap NewPAP
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
if [ $failed == "yes" ]; then
  echo "---Test-Add/Remove-local-PAP: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-Add/Remove-local-PAP: TEST PASSED---"
  echo `date`
  exit 0
fi

