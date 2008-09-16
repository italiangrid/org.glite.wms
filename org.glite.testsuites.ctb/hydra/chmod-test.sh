#!/bin/bash
#
# Tests glite-eds-chmod command
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
  check_man glite-eds-chmod
  echo "Man page found"
fi

###############################
## Prepare keys and proxies ###
###############################

register_key testkey-1

TEMP_FILE=`mktemp`

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $TEMP_FILE ] ; then
    rm -f $TEMP_FILE 
  fi

  unregister_key testkey-1

}

###############################
#### Test simple +,- and = ####
###############################

glite-eds-chmod og=pdrwlxgs testkey-1

if [ $? -ne 0 ] ; then
  echo "Error setting permissions"
  cleanup
  my_exit $ERROR
fi

get_acl testkey-1 $TEMP_FILE

grep "group pdrwlxgs, other pdrwlxgs" $TEMP_FILE >/dev/null

if [ $? -ne 0 ] ; then
  echo "Permissions incorrect"
  cleanup
  my_exit $ERROR
fi

echo "Privilege setting ok"

glite-eds-chmod a-x testkey-1

if [ $? -ne 0 ] ; then
  echo "Error setting permissions"
  cleanup
  my_exit $ERROR
fi

get_acl testkey-1 $TEMP_FILE

grep "user pdrwl-gs, group pdrwl-gs, other pdrwl-gs" $TEMP_FILE >/dev/null

if [ $? -ne 0 ] ; then
  echo "Permissions incorrect"
  cleanup
  my_exit $ERROR
fi

echo "Privilege removal ok"

glite-eds-chmod u+x testkey-1

if [ $? -ne 0 ] ; then
  echo "Error setting permissions"
  cleanup
  my_exit $ERROR
fi

get_acl testkey-1 $TEMP_FILE

grep "user pdrwlxgs, group pdrwl-gs, other pdrwl-gs" $TEMP_FILE >/dev/null

if [ $? -ne 0 ] ; then
  echo "Permissions incorrect"
  cleanup
  my_exit $ERROR
fi

echo "Privilege adding ok"

###############################
### Test multiple commands ####
###############################

glite-eds-chmod u-x,go=rg testkey-1

if [ $? -ne 0 ] ; then
  echo "Error setting permissions"
  cleanup
  my_exit $ERROR
fi

get_acl testkey-1 $TEMP_FILE

grep "user pdrwl-gs, group --r---g-, other --r---g-" $TEMP_FILE >/dev/null

if [ $? -ne 0 ] ; then
  echo "Permissions incorrect"
  cleanup
  my_exit $ERROR
fi

echo "Multiple operations succeeded"

###############################
## Test if permissions work ###
###############################

if [ "$USE_2_PROXIES" -eq 1 ] ; then
  X509_ORIG=$X509_USER_PROXY
  X509_USER_PROXY=$X509_USER_PROXY_2

  glite-eds-encrypt testkey-1 $TEMP_FILE /dev/null >/dev/null

  if [ $? -ne 0 ] ; then
    echo "Error in usage permissions"
    X509_USER_PROXY=$X509_ORIG
    cleanup
    my_exit $ERROR
  fi

  echo "Permission granting for others works"

  X509_USER_PROXY=$X509_ORIG
  glite-eds-chmod og-rg testkey-1

  if [ $? -ne 0 ] ; then
    echo "Error setting permissions"
    cleanup
    my_exit $ERROR
  fi

  X509_USER_PROXY=$X509_USER_PROXY_2
  glite-eds-encrypt testkey-1 $TEMP_FILE /dev/null >/dev/null 2>&1 

  if [ $? -eq 0 ] ; then
    echo "Error enforcing chmod permissions"
    X509_USER_PROXY=$X509_ORIG
    cleanup
    my_exit $ERROR
  fi

  echo "Permission enforcing for others works"

  X509_USER_PROXY=$X509_ORIG
fi

###############################
############ Done #############
###############################

cleanup
echo "Tests completed"
my_exit $SUCCESS
