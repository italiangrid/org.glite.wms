#!/bin/bash

# Test whether EDG job submission commands can be found by the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of edg-job-* ===    "

. $(dirname $0)/command-exist.sh

command_exist edg-job-submit           || exit $?
command_exist edg-job-status           || exit $?
command_exist edg-job-list-match       || exit $?
command_exist edg-job-cancel           || exit $?
command_exist edg-job-attach           || exit $?
command_exist edg-job-get-output       || exit $?
command_exist edg-job-get-logging-info || exit $?
command_exist edg-job-get-chkpt        || exit $?

echo "    ===  Ok  ===    "
