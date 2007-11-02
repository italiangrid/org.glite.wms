#!/usr/bin/env python
import sys
import commands
import re
sys.path.append('../')
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
    SAME.samPrintOK("Host responded...")
    SAME.samNewLine()
    sys.exit(SAME.SAME_OK)
else :
    SAME.samPrintERROR(AMGA_HOST + " is not accessible.")
    SAME.samPrintFAILED("AMGA Ping test ")
    print "</pre>\n"
    sys.exit(SAME.SAME_ERROR)