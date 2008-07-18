#!/bin/sh

PREFIX=$1
STAGEDIR=$2

mkdir -p $STAGEDIR/share/man/man1
for item in `ls src/cream-test-*`; do $item -h >$STAGEDIR/share/man/man1/`basename $item`.1; done

mkdir -p $STAGEDIR/lib/python/CREAMTestUtils
cp src/CREAMTestUtils/*.py src/CREAMTestUtils/*.pyc $STAGEDIR/lib/python/CREAMTestUtils

mkdir -p $STAGEDIR/bin
cp src/cream-test-* $STAGEDIR/bin



mkdir -p $PREFIX/lib/python/CREAMTestUtils
cp src/CREAMTestUtils/*.py src/CREAMTestUtils/*.pyc $PREFIX/lib/python/CREAMTestUtils

mkdir -p $PREFIX/bin
cp src/cream-test-* $PREFIX/bin

mkdir -p $PREFIX/share/man/man1
cp $STAGEDIR/share/man/man1/cream-test*.1 $PREFIX/share/man/man1


