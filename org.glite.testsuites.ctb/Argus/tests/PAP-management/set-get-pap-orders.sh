#!/bin/sh

PAP_HOME=/opt/authz/pap
failed="no"

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
echo "---Test-Set/Get-paps-order---"
###############################################################
echo "1) testing gpo with no order"
/opt/authz/pap/bin/pap-admin gpo
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "2) testing spo with 3 paps"

#Add 3 local paps
/opt/authz/pap/bin/pap-admin apap local-pap1
/opt/authz/pap/bin/pap-admin apap local-pap2
/opt/authz/pap/bin/pap-admin apap local-pap3
if [ $? -ne 0 ]; then
  echo "Error addings paps"
  exit 1
fi


/opt/authz/pap/bin/pap-admin spo local-pap1 local-pap2 local-pap3 default
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "2) Inverting the order"

/opt/authz/pap/bin/pap-admin spo default local-pap3 local-pap2 local-pap1
if [ $? -ne 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi

###############################################################
echo "3) using a non existing alias"

/opt/authz/pap/bin/pap-admin spo default local-pp3 local-pp2 local-pp1 
if [ $? -eq 0 ]; then
  echo "Failed"
  failed="yes"
else
  echo "OK"
fi


###############################################################
#Removing paps
/opt/authz/pap/bin/pap-admin rpap local-pap1
/opt/authz/pap/bin/pap-admin rpap local-pap2
/opt/authz/pap/bin/pap-admin rpap local-pap3
if [ $? -ne 0 ]; then
  echo "Error removing paps"
fi



###############################################################
if [ $failed == "yes" ]; then
  echo "---Test-Set/Get-paps-order: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-Set/Get-paps-order: TEST PASSED---"
  echo `date`
  exit 0
fi

