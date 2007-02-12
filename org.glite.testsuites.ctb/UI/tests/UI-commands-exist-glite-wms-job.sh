# This is a "multishell" script.

# Test whether gLite WMS job submission commands can be found by the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of glite-wms-job-* ===    "

source `dirname $0`/command-exist.sh glite-wms-job-submit         || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-status	  || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-list-match	  || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-cancel	  || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-attach	  || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-output	  || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-logging-info   || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-get-chkpt      || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-delegate-proxy || exit $?
source `dirname $0`/command-exist.sh glite-wms-job-perusal        || exit $?

echo "    ===  Ok  ===    "
