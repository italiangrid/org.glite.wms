#!/bin/bash

# Test whether the EDG GridFTP commands and globus-url-copy can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of edg-gridftp-* ===    "

. $(dirname $0)/command-exist.sh

command_exist edg-gridftp-exists || exit $?
command_exist edg-gridftp-ls     || exit $?
command_exist edg-gridftp-mkdir  || exit $?
command_exist edg-gridftp-rename || exit $?
command_exist edg-gridftp-rm     || exit $?
command_exist edg-gridftp-rmdir  || exit $?
command_exist globus-url-copy    || exit $?

echo "    ===  Ok  ===    "
