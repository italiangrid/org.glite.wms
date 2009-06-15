#!/bin/sh

if type python &>/dev/null; then
    PERL_VER=`perl -v 2>&1 | sed -ne '/.*perl, v/{s///; s/built.*//p}'`
    echo "summary: detected perl version $PERL_VER"
    echo "-TEST PASSED-"
    exit 0
else
    echo "summary: perl not found"
    echo "-TEST FAILED-"
    exit 1
fi
