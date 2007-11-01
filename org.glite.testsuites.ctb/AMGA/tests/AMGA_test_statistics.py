#!/usr/bin/env python
import socket
import sys
sys.path.append('../')
from config import samPrintERROR,samNewLine,samPrintINFO,ExitCodes,samPrintOK,samPrintPASSED

error_code = ExitCodes()

AMGA_HOST = sys.argv[1]

samPrintINFO('Trying to connect to: ' + AMGA_HOST + ' ...')
samNewLine()

# Open TCP socket to AMGA server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.connect((AMGA_HOST,8822))
    samPrintINFO("Connected")
    samNewLine()
    samNewLine()
    # Request statistics
    s.send('statistics\n\n')

    # Read response
    result = ''
    while 1:
        r = s.recv(1000)
        result = result + r
        if not r:
            break

    print "Server response:"
    print "<pre>"
    print result
    print "</pre>\n"
    samPrintOK("Got the Server Statistics")
    samNewLine()
    samPrintPASSED("AMGA statistics test")
    sys.exit(error_code.SAME_OK)

except Exception, e:
    samPrintERROR(e)
    sys.exit(error_code.SAME_ERROR)
