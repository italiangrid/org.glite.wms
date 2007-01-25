#!/bin/bash

# Test whether the gLite FTS commands can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of glite-transfer-* ===    "

. $(dirname $0)/command-exist.sh

command_exist glite-transfer-submit || exit $?
command_exist glite-transfer-status || exit $?
command_exist glite-transfer-list   || exit $?
command_exist glite-transfer-cancel || exit $?

command_exist glite-transfer-channel-add    || exit $?
command_exist glite-transfer-channel-list   || exit $?
command_exist glite-transfer-channel-set    || exit $?
command_exist glite-transfer-channel-signal || exit $?

#command_exist glite-transfer-addvomanager     || exit $?
#command_exist glite-transfer-discovery        || exit $?
#command_exist glite-transfer-getroles         || exit $?
#command_exist glite-transfer-listvomanagers   || exit $?
#command_exist glite-transfer-removevomanager  || exit $?
#command_exist glite-transfer-service-info     || exit $?
#command_exist glite-transfer-setpriority      || exit $?
#command_exist glite-transfer-stats-agents     || exit $?
#command_exist glite-transfer-stats-channel    || exit $?
#command_exist glite-transfer-stats-vo         || exit $?
#command_exist glite-transfer-submit-placement || exit $?

#command_exist glite-transfer-channel-addmanager    || exit $?
#command_exist glite-transfer-channel-drop          || exit $?
#command_exist glite-transfer-channel-listmanagers  || exit $?
#command_exist glite-transfer-channel-removemanager || exit $?
#command_exist glite-transfer-channel-setvoshare    || exit $?

echo "    ===  Ok  ===    "
