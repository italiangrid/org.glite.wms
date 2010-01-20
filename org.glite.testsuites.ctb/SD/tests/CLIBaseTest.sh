#!/bin/bash

#
#  Copyright (c) Members of the EGEE Collaboration. 2009.
#  See http://public.eu-egee.org/partners/ for details on the
#  copyright holders.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

#-------------------------------------------------------
# Service Discovery CLI tool and environmental variable
# existence test script.
#-------------------------------------------------------


#-------------------------------------------------------
# First part of the test is to check
# if the glite-sd-query CLI tool exists in the middleware.
#-------------------------------------------------------

#default location of the SD CLI tool/
sdFileLocation="/opt/glite/bin/glite-sd-query"

testPassed=0;

if [ -e "$sdFileLocation" ]; then
	echo -e "\nThe glite-sd-query command line tool EXIST in location: $sdFileLocation\n"
	testPassed=1;
else
	echo -e "\nThe glide-sd-query command line tool DOES NOT EXIST\n."
fi

#-------------------------------------------------------
# Test if the necessary environmental variables are set.
#-------------------------------------------------------

echo "Testing if necessary environmental variables are set..."

if [ -z "$GLITE_SD_PLUGIN" ]; then
  echo -e "\t GLITE_SD_PLUGIN is NOT SET."
  testPassed=0;
else
  echo -e "\t GLITE_SD_PLUGIN is SET."
fi

if [ -z "$GLITE_SD_SITE" ]; then
  echo -e "\t GLITE_SD_SITE is NOT SET."
  testPassed=0;
else
  echo -e "\t GLITE_SD_SITE is SET."
fi

if [ -z "$LCG_GFAL_INFOSYS" ]; then
  echo -e "\t LCG_GFAL_INFOSYS is NOT SET."
  testPassed=0;
else
  echo -e "\t LCG_GFAL_INFOSYS is SET."
fi


#-------------------------------------------------------
# Exit with status = 0 if -TEST PASSED-
# Exit with status = 155 if -TEST FAILED-
#-------------------------------------------------------

if [ "$testPassed" -eq 1 ]; then
  echo -e "\n-TEST PASSED-\n"
  exit 0
else
  echo -e "\n-TEST FAILED-\n"
  exit 155
fi




