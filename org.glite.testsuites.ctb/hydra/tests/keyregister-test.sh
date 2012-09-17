#!/bin/bash
#
# Tests glite-eds-key-register and glite-eds-key-unregister commands
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
  check_man glite-eds-key-register
  check_man glite-eds-key-unregister
  echo "Man pages found"
fi

###############################
##### Normal (un)register #####
###############################

register_key testkey-1

echo "Key succesfully registered"

# Test registering an existing key
glite-eds-key-register testkey-1 >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "No error in registering an existing key"
  unregister_key testkey-1
  my_exit $ERROR
fi

echo "Registering existing key gives error"

unregister_key testkey-1

echo "Key succesfully unregistered"

# Test unregistering a nonexistent key
glite-eds-key-unregister testkey-1 >/dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo "No error in unregistering a nonexistent key"
  my_exit $ERROR
fi

echo "Removing nonexistent key gives error"

###############################
#### Specify cipher + key #####
###############################

# Test registering a key with cipher and keylength
glite-eds-key-register -c aes-256-cbc -k 128 testkey-1 >/dev/null
if [ $? -ne 0 ] ; then
  echo "Error registering key with cipher and key length"
  my_exit $ERROR
fi

echo "Key with specified cipher and keylength succesfully registered"

unregister_key testkey-1

###############################
############ Done #############
###############################

echo "Tests completed"
my_exit $SUCCESS
