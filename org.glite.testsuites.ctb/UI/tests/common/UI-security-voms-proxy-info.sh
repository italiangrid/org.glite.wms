#!/bin/sh

# Test voms-proxy-info with different options.
#
# Note: this test might fail for proxy with no VOMS extension if voms-proxy-info does.
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function myexit() {

  if [ $1 -ne 0 ]; then
    echo ""
    echo " *** something went wrong *** "
    echo " *** test NOT passed *** "
    exit $1
  else
    echo ""
    echo "   === test PASSED === "
  fi
   
  exit 0
}

function myecho()
{
  echo "#voms-proxy-info test# $1"
}

echo "    === Option test of voms-proxy-info === "
echo ""
myecho "voms-proxy-info without options ..."
voms-proxy-info || myexit 1

echo ""
myecho "voms-proxy-info -all ..."
voms-proxy-info -all || myexit 1

echo ""
myecho "voms-proxy-info -debug ..."
voms-proxy-info -debug || myexit 1

echo ""
myecho "voms-proxy-info -text ..."
voms-proxy-info -text || myexit 1

echo ""
myecho "voms-proxy-info -subject ..."
voms-proxy-info -subject || myexit 1

echo ""
myecho "voms-proxy-info -issuer ..."
grid-cert-info -issuer || myexit 1

echo ""
myecho "voms-proxy-info -identity ..."
voms-proxy-info -identity || myexit 1

echo ""
myecho "voms-proxy-info -type ..."
voms-proxy-info -type || myexit 1

echo ""
myecho "voms-proxy-info -timeleft ..."
voms-proxy-info -timeleft || myexit 1

echo ""
myecho "voms-proxy-info -strength ..."
voms-proxy-info -strength || myexit 1

echo ""
myecho "voms-proxy-info -path ..."
voms-proxy-info -path || myexit 1

echo ""
myecho "voms-proxy-info -exists -bits 256 ..."
voms-proxy-info -exists -bits 256 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -bits 512 ..."
voms-proxy-info -exists -bits 512 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -bits 1024 ..."
voms-proxy-info -exists -bits 1024 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 1:00 ..."
voms-proxy-info -exists  -valid 1:00 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 3:00 ..."
voms-proxy-info -exists  -valid 3:00 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 10:00 ..."
voms-proxy-info -exists  -valid 10:00 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -exists -valid 24:00 ..."
voms-proxy-info -exists  -valid 24:00 && echo "YES" || echo "NO"

echo ""
myecho "voms-proxy-info -vo ..."
voms-proxy-info -vo || myexit 1

echo ""
myecho "voms-proxy-info -fqan ..."
voms-proxy-info -fqan || myexit 1

echo ""
myecho "voms-proxy-info -acissuer ..."
voms-proxy-info -acissuer || myexit 1

echo ""
myecho "voms-proxy-info -actimeleft ..."
voms-proxy-info -actimeleft || myexit 1

echo ""
myecho "voms-proxy-info -serial ..."
voms-proxy-info -serial || myexit 1

echo ""
myecho "Analysing output of voms-proxy-info -vo ..."
VO=`voms-proxy-info -vo`

if [ -n "$VO" ]; then

  # ... check for the bug when vo appears twice after a new line
  N=`grep -c "$VO" <<<"$VO"`
  if [ $N -ne 1 ]; then
    echo "VO name \"$VO\" is a malformed"
    myexit 1
  fi

  myecho "voms-proxy-info -acexists $VO ..."
  voms-proxy-info -acexists $VO

  if [ $? -eq 0 ]; then
    echo "YES"
  else 
    echo "NO"
    myexit 1
  fi
fi

echo ""
VO=for_sure_non_existing_VO
myecho "voms-proxy-info -acexists $VO ..."
voms-proxy-info -acexists $VO
if [ $? -eq 0 ]; then
  echo ""
  myecho "ERROR: voms-proxy-info claims $VO exists!"
  myexit 1
else
  echo "Of course not!"
fi

myexit 0
