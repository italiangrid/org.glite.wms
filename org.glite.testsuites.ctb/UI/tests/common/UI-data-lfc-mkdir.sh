#!/bin/sh
#
# Create a directory in LFC, list it and remove.
#
# See comments in lfc-tests-common.sh for explanation of command line options.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

. $(dirname $0)/lfc-tests-common.sh

echo ""
echo "    === lfc-mkdir test ===    "
echo ""

# ... define LFC_HOST TEST_DIR

lfc_test_startup $@

# ... I. Create directory

echo ""; myecho "lfc-mkdir $TEST_DIR"; echo ""
lfc-mkdir $TEST_DIR || exit_failure

TEST_DIR_CREATED="yes"

# ... II. List directory

echo ""; myecho "lfc-ls -d $TEST_DIR"; echo ""
OUTPUT=`lfc-ls -d $TEST_DIR`

if [ $? -ne 0 ]; then
  echo $OUTPUT
  myecho "Problem listing directory $TEST_DIR"
  exit_failure
fi

echo $OUTPUT

if grep -q $RELATIVE_NAME <<<"$OUTPUT"; then
  echo ""
  myecho "directory has been successfully created"
else
  myecho "Error: created directory is not found with lfc-ls -d !"
  exit_failure
fi

echo ""; myecho "lfc-ls -l -d $TEST_DIR"; echo ""
lfc-ls -l -d $TEST_DIR || exit_failure

# ... III. Remove directory

echo ""; myecho "lfc-rm -r $TEST_DIR"; echo ""
lfc-rm -r $TEST_DIR || exit_failure

exit_success
