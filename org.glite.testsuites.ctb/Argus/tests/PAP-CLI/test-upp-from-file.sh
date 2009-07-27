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
resource "resource_1" {
   action ".*" {
        rule permit { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
EOF

/opt/authz/pap/bin/pap-admin up dummy-id-999 $policyfile
if [ $? -ne 0 ]; then
  echo "OK" 
else
  echo "Failed"
  failed="yes"
fi

###############################################################
# Retrieve resource id
echo "3) testing up with correct resource id "
id=`/opt/authz/pap/bin/pap-admin lp -srai | egrep 'id=[^public]' | sed 's/id=//'`
echo "ID=$id"

#Create new policy file
cat <<EOF > $policyfile
resource "resource_1" {
   action ".*" {
        rule permit { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
    }
}
EOF

/opt/authz/pap/bin/pap-admin up $id $policyfile
if [ $? -eq 0 ]; then
  echo "OK" 
else
  echo "Failed"
  failed="yes"
fi

###############################################################
# Retrieve resource id

echo "4) testing up with changing only an action "

#Create new policy file
cat <<EOF > $policyfile
action ".*" {
        rule deny { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name" }
}
EOF

#Retrieve action id and update policy
id=`/opt/authz/pap/bin/pap-admin lp -srai | egrep 'id=public' | awk '{print $1}' | sed 's/id=//'`
echo "ID=$id"
/opt/authz/pap/bin/pap-admin up $id $policyfile
if [ $? -eq 0 ]; then
  echo "OK" 
else
  echo "Failed"
  echo "Command run was: /opt/authz/pap/bin/pap-admin up $id $policyfile"
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
  echo "---Test-Update-Policy-From-File: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---Test-Update-Policy-From-File: TEST PASSED---"
  echo `date`
  exit 0
fi

