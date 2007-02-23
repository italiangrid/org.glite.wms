# This is a "multishell" script.

# Try gLite job submission commands with --version

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Version test of glite-job-*  ===     "
echo ""

source `dirname $0`/command-version.sh glite-job-submit       --version || exit $?
source `dirname $0`/command-version.sh glite-job-status       --version || exit $?
source `dirname $0`/command-version.sh glite-job-list-match   --version || exit $?
source `dirname $0`/command-version.sh glite-job-cancel       --version || exit $?
source `dirname $0`/command-version.sh glite-job-attach       --version || exit $?
source `dirname $0`/command-version.sh glite-job-output       --version || exit $?
source `dirname $0`/command-version.sh glite-job-logging-info --version || exit $?
source `dirname $0`/command-version.sh glite-job-get-chkpt    --version || exit $?

echo "    ===    seems Ok    ===    "
exit 0
