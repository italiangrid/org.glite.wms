#!/bin/sh

if type python &>/dev/null; then
    PYTHON_VER=`python -V 2>&1 | sed -ne '1 {s/Python *//; p}'`
    echo summary: detected python version $PYTHON_VER
    echo "-TEST PASSED-"
    exit 0
else
    echo summary: python not found
    echo "-TEST FAILED-"
    exit 1
fi

