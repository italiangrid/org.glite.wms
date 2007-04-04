#!/bin/sh

# Test whether system time is being synchronized with ntp.
# Return exit status of ntpstat.
# If ntpstat is not available then grep the process list.

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo ""
echo "    === ntp test ===    "
echo ""

if type -p ntpstat >/dev/null; then

  echo " # UI ntp test # ntpstat says: "
  echo ""
  
  ntpstat
  
  if [ $? -ne 0 ]; then

    echo ""
    echo " # UI ntp test # It seems you have problem with ntp"
    echo ""
    echo " === test NOT passed === "
    exit 1

  fi

else

  echo ""
  echo " # UI ntp test # ntpstat not available - have to grep process list"
  echo ""

  if ps ax | grep -v grep | grep ntpd; then

    echo ""
    echo " # UI ntp test # ntpd seems running "

  else

    echo ""
    echo " # UI ntp test # ntpd probably not running"
    echo ""
    echo " === test NOT passed === "
    exit 1

  fi

fi

  
echo ""
echo " === test PASSED === "
exit 0
