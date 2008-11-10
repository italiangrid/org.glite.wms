#!/bin/bash
#
# Tests glite-ssss-split-key, glite-ssss-join-key, glite-ssss-split-passwd 
# and glite-ssss-join-passwd commands
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

. $(dirname $0)/functions.sh

if [ $# -ne 0 ] ; then
  usage $0
  if [ $1 = "--help" ] ; then
    my_exit $SUCCESS
  else
    my_exit $ERROR
  fi
fi

setup 

###############################
###### Test for man pages #####
###############################

if [ "$MAN_PAGE_CHECK" -ne 0 ] ; then
  check_man glite-ssss-split-passwd
  check_man glite-ssss-join-passwd
  check_man glite-ssss-split-key
  check_man glite-ssss-join-key
  echo "Man pages found"
fi

###############################
##### Prepare test files ######
###############################

TEMP_FILE=`mktemp`

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $TEMP_FILE ] ; then
    rm -f $TEMP_FILE 
  fi
}


###############################
### Split and join password ###
###############################

glite-ssss-split-passwd 5 3 thebestpw3vah > $TEMP_FILE

if [ $? -ne 0 ] ; then
  echo "Unable to split password" 
  cleanup
  my_exit $ERROR
fi

echo "Password succesfully split"

keypart1=`awk '/^1/ {print $4}' $TEMP_FILE`
keypart3=`awk '/^3/ {print $4}' $TEMP_FILE`
keypart5=`awk '/^5/ {print $4}' $TEMP_FILE`

glite-ssss-join-passwd $keypart1 NULL $keypart3 NULL $keypart5 |grep thebestpw3vah >/dev/null

if [ $? -ne 0 ] ; then
  echo "Unable to join password" 
  cleanup
  my_exit $ERROR
fi

echo "Password succesfully joined"

###############################
##### Split and join key ######
###############################

glite-ssss-split-key 6 4 abcdef1234567890fedc > $TEMP_FILE

if [ $? -ne 0 ] ; then
  echo "Unable to split key"
  cleanup
  my_exit $ERROR
fi

echo "Key succesfully split"

keypart2=`awk '/^2/ {print $4}' $TEMP_FILE`
keypart4=`awk '/^4/ {print $4}' $TEMP_FILE`
keypart5=`awk '/^5/ {print $4}' $TEMP_FILE`
keypart6=`awk '/^6/ {print $4}' $TEMP_FILE`

glite-ssss-join-key NULL $keypart2 NULL $keypart4 $keypart5 $keypart6 |grep bcdef1234567890fedc >/dev/null

if [ $? -ne 0 ] ; then
  echo "Unable to join key" 
  cleanup
  my_exit $ERROR
fi

echo "Key succesfully joined"

###############################
############ Done #############
###############################

cleanup
echo "Test completed"
my_exit $SUCCESS
