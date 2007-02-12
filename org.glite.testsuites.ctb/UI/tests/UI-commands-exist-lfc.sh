# This is a "multishell" script.

# Test whether the LFC catalog commands can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of lfc-* ===    "

source `dirname $0`/command-exist.sh lfc-chmod        || exit $?
source `dirname $0`/command-exist.sh lfc-chown        || exit $?
source `dirname $0`/command-exist.sh lfc-delcomment   || exit $?
source `dirname $0`/command-exist.sh lfc-getacl       || exit $?
source `dirname $0`/command-exist.sh lfc-ln	      || exit $?
source `dirname $0`/command-exist.sh lfc-ls	      || exit $?
source `dirname $0`/command-exist.sh lfc-mkdir        || exit $?
source `dirname $0`/command-exist.sh lfc-rename       || exit $?
source `dirname $0`/command-exist.sh lfc-rm	      || exit $?
source `dirname $0`/command-exist.sh lfc-setacl       || exit $?
source `dirname $0`/command-exist.sh lfc-setcomment   || exit $?
source `dirname $0`/command-exist.sh lfc-entergrpmap  || exit $?
source `dirname $0`/command-exist.sh lfc-enterusrmap  || exit $?
source `dirname $0`/command-exist.sh lfc-modifygrpmap || exit $?
source `dirname $0`/command-exist.sh lfc-modifyusrmap || exit $?
source `dirname $0`/command-exist.sh lfc-rmgrpmap     || exit $?
source `dirname $0`/command-exist.sh lfc-rmusrmap     || exit $?

echo "    ===  Ok  ===    "
