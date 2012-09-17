#!/bin/sh

SAME_OK=0
SAME_ERROR=1
SAME_WARNING=2

. lib.sh

# get version
if type glite-version; then
    VER=`glite-version`
elif type lcg-version; then
    VER=`lcg-version`
else
    exit_with_summary $SAME_ERROR "version could not be determined"
fi

# check version
case "$VER" in
    2.*) ;;
    3.*) ;;
    *) exit_with_summary $SAME_ERROR "unknown version: $VER" ;;
esac

# check version in IS
CE=`ce`
if [ ! "$CE" ]; then
    exit_with_summary $SAME_WARNING "could not get CE name"
fi

# get the hostname
CE=${CE%%/*} # strip queue name
CE=${CE%%:*} # strip port number

# get the latest version published in the IS
ATTR="GlueHostApplicationSoftwareRunTimeEnvironment"
IS_VER=`ldapsearch -LLL -H ldap://$LCG_GFAL_INFOSYS -x -b o=grid \
            "(&(objectClass=GlueHostApplicationSoftware)
            (GlueChunkKey=GlueClusterUniqueID=$CE))" $ATTR | \
        sed -n -e "/^$ATTR: "'\(LCG\|GLITE\)-\([2-9]\)/s//\2/p' | \
        sort | tail -1`

# TODO: timeout

if [ ! "$IS_VER" ]; then
    exit_with_summary $SAME_WARNING "could not get version from IS"
fi

# strip the last digit
IS_VER=${IS_VER%_*}
IS_VER=${IS_VER//_/.}
VER_SHORT=${VER%.*}

if [ "$VER_SHORT" != "$IS_VER" ]; then
    exit_with_summary $SAME_ERROR "versions differ: WN=$VER_SHORT IS=$IS_VER"
fi

exit_with_summary $SAME_OK "$VER"

