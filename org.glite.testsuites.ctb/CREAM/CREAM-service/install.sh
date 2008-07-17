#!/bin/sh

PREFIX=$1
STAGEDIR=$2

mkdir -p $STAGEDIR/lib/python
cp -R src/CREAMTestUtils $STAGEDIR/lib/python

mkdir -p $STAGEDIR/bin
cp src/cream-test-* $STAGEDIR/bin

mkdir -p $STAGEDIR/share/man/man1
for item in `ls src/cream-test-*`; do $item -h >$STAGEDIR/share/man/man1/`basename $item`.1; done



mkdir -p $PREFIX/lib/python
cp -R src/CREAMTestUtils $PREFIX/lib/python

mkdir -p $PREFIX/bin
cp src/cream-test-* $PREFIX/bin

mkdir -p $PREFIX/share/man/man1
cp $STAGEDIR/share/man/man1/cream-test*.1 $PREFIX/share/man/man1


