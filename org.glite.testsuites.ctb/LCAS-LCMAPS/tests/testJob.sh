#!/bin/bash

if [ -a "$X509_USER_PROXY" ]; then
   export X509_USER_PROXY
   echo "SCRIPT SUCCESSFUL"
	echo "X509_USER_PROXY = "
	echo $X509_USER_PROXY
   which glite-version
else
   echo "Could not find user proxy!" 1>&2
   echo "WARNING ... NOT WORKING"
   exit 1
fi

 sleep 100
