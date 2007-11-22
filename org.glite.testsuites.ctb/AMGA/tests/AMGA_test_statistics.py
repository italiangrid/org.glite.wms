#!/usr/bin/env python
import socket
import sys
sys.path.append('../')
from config import Config

SAME = Config()

AMGA_HOST = sys.argv[1]

SAME.samPrintINFO('Trying to connect to: ' + AMGA_HOST + ' ...')
SAME.samNewLine()

# Open TCP socket to AMGA server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.connect((AMGA_HOST,8822))
    SAME.samPrintINFO("Connected")
    SAME.samNewLine()
    SAME.samNewLine()
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
    SAME.samPrintOK("Got the Server Statistics")
    SAME.samNewLine()
    SAME.samPrintPASSED("AMGA statistics test")
    os._exit(SAME.SAME_OK)

except Exception, e:
    SAME.samPrintERROR(e)
    os._exit(SAME.SAME_ERROR)
