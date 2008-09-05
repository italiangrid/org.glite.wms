#!/usr/bin/env python
import sys
import commands
import re
import os
sys.path.append(os.environ.get('SAME_SENSOR_HOME','..'))

from config import Config

SAME = Config()

AMGA_HOST = sys.argv[1];

SAME.samPrintINFO('Testing ping to ' + AMGA_HOST)  

command = 'ping -c 5 ' + AMGA_HOST

output = commands.getoutput(command)

#Check if output of the command contains the "Unreachable" word
host_found = not re.compile("Unreachable").match(output)

if host_found :
    print '<pre>'
    print output
    print '</pre>'
    SAME.samPrintOK("Host responded")
    SAME.samNewLine()
    SAME.samPrintPASSED("AMGA ping test -TEST PASSED-")
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(SAME.SAME_OK)
else :
    SAME.samPrintERROR(AMGA_HOST + " is not accessible.")
    SAME.samPrintFAILED("AMGA ping test -TEST FAILED-")
    print "</pre>\n"
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(SAME.SAME_ERROR)
