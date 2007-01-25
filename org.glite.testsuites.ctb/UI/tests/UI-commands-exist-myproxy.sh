#!/bin/bash

# Test whether MyProxy commands can be found by the OS

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of myproxy-* ===    "

. $(dirname $0)/command-exist.sh

command_exist myproxy-init               || exit $?
command_exist myproxy-info		 || exit $?
command_exist myproxy-destroy		 || exit $?
command_exist myproxy-server		 || exit $?
command_exist myproxy-get-delegation	 || exit $?
command_exist myproxy-change-pass-phrase || exit $?

echo "    ===  Ok  ===    "
