#!/bin/sh

script_name=`basename $0`
failed="no"
policyfile=policyfile.txt

if [ -z $PAP_HOME ]
then
    if [ -d /opt/argus/pap ]
    then
        export PAP_HOME=/opt/argus/pap
    else
        echo "${script_name}: PAP_HOME cannot be found. Exiting"
        exit 0;
    fi
fi

/etc/rc.d/init.d/pap-standalone status | grep -q 'PAP running'
if [ $? -ne 0 ]; then
  echo "PAP is not running"
  exit 1
fi

#Remove all policies defined for the default pap
$PAP_HOME/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: $PAP_HOME/bin/pap-admin rap"
  exit 1
fi

echo `date`

###############################################################
echo "Running: ${script_name}"

#Store initial policy
cat <<EOF > $policyfile
resource "resource_1" {
    action ".*" {
        rule deny { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=user/CN=999999/CN=user name/testslash" }
    }
}
EOF
$PAP_HOME/bin/pap-admin apf $policyfile
if [ $? -ne 0 ]; then
  echo "Error preparing the test environment"
  echo "Failed command: $PAP_HOME/bin/pap-admin apf $policyfile"
  exit 1
fi

cat <<EOF > $policyfile
resource "resource_1" {
    action ".*" {
        rule allow { subject="/DC=ch/DC=cern/OU=Organic Units/OU=Users/slashtest/CN=user/CN=999999/CN=user name" }
    }
}
EOF

$PAP_HOME/bin/pap-admin apf $policyfile
if [ $? -ne 0 ]; then
  echo "Error preparing the test environment"
  echo "Failed command: $PAP_HOME/bin/pap-admin apf $policyfile"
  exit 1
fi

$PAP_HOME/bin/pap-admin lp --resource "resource_1"

###############################################################
#clean up

clean_up=0
# clean_up=1

if [ $clean_up -eq 0 ]
then
rm -f $policyfile
#Remove all policies defined for the default pap
$PAP_HOME/bin/pap-admin rap
if [ $? -ne 0 ]; then
  echo "Error cleaning the default pap"
  echo "Failed command: $PAP_HOME/bin/pap-admin rap"
  exit 1
fi
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

