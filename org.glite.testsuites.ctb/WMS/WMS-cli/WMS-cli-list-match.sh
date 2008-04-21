#!/bin/sh

###############################################################################
#
# A glite-wms-job-list-match test.
#
# NB: some tests which might be also of some interest are commented out in order
# to reduce the amount of text output (optimization for production environment).
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$
#
###############################################################################

# ... startup

. $(dirname $0)/functions.sh

prepare

# ... print help if requested
if [ "$1" == "--help" ]; then
  echo "Usage: "
  echo ""
  echo $(basename $0) "[-d] [<endpoint>]"
  echo ""
  echo " -d        use glite-wms-delegate-proxy (default behaviour is to use automatic delegation)"
  echo " endpoint  do extra test with --endpoint <endpoint>"
  echo ""
  exit 0
fi

# ... define delegation parameters

if [ "$1" == "-d" ]; then
  DELEGATION_OPTIONS="-d $$"
  myecho "delegating proxy ..."
  run_command glite-wms-job-delegate-proxy $DELEGATION_OPTIONS
  shift
else
  DELEGATION_OPTIONS="-a"
fi

define_delegation

# ... list matched CEs

#run_command glite-wms-job-list-match $DELEGATION_OPTIONS $JDLFILE
#run_command glite-wms-job-list-match $DELEGATION_OPTIONS --rank $JDLFILE
#run_command glite-wms-job-list-match $DELEGATION_OPTIONS --config $CONFIG_FILE $JDLFILE
#run_command glite-wms-job-list-match $DELEGATION_OPTIONS --output $OUTPUTFILE $JDLFILE
#run_command cat $OUTPUTFILE
#rm -f $OUTPUTFILE

run_command glite-wms-job-list-match $DELEGATION_OPTIONS --logfile $LOGFILE $JDLFILE
run_command cat $LOGFILE
rm -f $LOGFILE
echo "" > $LOGFILE

run_command glite-wms-job-list-match $DELEGATION_OPTIONS --noint --rank --config $CONFIG_FILE --output $OUTPUTFILE $JDLFILE
run_command cat $OUTPUTFILE
rm -f $OUTPUTFILE

if [ -n "$1" ]; then
  run_command glite-wms-job-list-match $DELEGATION_OPTIONS --endpoint $1 $JDLFILE
fi

# ... terminate

exit_success
