#!/bin/bash

# Test whether the VOMS proxy commands can be found by the OS

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of voms-proxy-* ===    "

. $(dirname $0)/command-exist.sh

command_exist voms-proxy-init     || exit $?
command_exist voms-proxy-destroy  || exit $?
command_exist voms-proxy-info	  || exit $?
command_exist voms-proxy-list	  || exit $?
command_exist voms-proxy-fake	  || exit $?

echo "    ===  Ok  ===    "

