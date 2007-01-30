#!/bin/bash

# This test ensures that voms-proxy-init really uses the file given with the -userconf or -confile option.
# Also tested are -voms and -out options.
# Note that voms-proxy-init ignores the sytem-wide vomses file if a user-level one is found,
# thus both the options can not be tested simultaneousely.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

SYSTEM_VOMS_FILE=${GLITE_LOCATION}/etc/vomses
USER_VOMS_FILE=${HOME}/.glite/vomses

TMP_SYSTEM_VOMS_FILE=/tmp/systemvomses.test-voms-proxy_`id -un`
TMP_USER_VOMS_FILE=/tmp/uservomses.test-voms-proxy_`id -un`

function myexit() {

  rm -f $TMP_SYSTEM_VOMS_FILE
  rm -f $TMP_USER_VOMS_FILE

  if [ $1 -ne 0 ]; then
    echo " *** something went wrong *** "
    echo " *** test NOT passed *** "
    exit $1
  else
    echo ""
    echo "    === test PASSED === "
  fi
   
  exit 0
}

function myecho()
{
  echo "#voms-proxy test# $1"
}

if [ -z "$GLITE_LOCATION" ]; then
   echo "GLITE_LOCATION not defined!"
   exit 1
fi

echo ""
echo "    === voms-proxy-init test === "
TMPPROXY=/tmp/proxy_`id  -u`

echo ""
myecho "trying to guess user's VO ..."
if voms-proxy-info -vo; then
  echo "voms-proxy-info -vo succeded"
  USER_VO=`voms-proxy-info -vo`
fi
if [ -n "$USER_VO" ]; then
  myecho "the user is from $USER_VO"
else
  USER_VO="" # just a dummy text
  myecho "user proxy not found or does not have VO extension - could not guess VO"
fi

if [ -f ${USER_VOMS_FILE} ]; then

  echo ""
  myecho "user-level VOMS file found: ${USER_VOMS_FILE}"
  myecho " -> will test -userconf option"

  echo ""
  myecho "making a slightly modified test copy"
  awk -F ' ' '/[:print:]/ {$1="\"testvoms\""} {print $0}' $USER_VOMS_FILE | grep "$USER_VO" > $TMP_USER_VOMS_FILE

  chmod 644 $TMP_USER_VOMS_FILE 
  cat $TMP_USER_VOMS_FILE

  echo ""
  myecho "trying  voms-proxy-init -voms testvoms -userconf $TMP_USER_VOMS_FILE -out $TMPPROXY"
  voms-proxy-init -voms testvoms  -userconf $TMP_USER_VOMS_FILE  -out $TMPPROXY || myexit $?

elif [ -d ${USER_VOMS_FILE} ]; then

    myecho "the use directory of user-defined voms files is not supported by the current version of this test, sorry"
    myexit 1

elif [ -e ${SYSTEM_VOMS_FILE} ]; then

  if [ -f ${SYSTEM_VOMS_FILE} ]; then

    echo ""
    myecho "system-wide VOMS file found: ${SYSTEM_VOMS_FILE}"
    myecho " -> will test -confile option"
    echo ""
    myecho "making a slightly modified test copy"
    awk -F ' ' '/[:print:]/ {$1="\"testvoms\""} {print $0}' $SYSTEM_VOMS_FILE | grep "$USER_VO" > $TMP_SYSTEM_VOMS_FILE

  elif [ -d ${SYSTEM_VOMS_FILE} ]; then

    myecho "${SYSTEM_VOMS_FILE} is a directory "
    myecho " -> will try to merge its files into a test voms file"
    echo ""
    myecho "making a test voms file"
    cat ${SYSTEM_VOMS_FILE}/* | grep "CN" | awk -F ' ' '/[:print:]/ {$1="\"testvoms\""} {print $0}' /dev/stdin | grep "$USER_VO" > ${TMP_SYSTEM_VOMS_FILE}

  else
    
    myecho "${SYSTEM_VOMS_FILE} is neither file nor directory - exiting"
    myexit 1
    
  fi
  
  chmod 644 $TMP_SYSTEM_VOMS_FILE 
  cat $TMP_SYSTEM_VOMS_FILE

  #echo "making an empty \"voms\" file"
  #echo "" > $TMP_USER_VOMS_FILE

  echo ""
  myecho "trying voms-proxy-init -debug -voms testvoms -confile $TMP_SYSTEM_VOMS_FILE -out $TMPPROXY"
  voms-proxy-init -debug -voms testvoms  -confile $TMP_SYSTEM_VOMS_FILE  -out $TMPPROXY || myexit $?

  chmod 644 $TMP_SYSTEM_VOMS_FILE 
  cat $TMP_SYSTEM_VOMS_FILE
  myexit 1

else

  myecho "neither ${USER_VOMS_FILE} nor ${SYSTEM_VOMS_FILE} exists - VOMS extension can not be tested"
  myexit 1
  
fi

echo ""
myecho "trying voms-proxy-info -all -file $TMPPROXY ... "
echo ""
voms-proxy-info -all -file $TMPPROXY || myexit $?

echo ""
myecho "destroying proxy ..."
voms-proxy-destroy -file $TMPPROXY || myexit $?

myexit 0
