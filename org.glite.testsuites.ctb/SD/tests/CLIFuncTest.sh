#!/bin/bash

#-------------------------------------------------------
# Service Discovery CLI tool function test. It tests 
# whether given a specific gLite Service, the correct
# output will be given.
#
# Script provided by: Nicholas Loulloudes (UCY)
# Contact: loulloudes.n |_AT_| cs.ucy.ac.cy
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