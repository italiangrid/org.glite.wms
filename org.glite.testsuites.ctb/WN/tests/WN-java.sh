#!/bin/sh


if type java &>/dev/null; then
    JAVA_VER=`java -version 2>&1 | sed -ne '/java version */{s///; s/"//g; p}'`
    echo summary: detected version $JAVA_VER
    echo "-TEST PASSED-"
    exit 0
else
    echo summary: java not found
    echo "-TEST FAILED-"
    exit 1
fi

