# This is a "multishell" script.

# Test whether the EDG GridFTP commands and globus-url-copy can be found

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$

echo "    === Existence test of edg-gridftp-* ===    "

source `dirname $0`/command-exist.sh edg-gridftp-exists || exit $?
source `dirname $0`/command-exist.sh edg-gridftp-ls     || exit $?
source `dirname $0`/command-exist.sh edg-gridftp-mkdir  || exit $?
source `dirname $0`/command-exist.sh edg-gridftp-rename || exit $?
source `dirname $0`/command-exist.sh edg-gridftp-rm     || exit $?
source `dirname $0`/command-exist.sh edg-gridftp-rmdir  || exit $?
source `dirname $0`/command-exist.sh globus-url-copy    || exit $?

echo "    ===  Ok  ===    "
