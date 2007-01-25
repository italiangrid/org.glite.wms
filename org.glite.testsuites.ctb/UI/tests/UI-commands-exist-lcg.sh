#!/bin/bash

# Test whether the LCG data management tools can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of lcg-* ===    "

. $(dirname $0)/command-exist.sh

command_exist lcg-cp  || exit $?
command_exist lcg-cr  || exit $?
command_exist lcg-del || exit $?
command_exist lcg-rep || exit $?
command_exist lcg-gt  || exit $?
command_exist lcg-sd  || exit $?

command_exist lcg-aa  || exit $?
command_exist lcg-ra  || exit $?
command_exist lcg-rf  || exit $?
command_exist lcg-uf  || exit $?
command_exist lcg-la  || exit $?
command_exist lcg-lg  || exit $?
command_exist lcg-lr  || exit $?

#command_exist lcg-fetch           || exit $?
#command_exist lcg-info		  || exit $?
#command_exist lcg-infosites	  || exit $?
#command_exist lcg-job-monitor	  || exit $?
#command_exist lcg-job-status	  || exit $?
#command_exist lcg-ManageSoftware  || exit $?
#command_exist lcg-ManageVOTag	  || exit $?
#command_exist lcg-mon-wn	  || exit $?
#command_exist lcg-replica-manager || exit $?
#command_exist lcg-tags		  || exit $?
#command_exist lcg-version         || exit $?
#command_exist lcg-wn-os 	  || exit $?

echo "    ===  Ok  ===    "
