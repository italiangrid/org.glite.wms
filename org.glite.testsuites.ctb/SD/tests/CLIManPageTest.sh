#!/bin/sh

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
# Service Discovery CLI MAN page test. It tests 
# whether a man page exists for glite-sd-query.
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