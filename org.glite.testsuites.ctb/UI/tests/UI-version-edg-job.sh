# This is a "multishell" script.

# Try gLite EDG job submission commands with --version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of edg-job-*  ===     "
echo ""

source `dirname $0`/command-version.sh edg-job-submit	        --version || exit $?
source `dirname $0`/command-version.sh edg-job-status	        --version || exit $?
source `dirname $0`/command-version.sh edg-job-list-match       --version || exit $?
source `dirname $0`/command-version.sh edg-job-cancel	        --version || exit $?
source `dirname $0`/command-version.sh edg-job-attach	        --version || exit $?
source `dirname $0`/command-version.sh edg-job-get-output       --version || exit $?
source `dirname $0`/command-version.sh edg-job-get-logging-info --version || exit $?
source `dirname $0`/command-version.sh edg-job-get-chkpt        --version || exit $?

echo "    ===    seems Ok    ===    "
exit 0
