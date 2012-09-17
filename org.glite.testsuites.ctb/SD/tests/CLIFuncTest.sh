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
# Service Discovery CLI tool function test. It tests 
# whether given a specific gLite Service, the correct
# output will be given.
#-------------------------------------------------------

#-------------------------------------------------------
# Default location of the SD CLI tool
#-------------------------------------------------------

sdFile="/opt/glite/bin/glite-sd-query"

#-------------------------------------------------------
# Set the necessary environmental variables.
#-------------------------------------------------------

export GLITE_SD_PLUGIN="bdii,rgma,file"
export GLITE_SD_SITE="CY-01-KIMON"
export LCG_GFAL_INFOSYS="bdii101.grid.ucy.ac.cy:2170"


#-------------------------------------------------------
# Set the type of Service to Query.
#-------------------------------------------------------

serviceType="org.glite.wms.WMProxy"

#-------------------------------------------------------
# Command to run.
#-------------------------------------------------------

cmd="$sdFile -t $serviceType -x"

$cmd