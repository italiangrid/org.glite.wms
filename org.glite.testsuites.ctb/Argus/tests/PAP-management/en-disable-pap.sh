#!/bin/sh

PAP_HOME=/opt/authz/pap
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-Enable/Disable-PAP---"
###############################################################
echo "1) testing dpap with non existing pap"
/opt/authz/pap/bin/pap-admin dpap mypap
if [ $? -eq 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "2) testing dpap with already disabled pap"

#Add pap
/opt/authz/pap/bin/pap-admin apap mypap 
if [ $? -ne 0 ]; then
  echo "Failed adding a pap"
  exit 1
fi

/opt/authz/pap/bin/pap-admin dpap mypap
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "3) testing epap with wrong alias"
/opt/authz/pap/bin/pap-admin epap Dummy
if [ $? -eq 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "4) testing epap with good alias"
/opt/authz/pap/bin/pap-admin epap mypap
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  /opt/authz/pap/bin/pap-admin list-paps | grep mypap | grep -q enabled
  if [ $? -eq 0 ]; then
    echo "OK"
  else
    echo "Failed"
    failed="yes"
  fi
fi

###############################################################
echo "4) testing dpap with good alias"
/opt/authz/pap/bin/pap-admin dpap mypap
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  /opt/authz/pap/bin/pap-admin list-paps | grep mypap | grep -q disabled
  if [ $? -eq 0 ]; then
    echo "OK"
  else
    echo "Failed"
    failed="yes"
  fi
fi


###############################################################
#Remove pap
/opt/authz/pap/bin/pap-admin rpap mypap
if [ $? -ne 0 ]; then
  echo "Failed removed pap"
fi


###############################################################
if [ $failed == "yes" ]; then
  echo "---Test-Enable/Disable-PAP: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-Enable/Disable-PAP: TEST PASSED---"
  echo `date`
  exit 0
fi

