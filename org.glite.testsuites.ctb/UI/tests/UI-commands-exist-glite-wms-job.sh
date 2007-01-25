#!/bin/bash

# Test whether gLite WMS job submission commands can be found by the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

echo "    === Existence test of glite-wms-job-* ===    "

. $(dirname $0)/command-exist.sh

command_exist glite-wms-job-submit          || exit $?
command_exist glite-wms-job-status	    || exit $?
command_exist glite-wms-job-list-match	    || exit $?
command_exist glite-wms-job-cancel	    || exit $?
command_exist glite-wms-job-attach	    || exit $?
command_exist glite-wms-job-output	    || exit $?
command_exist glite-wms-job-logging-info    || exit $?
command_exist glite-wms-job-get-chkpt	    || exit $?
command_exist glite-wms-job-delegate-proxy  || exit $?
command_exist glite-wms-job-perusal         || exit $?

echo "    ===  Ok  ===    "
