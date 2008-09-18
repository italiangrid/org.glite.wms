#!/bin/bash
#
# Support file for stress test
# 
# Author: Kalle Happonen <kalle.happonen@cern.ch>

# Registering 100 keys
. $(dirname $0)/functions.sh

setup 

export X509_USER_PROXY=$1
UNI=`mktemp`

i=0
for (( i=0 ; i<=$STRESS_TEST_KEYS_PER_PROC ; i++ )) ; do
    glite-eds-key-register "$UNI""$i"
done

# Unregistering 100 keys

for (( i=0 ; i<=$STRESS_TEST_KEYS_PER_PROC ; i++ )) ; do
    glite-eds-key-unregister "$UNI""$i"
done

rm $UNI
