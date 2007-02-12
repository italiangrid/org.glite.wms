# This is a "multishell" script.

# Test whether the gLite FTS commands can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of glite-transfer-* ===    "

source `dirname $0`/command-exist.sh glite-transfer-submit || exit $?
source `dirname $0`/command-exist.sh glite-transfer-status || exit $?
source `dirname $0`/command-exist.sh glite-transfer-list   || exit $?
source `dirname $0`/command-exist.sh glite-transfer-cancel || exit $?

source `dirname $0`/command-exist.sh glite-transfer-channel-add    || exit $?
source `dirname $0`/command-exist.sh glite-transfer-channel-list   || exit $?
source `dirname $0`/command-exist.sh glite-transfer-channel-set    || exit $?
source `dirname $0`/command-exist.sh glite-transfer-channel-signal || exit $?

#source `dirname $0`/command-exist.sh glite-transfer-addvomanager     || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-discovery        || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-getroles         || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-listvomanagers   || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-removevomanager  || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-service-info     || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-setpriority      || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-stats-agents     || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-stats-channel    || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-stats-vo         || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-submit-placement || exit $?

#source `dirname $0`/command-exist.sh glite-transfer-channel-addmanager    || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-channel-drop          || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-channel-listmanagers  || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-channel-removemanager || exit $?
#source `dirname $0`/command-exist.sh glite-transfer-channel-setvoshare    || exit $?

echo "    ===  Ok  ===    "
