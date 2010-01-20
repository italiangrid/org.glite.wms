#!/bin/sh

#-------------------------------------------------------
# Service Discovery CLI MAN page test. It tests 
# whether a man page exists for glite-sd-query.
#
# Script provided by: Nicholas Loulloudes (UCY)
# Contact: loulloudes.n |_AT_| cs.ucy.ac.cy
#-------------------------------------------------------

name="glite-sd-query"

echo ""

man -P "head | grep $name" $name

echo ""
if [ $? -ne 0 ]; then
  echo -e "glite-sd-query Man page DOES NOT exist."
  echo -e "\n-TEST FAILED-\n"
  exit 155
else
	echo -e "glite-sd-query Man page exists."
	echo -e "\n-TEST PASSED-\n"
	exit 0
fi