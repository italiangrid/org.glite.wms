#!/bin/bash
#
# Tests glite-eds-put, glite-eds-get and glite-eds-rm commands
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
  check_man glite-eds-put
  check_man glite-eds-get
  check_man glite-eds-rm
  echo "Man pages found"
fi

###############################
###### Prepare testfiles ######
###############################

ORIG_FILE=`mktemp`
RETR_FILE=`mktemp`
ENC_FILE=`mktemp`

cat >> $ORIG_FILE <<EOF
A quick brown fox
jumps over the
lazy dog
EOF

lfc-ls $LFC_TEST_DIR >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "LFC path already exists. Please choose another path"
  rm $ORIG_FILE $RETR_FILE $ENC_FILE
  my_exit $ERROR
fi

lfc-mkdir $LFC_TEST_DIR
if [ $? -ne 0 ] ; then
  echo "Could not create lfc path"
  rm $ORIG_FILE $RETR_FILE $ENC_FILE
  my_exit $ERROR
fi

export LFC_HOME=$LFC_TEST_DIR

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $ORIG_FILE ] ; then
    rm -f $ORIG_FILE 
  fi

  if [ -e $RETR_FILE ] ; then
    rm -f $RETR_FILE
  fi

  if [ -e $ENC_FILE ] ; then
    rm -f $ENC_FILE
  fi
 
  lcg-del -a lfn:testfile1 >/dev/null 2>&1
  lfc-rm -r $LFC_TEST_DIR >/dev/null 2>&1
}

###############################
##### Write and read file #####
###############################

TESTFILEGUID=`glite-eds-put $ORIG_FILE lfn:testfile1 2>/dev/null |grep GUID |cut -f2 -d ':'`

if [ $? -ne 0 ] ; then
  echo "Error in creating file"
  cleanup
  my_exit $ERROR
fi

echo "File encrypted and stored"

glite-eds-get lfn:testfile1 $RETR_FILE >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "Error retrieving file"
  glite-eds-rm lfn:testfile1
  cleanup
  my_exit $ERROR
fi

echo "File retrieved and decrypted"

diff --brief $RETR_FILE $ORIG_FILE >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "Original and retrieved files don't match"
  cleanup
  my_exit $ERROR
fi

echo "Original and retrieved files match"

###############################
### Retrieve encrypted file ###
###############################

lcg-cp lfn:testfile1 file:$ENC_FILE >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "Unable to retrieve (lfc-cp) file"
  glite-eds-rm lfn:testfile1
  cleanup
  my_exit $ERROR
fi

echo "Retrieved encrypted file with lcg tools"

diff --brief $ENC_FILE $ORIG_FILE >/dev/null 2>&1
if [ $? -eq 0 ] ; then
  echo "Original file the same as the encrypted file"
  glite-eds-rm lfn:testfile1
  cleanup
  my_exit $ERROR
fi

echo "Stored encrypted file differs from original file"

echo "" > $RETR_FILE
glite-eds-decrypt $TESTFILEGUID $ENC_FILE $RETR_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Unable to decrypt file"
  cleanup
  glite-eds-rm lfn:testfile1
  my_exit $ERROR
fi

echo "Stored decrypted file matches original file"

diff --brief $RETR_FILE $ORIG_FILE >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "Original and retrieved files don't match"
  glite-eds-rm lfn:testfile1
  cleanup
  my_exit $ERROR
fi

echo "Original file and manually decrypted files match"

glite-eds-rm lfn:testfile1 >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "Error removing file and key"
  cleanup
  my_exit $ERROR
fi

echo "File removal succeeded"


###############################
### Test separating the save ##
###############################

echo "Testing a manual registration process"

glite-eds-key-register testkey1 >/dev/null 2>&1 

if [ $? -ne 0 ] ; then
  echo "Error registering key"
  cleanup
  my_exit $ERROR
fi

echo "Testkey registered"

glite-eds-encrypt testkey1 $ORIG_FILE $ENC_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Unable to encrypt file"
  glite-eds-key-unregister testkey1 >/dev/null 2>&1 
  cleanup
  my_exit $ERROR
fi

echo "File encrypted successfully"

lcg-cr file:$ENC_FILE -l lfn:testfile1 >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Unable to store file in LFC"
  glite-eds-key-unregister testkey1 >/dev/null 2>&1 
  cleanup
  my_exit $ERROR
fi

echo "File stored in LFC"

glite-eds-get -i testkey1 lfn:testfile1 $RETR_FILE >/dev/null 2>&1

if [ $? -ne 0 ] ; then
  echo "Unable to retrieve and decrypt file"
  glite-eds-key-unregister testkey1 >/dev/null 2>&1 
  cleanup
  my_exit $ERROR
fi

echo "Successfully retrieved and decrypted stored file"

diff --brief $RETR_FILE $ORIG_FILE >/dev/null 2>&1
if [ $? -ne 0 ] ; then
  echo "Original and retrieved files don't match"
  glite-eds-key-unregister testkey1 >/dev/null 2>&1 
  cleanup
  my_exit $ERROR
fi

echo "Original file and manually decrypted files match"

glite-eds-key-unregister testkey1 >/dev/null 2>&1 

if [ $? -ne 0 ] ; then
  echo "Error unregistering key"
  cleanup
  my_exit $ERROR
fi

echo "Testkey unregistered" 
###############################
############ Done #############
###############################

cleanup
echo "Tests completed"
my_exit $SUCCESS
