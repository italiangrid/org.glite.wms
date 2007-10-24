#!/usr/bin/env python
import math
import sys
import os
sys.path.append('../')
from config import samPrintERROR,samNewLine,samPrintINFO,ExitCodes,samPrintOK,samPrintPASSED

error_code = ExitCodes()

try:
    USER_PROXY_CERTIFICATE = os.environ['X509_USER_PROXY']
except:
    USER_PROXY_CERTIFICATE="/home/asterios/x509up_u30025"

AMGA_HOST = sys.argv[1]

print "Testing several AMGA functions "
try:
    sys.path.append("../lib/amga.client.api.python")
    import mdstandalone
    import mdclient
    import mdparser
    import mdinterface
except ImportError, e:
    print "No mdclient in ", sys.path

client = mdclient.MDClient(AMGA_HOST, 8822, 'test_user')
client.requireSSL(USER_PROXY_CERTIFICATE, USER_PROXY_CERTIFICATE)


def delete_entries():
    
    could_not_delete = 0
    
    try:
        client.removeDir("/test/pytest/testdir") 
    except:
        could_not_delete += 1 
    
    try:
        client.rm("/test/pytest/*")
    except:
        could_not_delete += 1 
        
    try:
        client.rm("/test/pylock/*")
    except:
        could_not_delete += 1 
    
    try:
        client.cd("/test/pytest")
        client.removeAttr(".", "events")
    except:
        could_not_delete += 1 
        
    try:
        client.removeAttr("/test/pytest", "sinus")
    except:
        could_not_delete += 1 
         
    try:
        client.removeAttr("/test/pytest", "eventGen")
    except:
        could_not_delete += 1 
        
    try:
        client.removeAttr("/test/pytest", "l1")  
    except:
        could_not_delete += 1 
        
    try:
        client.removeAttr("/test/pylock", "id")
    except:
        could_not_delete += 1 
        
    try:
        client.sequenceRemove("/test/pytest/seq")    
    except:
        could_not_delete += 1 
        
    try:
        client.removeDir("/test/pytest")
    except:
        could_not_delete += 1 
        
    try:
        client.removeDir("/test/pylock")
    except:
        could_not_delete += 1 
    
    #try:
    #    client.removeDir("/test/")
    #except:
    #    could_not_delete += 1 
    
    if could_not_delete != 11:
        samPrintERROR("Could not \"clean\" the /test/ folder")                                        

try:
    
    print "<pre>"
    
    delete_entries()

    print "Creating directory /pytest ..."
    client.createDir("/test/pytest")
    print "OK"

    print "cd /pytest"
    client.cd("/test/pytest")
    print "OK"
        
    print "Adding attribute..."
    client.addAttr(".", "events", "int")
    print "OK"    

    print "Adding attribute..."
    client.addAttr("/test/pytest", "eventGen", "varchar(20)")
    print "OK"    
    
    print "Adding attribute..."
    client.addAttr("/test/pytest", "sinus", "float")
    print "OK"    
    
    print "Adding attribute..."
    client.addAttr("/test/pytest", "l1", "int")
    print "OK"    

    print "Listing attributes..."
    attributes, types=client.listAttr("./t0")
    print attributes
    print types
    print "OK"

    print "Adding entries..."
    for i in range(0,1):
        client.addEntry("/test/pytest/t"+str(i),
                        ['events', 'eventGen', 'sinus', 'l1'],
                        [ i*100, 'LHCs Gen', math.sin(float(i)), i%2 ])
