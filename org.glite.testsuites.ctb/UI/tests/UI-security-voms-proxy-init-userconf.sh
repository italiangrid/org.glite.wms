#!/bin/sh

# This test ensures that voms-proxy-init really uses the files given with the -userconf and -confile options.
# Also tested are -voms and -out options.
# Note that voms-proxy-init 1.6.16 has a bug preventing VOMSes given with -confile to be used
# if such a VO was not defined in the default voms file ${GLITE_LOCATION}/etc/vomses

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

SYSTEM_VOMS_FILE=${GLITE_LOCATION}/etc/vomses
USER_VOMS_FILE=${HOME}/.glite/vomses

TMP_VOMS_FILE=/tmp/vomses.test-voms-proxy_`id -un`

function myexit() {

  rm -f $TMP_VOMS_FILE

  if [ $1 -ne 0 ]; then
    echo ""
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
  myecho "voms-proxy-info -vo succeeded"
  USER_VO=`voms-proxy-info -vo`
fi
if [ -n "$USER_VO" ]; then
  myecho "the user is from \"$USER_VO\""
else
  USER_VO="" # just a dummy text, all VOMS servers will be put in the test file
  myecho "user proxy not found or does not have VO extension - will try all VOMSes"
fi

echo ""
echo "    === Preparations === "

touch $TMP_VOMS_FILE
chmod 644 $TMP_VOMS_FILE
ls -l $TMP_VOMS_FILE

if [ -f ${USER_VOMS_FILE} ]; then

  echo ""
  myecho "user-level VOMS file found: ${USER_VOMS_FILE}"
  myecho " -> adding entries containing $USER_VO content to the test vomses file"
  awk -F ' ' '/[:print:]/ {$1="\"testvoms\""} {print $0}' $USER_VOMS_FILE | grep "$USER_VO" > $TMP_VOMS_FILE

  cat $TMP_VOMS_FILE

elif [ -d ${USER_VOMS_FILE} ]; then

    myecho "the use directory of user-defined voms files is not supported by the current version of this test, sorry"
    myexit 1

fi

if [ -e ${SYSTEM_VOMS_FILE} ]; then

  if [ -f ${SYSTEM_VOMS_FILE} ]; then

    echo ""
    myecho "system-wide VOMS file found: ${SYSTEM_VOMS_FILE}"
    myecho " -> adding entries containing $USER_VO to the test vomses file"
    awk -F ' ' '/[:print:]/ {$1="\"testvoms\""} {print $0}' $SYSTEM_VOMS_FILE | grep "$USER_VO" >> $TMP_VOMS_FILE

  elif [ -d ${SYSTEM_VOMS_FILE} ]; then

    myecho "${SYSTEM_VOMS_FILE} is a directory "
    myecho " -> will scan directory and merge all entries containing $USER_VO into the test voms file"
    cat ${SYSTEM_VOMS_FILE}/* | grep "CN" | awk -F ' ' '/[:print:]/ {$1="\"testvoms\""} {print $0}' /dev/stdin | grep "$USER_VO" >> ${TMP_VOMS_FILE}

  else
    
    myecho "${SYSTEM_VOMS_FILE} is neither file nor directory - exiting"
    myexit 1
    
  fi
  
  cat $TMP_VOMS_FILE

else

  myecho "WARNING: ${SYSTEM_VOMS_FILE} does not exists"
  
fi

echo "    === Part I: -userconf === "

echo ""
myecho "trying  voms-proxy-init -voms testvoms -userconf $TMP_VOMS_FILE -out $TMPPROXY"
voms-proxy-init -voms testvoms  -userconf $TMP_VOMS_FILE  -out $TMPPROXY || myexit $?

echo ""
myecho "trying voms-proxy-info -all -file $TMPPROXY ... "
echo ""
voms-proxy-info -all -file $TMPPROXY || myexit $?

echo ""
myecho "destroying proxy ..."
voms-proxy-destroy -file $TMPPROXY || myexit $?

echo "    --- Part I succeeded --- "
echo ""
echo "    === Part II: -confile === "

echo ""
myecho "trying voms-proxy-init -debug -voms testvoms -confile $TMP_VOMS_FILE -out $TMPPROXY"
voms-proxy-init -debug -voms testvoms  -confile $TMP_VOMS_FILE  -out $TMPPROXY || myexit $?


echo ""
myecho "trying voms-proxy-info -all -file $TMPPROXY ... "
echo ""
voms-proxy-info -all -file $TMPPROXY || myexit $?

echo ""
myecho "destroying proxy ..."
voms-proxy-destroy -file $TMPPROXY || myexit $?

echo "    --- Part II succeeded --- "

myexit 0
