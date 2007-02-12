# This is a "multishell" script.

# Test whether EDG job submission commands can be found by the system

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of edg-job-* ===    "

source `dirname $0`/command-exist.sh edg-job-submit           || exit $?
source `dirname $0`/command-exist.sh edg-job-status           || exit $?
source `dirname $0`/command-exist.sh edg-job-list-match       || exit $?
source `dirname $0`/command-exist.sh edg-job-cancel           || exit $?
source `dirname $0`/command-exist.sh edg-job-attach           || exit $?
source `dirname $0`/command-exist.sh edg-job-get-output       || exit $?
source `dirname $0`/command-exist.sh edg-job-get-logging-info || exit $?
source `dirname $0`/command-exist.sh edg-job-get-chkpt        || exit $?

echo "    ===  Ok  ===    "
