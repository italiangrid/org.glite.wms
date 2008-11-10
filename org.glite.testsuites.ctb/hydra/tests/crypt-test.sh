#!/bin/bash
#
# Tests glite-eds-encrypt and glite-eds-decrypt commands
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
  check_man glite-eds-encrypt
  check_man glite-eds-decrypt
  echo "Man pages found"
fi

###############################
## Prepare key and testfile ###
###############################

register_key testkey-1

if [ $? -ne 0 ] ; then
  echo "Error registering key"
  my_exit $ERROR
fi

ORIG_FILE=`mktemp`
ENCRYPT_FILE=`mktemp`
DECRYPT_FILE=`mktemp`

cat >> $ORIG_FILE <<EOF
A quick brown fox
jumps over the
lazy dog
EOF

###############################
########### Cleanup ###########
###############################

function cleanup () {
  if [ -e $ORIG_FILE ] ; then
    rm -f $ORIG_FILE 
  fi

  if [ -e $ENCRYPT_FILE ] ; then
    rm -f $ENCRYPT_FILE
  fi
 
  if [ -e $DECRYPT_FILE ] ; then
    rm -f $DECRYPT_FILE
  fi

  unregister_key testkey-1
}

###############################
#### Crypt and verify file ####
###############################

glite-eds-encrypt testkey-1 $ORIG_FILE $ENCRYPT_FILE >/dev/null

if [ $? -ne 0 ] ; then
  echo "Error encrypting file"
  cleanup
  my_exit $ERROR
fi

diff --brief $ORIG_FILE $ENCRYPT_FILE >/dev/null

if [ $? -ne 1 ] ; then
  echo "Encrypted file and original file identical"
  cleanup
  my_exit $ERROR
fi

echo "File encryption ok"

glite-eds-decrypt testkey-1 $ENCRYPT_FILE $DECRYPT_FILE >/dev/null

if [ $? -ne 0 ] ; then
  echo "Error decrypting file"
  cleanup
  my_exit $ERROR
fi

diff --brief $ORIG_FILE $DECRYPT_FILE >/dev/null

if [ $? -ne 0 ] ; then
  echo "Decrypted file and original file differ"
  cleanup
  my_exit $ERROR
fi

echo "File decryption ok"

###############################
############ Done #############
###############################

cleanup
echo "Tests completed"
my_exit $SUCCESS
