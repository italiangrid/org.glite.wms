#!/usr/bin/env python
import sys
import commands
import re
sys.path.append('../')
from config import samPrintERROR,samNewLine,samPrintINFO,ExitCodes,samPrintOK,samPrintFAILED

error_code = ExitCodes()

AMGA_HOST = sys.argv[1];

samPrintINFO('Testing ping to ' + AMGA_HOST)  

command = 'ping -c 5 ' + AMGA_HOST

output = commands.getoutput(command)

#Check if output of the command contains the "Unreachable" word
host_found = not re.compile("Unreachable").match(output)

if host_found :
    print '<pre>'
    print output
    print '</pre>'
    samPrintOK("Host responded...")
    samNewLine()
    sys.exit(error_code.SAME_OK)
else :
    samPrintERROR(AMGA_HOST + " is not accessible.")
    samPrintFAILED("AMGA Ping test ")
    print "</pre>\n"
    sys.exit(error_code.SAME_ERROR)