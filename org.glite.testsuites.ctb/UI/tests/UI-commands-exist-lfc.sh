#!/bin/bash

# Test whether the LFC catalog commands can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of lfc-* ===    "

. $(dirname $0)/command-exist.sh

command_exist lfc-chmod        || exit $?
command_exist lfc-chown        || exit $?
command_exist lfc-delcomment   || exit $?
command_exist lfc-getacl       || exit $?
command_exist lfc-ln	       || exit $?
command_exist lfc-ls	       || exit $?
command_exist lfc-mkdir        || exit $?
command_exist lfc-rename       || exit $?
command_exist lfc-rm	       || exit $?
command_exist lfc-setacl       || exit $?
command_exist lfc-setcomment   || exit $?
command_exist lfc-entergrpmap  || exit $?
command_exist lfc-enterusrmap  || exit $?
command_exist lfc-modifygrpmap || exit $?
command_exist lfc-modifyusrmap || exit $?
command_exist lfc-rmgrpmap     || exit $?
command_exist lfc-rmusrmap     || exit $?

echo "    ===  Ok  ===    "
