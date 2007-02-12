# This is a "multishell" script.

# Test whether gLite job submission commands can be found by the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of glite-job-* ===    "

source `dirname $0`/command-exist.sh glite-job-submit       || exit $?
source `dirname $0`/command-exist.sh glite-job-status	    || exit $?
source `dirname $0`/command-exist.sh glite-job-list-match   || exit $?
source `dirname $0`/command-exist.sh glite-job-cancel	    || exit $?
source `dirname $0`/command-exist.sh glite-job-attach	    || exit $?
source `dirname $0`/command-exist.sh glite-job-output	    || exit $?
source `dirname $0`/command-exist.sh glite-job-logging-info || exit $?
source `dirname $0`/command-exist.sh glite-job-get-chkpt    || exit $?

echo "    ===  Ok  ===    "
