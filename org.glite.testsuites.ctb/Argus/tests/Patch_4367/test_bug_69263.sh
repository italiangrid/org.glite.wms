#!/bin/sh

script_name=`basename $0`
failed="no"
configfile="/opt/argus/pepd/conf/pepd.ini"

echo "Running: ${script_name}"
echo `date`

/etc/rc.d/init.d/pepd status > /dev/null
if [ $? -ne 0 ]; then
    echo "${script_name}: PEPd is not running. Good."
else
    echo "${script_name}: Stopping PEPd."
    /etc/rc.d/init.d/pepd stop > /dev/null
    sleep 5
fi

# Change the pips section to comment out...

grep pips $configfile
echo "changed to:"
sed -i 's/pips =/# pips =/g' $configfile
grep pips $configfile
# Now try to start pepd.

echo "${script_name}: Starting PEPd."
/etc/rc.d/init.d/pepd start > /dev/null; result=$?;
sleep 5
# echo $result
if [ $result -eq 0 ]
then
    echo "${script_name}: Stopping PEPd."
    /etc/rc.d/init.d/pepd stop > /dev/null
    sleep 5
else
    echo "${script_name}: PEPd failed to start."
    failed="yes"
fi

# Now restore to original

sed -i 's/# pips =/pips =/g' $configfile

# Now try to start pepd.

echo "${script_name}: Starting PEPd."
/etc/rc.d/init.d/pepd start > /dev/null; result=$?;
sleep 5
# echo $result

###############################################################
#clean up

clean_up=0
# clean_up=1

if [ $failed == "yes" ]; then
  echo "---${script_name}: TEST FAILED---"
  echo `date`
  exit 1
else 
  echo "---${script_name}: TEST PASSED---"
  echo `date`
  exit 0
fi

