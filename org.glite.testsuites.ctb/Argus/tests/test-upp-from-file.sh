#!/bin/sh

PAP_HOME=/opt/authz/pap
failed="no"
policyfile=policyfile.txt

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

echo `date`
###############################################################
echo "---Test-Update-Policy-From-File---"
echo "1) testing up with non existing file"

/opt/authz/pap/bin/pap-admin up resource_id dummy.txt

if [ $? -ne 0 ]; then
  echo "OK"
else
  echo "Failed"
  failed="yes"
fi

###############################################################
echo "2) testing up with non existing resource id"

#Store initial policy
cat <<EOF > $policyfile
resource "resource_1" {
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


#Create new policy file
cat <<EOF > $policyfile
resource "resource_2" {
   action ".*" {
        rule permit { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
EOF

/opt/authz/pap/bin/pap-admin up resource_2 $policyfile
if [ $? -ne 0 ]; then
  echo "OK" 
else
  echo "Failed"
  failed="yes"
fi

###############################################################
echo "3) testing up with non existing resource id 2"

#Create new policy file
cat <<EOF > $policyfile
resource "resource_2" {
   action ".*" {
        rule permit { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
EOF

/opt/authz/pap/bin/pap-admin up resource_3 $policyfile
if [ $? -ne 0 ]; then
  echo "OK" 
else
  echo "Failed"
  failed="yes"
fi

###############################################################
echo "4) testing up with correct resource id "

#Create new policy file
cat <<EOF > $policyfile
resource "resource_1" {
   action ".*" {
        rule permit { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
EOF

/opt/authz/pap/bin/pap-admin up resource_1 $policyfile
if [ $? -eq 0 ]; then
  echo "OK" 
else
  echo "Failed"
  failed="yes"
fi

###############################################################
#clean up
/opt/authz/pap/bin/pap-admin ban subject "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" --resource "resource_1"
if [ $? -ne 0 ]; then
  echo "Error during clean up"
  echo "Failed command: /opt/authz/pap/bin/pap-admin ban subject \"/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" --resource "resource_1\""
  failed="yes"
fi


if [ $failed == "yes" ]; then
  echo "---Test-PAP-FUNC-2: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-PAP-FUNC-2: TEST PASSED---"
  echo `date`
  exit 0
fi

