#!/usr/bin/env python
import socket
import sys
import os
import threading
sys.path.append(os.environ.get('SAME_SENSOR_HOME','..'))
from config import Config


def timeout():
    print "</pre>"
    SAME.samPrintERROR(AMGA_HOST + ': encountered errors.\n')
    print "<pre> ", "AMGA test timed out after 2 minutes of waiting", "</pre>" 
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(SAME.SAME_ERROR)


SAME = Config()

AMGA_HOST = sys.argv[1]

SAME.samPrintINFO('Trying to connect to: ' + AMGA_HOST + ' ...')
SAME.samNewLine()




t = threading.Timer(120.0, timeout)
t.start()



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
    SAME.samPrintPASSED("AMGA statistics test -TEST PASSED-")
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(SAME.SAME_OK)
except Exception, e:
    SAME.samPrintERROR(e)
    SAME.samPrintFAILED("AMGA statistic test -TEST FAILED-")
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(SAME.SAME_ERROR)
