#!/bin/sh
#
# Basic test of lfc-ls.
#
# See comments in lfc-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lfc-tests-common.sh

echo ""
echo "    === lfc-ls test ===    "
echo ""


# ... define LFC_HOST and LFC_DIR

lfc_test_startup $@


# ... Part I. lfc-ls with full path

echo ""; myecho "lfc-ls -d $LFC_DIR"; echo ""
lfc-ls -d $LFC_DIR || exit_failure

echo ""; myecho "lfc-ls -d -l $LFC_DIR"; echo ""
lfc-ls -d -l $LFC_DIR || exit_failure

echo ""; myecho "lfc-ls $LFC_DIR"; echo ""
lfc-ls $LFC_DIR || exit_failure

echo ""; myecho "lfc-ls -l $LFC_DIR"; echo ""
lfc-ls -l $LFC_DIR || exit_failure


# ... Part II. lfc-ls without any path specified, should work if LFC_HOME iscorrectly  set

if [ -n "$LFC_HOME" ]; then

  echo ""; myecho "lfc-ls -d"; echo ""
  lfc-ls -d || exit_failure

  echo ""; myecho "lfc-ls -d -l"; echo ""
  lfc-ls -d -l || exit_failure

  echo ""; myecho "lfc-ls"; echo ""
  lfc-ls || exit_failure

  echo ""; myecho "lfc-ls -l"; echo ""
  lfc-ls -l || exit_failure

fi

# ... finish

exit_success
