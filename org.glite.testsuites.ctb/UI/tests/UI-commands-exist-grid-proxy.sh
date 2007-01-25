#!/bin/bash

# Test whether the grid-proxy-* and grid-cert-info commands exist in the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of grid-proxy-* and grid-cert-info ===    "

. $(dirname $0)/command-exist.sh

command_exist grid-proxy-init    || exit $?
command_exist grid-proxy-info    || exit $?
command_exist grid-proxy-destroy || exit $?
command_exist grid-cert-info     || exit $?

echo "    ===  Ok  ===    "
