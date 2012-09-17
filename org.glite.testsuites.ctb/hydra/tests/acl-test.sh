#!/bin/bash
#
# Tests glite-eds-setacl and glite-eds-getacl commands
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
  check_man glite-eds-setacl
  check_man glite-eds-getacl
fi

###############################
## Prepare keys and proxies ###
###############################

if [ "$USE_2_PROXIES" -eq 1 ] ; then
  ACL_TWO=`voms-proxy-info -file $X509_USER_PROXY_2 -identity`
else
  ACL_TWO="/C=ch/O=Test organisation/CN=Test user"
fi

export USE_TWO
export ACL_TWO
export ACL_THREE="/C=ch/O=Test organisation/CN=Real user"

register_key testkey-1

TEMP_FILE=`mktemp`
ACL_FILE=`mktemp`

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $TEMP_FILE ] ; then
    rm -f $TEMP_FILE 
  fi

  if [ -e $ACL_FILE ] ; then
    rm -f $ACL_FILE 
  fi

  unregister_key testkey-1
}

###############################
#### ACL modify with file  ####
###############################

# Set ACL with a file
echo "$ACL_TWO:rwx" > $ACL_FILE
echo "$ACL_THREE:pld" >> $ACL_FILE

set_acl -M "$ACL_FILE" testkey-1

get_acl testkey-1 $TEMP_FILE

grep "$ACL_TWO:--rw-x--" $TEMP_FILE >/dev/null
RET1=$?
grep "$ACL_THREE:pd--l---" $TEMP_FILE >/dev/null
RET2=$?

if [ "$RET1" -ne 0 ] || [ "$RET2" -ne 0 ] ; then
  echo "Permissions incorrect after modifying from file"
  cat $TEMP_FILE
  cleanup
  my_exit $ERROR
fi

echo "ACL permissions successfully modified from file"

###############################
###### Simple ACL modify ######
###############################

set_acl -m "$ACL_TWO:pdrwlxgs" testkey-1

get_acl testkey-1 $TEMP_FILE

grep "$ACL_TWO:pdrwlxgs" $TEMP_FILE >/dev/null
RET1=$?
grep "$ACL_THREE:pd--l---" $TEMP_FILE >/dev/null
RET2=$?

if [ "$RET1" -ne 0 ] || [ "$RET2" -ne 0 ] ; then
  echo "Permissions incorrect after modifying from command line"
  cat $TEMP_FILE
  cleanup
  my_exit $ERROR
fi

echo "ACL permissions successfully modified from command line"

###############################
###### ACL set with file ######
###############################

echo "$ACL_TWO:gs" > $ACL_FILE

set_acl -X "$ACL_FILE" testkey-1

get_acl testkey-1 $TEMP_FILE

grep "$ACL_TWO:------gs" $TEMP_FILE >/dev/null
RET1=$?
grep "$ACL_THREE:pd--l---" $TEMP_FILE >/dev/null
RET2=$?

if [ "$RET1" -ne 0 ] || [ "$RET2" -eq 0 ] ; then
  echo "Permissions incorrect after setting from file"
  cat $TEMP_FILE
  cleanup
  my_exit $ERROR
fi

echo "ACL permissions succesully set from file"

###############################
######## Simple ACL set #######
###############################

set_acl -x "NONE" testkey-1

get_acl testkey-1 $TEMP_FILE

grep -v ^# $TEMP_FILE
RET1=$?

if [ "$RET1" -eq 0 ] ; then
  echo "Permissions incorrect after setting from command line"
  cat $TEMP_FILE
  cleanup
  my_exit $ERROR
fi

echo "ACL permissions succesully set from command line"

###############################
###### Simple ACL delete ######
###############################

# Set ACL with a file
echo "$ACL_TWO:rwx" > $ACL_FILE
echo "$ACL_THREE:pld" >> $ACL_FILE

set_acl  -M "$ACL_FILE" testkey-1

set_acl -d "$ACL_TWO" testkey-1

get_acl testkey-1 $TEMP_FILE

grep "$ACL_TWO:--rw-x--" $TEMP_FILE >/dev/null
RET1=$?
grep "$ACL_THREE:pd--l---" $TEMP_FILE >/dev/null
RET2=$?

if [ "$RET1" -eq 0 ] || [ "$RET2" -ne 0 ] ; then
  echo "Permissions incorrect after deleting from command line"
  cat $TEMP_FILE
  cleanup
  my_exit $ERROR
fi

echo "ACL permissions succesully deleted from command line"

###############################
#### ACL delete with file #####
###############################

echo "$ACL_THREE:" > $ACL_FILE

set_acl -D "$ACL_FILE" testkey-1

get_acl testkey-1 $TEMP_FILE

grep -v "^#" $TEMP_FILE >/dev/null
RET1=$?

if [ "$RET1" -eq 0 ] ; then
  echo "Permissions incorrect after deleting from file"
  cat $TEMP_FILE
  cleanup
  my_exit $ERROR
fi

echo "ACL permissions succesully deleted from command file"

###############################
### Test if ACL is enforced ###
###############################

if [ "$USE_2_PROXIES" -eq 1 ] ; then  
  set_acl -x "$ACL_TWO":pdrwlxgs testkey-1

  X509_ORIG=$X509_USER_PROXY
  X509_USER_PROXY=$X509_USER_PROXY_2

  glite-eds-encrypt testkey-1 $TEMP_FILE /dev/null >/dev/null

  if [ $? -ne 0 ] ; then
    echo "Error in using ACL permissions"
    X509_USER_PROXY=$X509_ORIG
    cleanup
    my_exit $ERROR
  fi
  
  echo "ACL permissions succesfully granted and used"

  set_acl -x "$ACL_TWO": testkey-1

  glite-eds-encrypt testkey-1 $TEMP_FILE /dev/null >/dev/null 2>&1

  if [ $? -eq 0 ] ; then
    echo "Error enforcing ACL permissions"
    X509_USER_PROXY=$X509_ORIG
    cleanup
    my_exit $ERROR
  fi

  echo "ACL modification permissions succesfully enforced"

  X509_USER_PROXY=$X509_ORIG

  set_acl -x "$ACL_TWO":d testkey-1

  X509_USER_PROXY=$X509_USER_PROXY_2

  glite-eds-key-unregister testkey-1 >/dev/null

  if [ $? -ne 0 ] ; then
    echo "Error enforcing acl"
    X509_USER_PROXY=$X509_ORIG
    cleanup
    my_exit $ERROR
  fi

  echo "ACL deletion permissions succesfully enforced"

  X509_USER_PROXY=$X509_ORIG

  register_key testkey-1
fi

###############################
############ Done #############
###############################

cleanup
echo "Tests completed - no errors"
my_exit $SUCCESS
