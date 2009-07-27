#!/bin/sh

PAP_HOME=/opt/authz/pap
failed="no"
policyfile=policyfile.txt

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

#Remove all policies defined for the default pap
/opt/authz/pap/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: /opt/authz/pap/bin/pap-admin rap"
  exit 1
fi

echo `date`
echo "---Test-List-Policies---"
###############################################################
echo "1) testing list policies on an empty repository"

/opt/authz/pap/bin/pap-admin lp 

if [ $? -eq 0 ]; then
  echo "OK" 
else
  echo "Failed"
  failed="yes"
fi

###############################################################
echo "2) testing list policies"

#Store initial policy
cat <<EOF > $policyfile
resource "resource_1" {
    action ".*" {
        rule deny { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
resource "resource_2" {
    action ".*" {
        rule deny { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
resource "resource_3" {
    action ".*" {
        rule deny { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}

EOF
/opt/authz/pap/bin/pap-admin apf $policyfile
if [ $? -ne 0 ]; then
  echo "Error preparing the test environment"
  echo "Failed command: /opt/authz/pap/bin/pap-admin apf $policyfile"
  exit 1
fi

#Retrieve resource id
lines=`/opt/authz/pap/bin/pap-admin lp -sai | egrep -c 'id='`

if [ $lines -eq 9 ]; then
  echo "OK" 
else
  echo "Failed"
  echo "expecting 9 ids, found $lines"
  failed="yes"
fi

###############################################################
echo "2) testing list policies with wrong pap-alias"

#Retrieve resource id
/opt/authz/pap/bin/pap-admin lp -sai --pap "dummy_pap"

if [ $? -ne 0 ]; then
  echo "OK" 
else
  echo "Failed"
  failed="yes"
fi

###############################################################
#clean up
rm -f $policyfile
#Remove all policies defined for the default pap
/opt/authz/pap/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: /opt/authz/pap/bin/pap-admin rap"
  exit 1
fi


if [ $failed == "yes" ]; then
  echo "---Test-List-Polices: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-List-Polices: TEST PASSED---"
  echo `date`
  exit 0
fi