#                        [ i*100, 'LHC\'s Gen', math.sin(float(i)), i%2 ])
    print "OK"
    
    print "Getting all attributes..."
    client.getattr('/test/pytest', ['eventGen', 'sinus', 'events'])
    while not client.eot(): 
        file, values=client.getEntry()
        print "->",file, values
    print "OK"


    print "Creating directory /pytest/testdir ..."
    client.createDir("/test/pytest/testdir")    
    print "OK"
    
    
    print "Listing entries..."
    client.listEntries('.')
    while not client.eot():
        file, type=client.getEntry()
        print "->",file, type[0]
    print "OK"
    
    print "Creating directory /pylock ..."
    client.createDir("/test/pylock")    
    print "OK"
    
    
    print "Adding attribute id..."
    client.addAttr("/test/pylock", "id", "int")
    print "OK"
    
    print "Creating LOCK table..."
    client.addEntry("/test/pylock/lock", ['id'], [4711])
    client.addEntry("/test/pylock/lock2", ['id'], [ 100])
    print "OK"

    print "Selecting attributes"
    client.selectAttr(['/test/pytest:eventGen'], '/test/pytest:FILE="t2" or /test/pytest:FILE="t1"')
#    client.selectAttr(['.:eventGen', '.:sinus', '.:FILE'], '')
#    client.selectAttr(['/pytest:eventGen', '/pytest:sinus', '/pytest:FILE'], '/pytest:eventGen="t1"')
#    client.selectAttr(['/pytest:eventGen', '/pytest:sinus', '/pytest:events'], '/pytest:events = 100')
    while not client.eot():
        values=client.getSelectAttrEntry()
        print  "selected ->", values
    client.cd("..");
    print "OK"

    print "Selecting attributes"
    client.selectAttr(['/test/pytest:eventGen', '/test/pytest:sinus', '/test/pytest:FILE'], '/test/pytest:FILE="my Gen"')
    while not client.eot():
        values=client.getSelectAttrEntry()
        print  "selected ->", values
    print "OK"

    mdparser.DEBUG = False
    mdstandalone.DEBUG = False                             
    client.cd("/test/pytest");


    print "Updating attributes, adding 1 to every sinus attribute"
    client.updateAttr('t?', ["sinus 'sinus + 1'"], '')
    print "OK"
    
    print "Setting events to 42..."
    client.setAttr('/test/pytest/t?', ['events'], [42])
    print "OK"

    print "Removing directory /pylock/testdir ..."
    client.removeDir("/test/pytest/testdir")
    print "OK"

    print "Getting all attributes..."
    client.getattr('/test/pytest/*', ['eventGen', 'sinus', 'events'])
    while not client.eot(): 
        file, values=client.getEntry()
        print "->",file, values 
    print "OK"
    
    print "Removing entries: rm /pytest/*"
    client.rm("/test/pytest/*")
    print "OK"

    print "Removing entries: rm /pylock/*"
    client.rm("/test/pylock/*")
    print "OK"
    
    print "Removing attribute: events ..."
    client.removeAttr(".", "events")
    print "OK"

    print "Removing attribute: sinus ..."
    client.removeAttr("/test/pytest", "sinus") 
    print "OK"
    
    print "Removing attribute..."
    client.removeAttr("/test/pytest", "eventGen")
    print "OK"
    
    print "Removing attribute: l1 ..."
    client.removeAttr("/test/pytest", "l1")  
    print "OK"

    print "Removing attribute..."
    client.removeAttr("/test/pylock", "id")
    print "OK"
    
    print "Creating sequence..."
    client.sequenceCreate("seq", "/test/pytest")
    print "OK"
    
    print "Getting next from sequence..."
    print client.sequenceNext("/test/pytest/seq")
    print "OK"
    
    print "Getting next from sequence..."
    print client.sequenceNext("/test/pytest/seq")
    print "OK"
    
    print "Removing sequence..."
    client.sequenceRemove("/test/pytest/seq")    
    print "OK"

    print "Removing directory /pytest..."
    client.removeDir("/test/pytest")
    print "OK"

    print "Removing directory /pylock..."
    client.removeDir("/test/pylock")
    print "OK"
    
    print "</pre>"
    
    samPrintOK("All functions executed properly")
    samNewLine()
    samPrintPASSED("AMGA functional tests passed")
    sys.exit(error_code.SAME_OK)
    
    
except Exception, ex:
    print "</pre>"
    samPrintERROR(AMGA_HOST + ': encountered errors.\n')
    print "<pre> ", ex, "</pre>" 
    sys.exit(error_code.SAME_ERROR)
                        
                        
