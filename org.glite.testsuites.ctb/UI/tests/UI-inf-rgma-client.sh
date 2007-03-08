#!/bin/sh

# Run rgma-client-check

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

echo "    === UI rgma client test ===    "

if [ -x $GLITE_LOCATION/bin/rgma-client-check ]; then

  $GLITE_LOCATION/bin/rgma-client-check

  if [ $? -eq 0 ]; then
     echo "    === test PASSED === "
     exit 0;
  else
     echo " *** rgma-client-check returned failure *** "
  fi

else
   echo " *** rgma-client-check script not found *** "
fi

echo " *** test NOT passed *** "
exit 1
