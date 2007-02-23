# This is a "multishell" script.

# Try gLite WMS job submission commands with --version

# Please make sure that you have a valid proxy with VO extension
# and the certificate of the used VOMS server is installed in your system.
# Otherwise the test may fail.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of glite-wms-job-*  ===     "
echo ""

echo " -- this test requires valid proxy, so checking proxy first --"

voms-proxy-info -all || (echo "Error! No proxy valid for this operation found."; echo "Please create a proxy with voms-proxy-init -voms <yourvo> and retry the test."; false) || exit $?

echo ""

# ... these are "real" glite-wms-job-* commands
# ... require a valid proxy to get server version

source `dirname $0`/command-version.sh glite-wms-job-submit	    --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-list-match     --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-cancel	    --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-output	    --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-delegate-proxy --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-perusal	    --version || exit $?

# ... these are normally symbolic links to the corresponding glite-job-* commands

source `dirname $0`/command-version.sh glite-wms-job-status	  --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-attach	  --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-logging-info --version || exit $?
source `dirname $0`/command-version.sh glite-wms-job-get-chkpt	  --version || exit $?

echo "    ===    seems Ok    ===    "
exit 0
