#!/bin/sh
#
# Create a directory in LFC, make a simbolic link to it and clean up.
#
# See comments in lfc-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lfc-tests-common.sh

echo ""
echo "    === lfc-ln test ===    "
echo ""

# ... define LFC_HOST TEST_DIR

lfc_test_startup $@

# ... I. Create directory

echo ""; myecho "lfc-mkdir $TEST_DIR"; echo ""
lfc-mkdir $TEST_DIR || exit_failure

TEST_DIR_CREATED="yes"

echo ""; myecho "lfc-ls -d -l $TEST_DIR"; echo ""
lfc-ls -d -l $TEST_DIR || exit_failure

# ... II. Make a symbolic link

LINK_NAME=${TEST_DIR}_link

echo ""; myecho "lfc-ln -s $TEST_DIR $LINK_NAME"; echo ""
lfc-ln -s $TEST_DIR $LINK_NAME || exit_failure

echo ""; myecho "lfc-ls -l $LINK_NAME"; echo ""
lfc-ls -l $LINK_NAME || exit_failure

# ... III. Remove both

echo ""; myecho "lfc-rm $LINK_NAME"; echo ""
lfc-rm $LINK_NAME || exit_failure

echo ""; myecho "lfc-rm -r $TEST_DIR"; echo ""
lfc-rm -r $TEST_DIR || exit_failure

# ... finish

exit_success
