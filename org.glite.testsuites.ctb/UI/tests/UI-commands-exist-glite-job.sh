#!/bin/bash

# Test whether gLite job submission commands can be found by the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of glite-job-* ===    "

. $(dirname $0)/command-exist.sh

command_exist glite-job-submit       || exit $?
command_exist glite-job-status	     || exit $?
command_exist glite-job-list-match   || exit $?
command_exist glite-job-cancel	     || exit $?
command_exist glite-job-attach	     || exit $?
command_exist glite-job-output	     || exit $?
command_exist glite-job-logging-info || exit $?
command_exist glite-job-get-chkpt    || exit $?

echo "    ===  Ok  ===    "
