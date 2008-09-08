#!/usr/bin/env python
#import mdstandalone
#import mdinterface
#import mdclient
#import mdparser
import os
import sys
import time
import math

try:
    MDPATHTOP = os.environ.get('SAME_SENSOR_HOME','..')
    sys.path.append(MDPATHTOP+"/lib/amga.client.api.python")
    import mdstandalone
    import mdclient
    import mdparser
    import mdinterface
except ImportError, e:
     print "No mdclient in ", sys.path


try:
    USER_PROXY_CERTIFICATE = os.environ['X509_USER_PROXY']
except:
    print "Cannot find a proper certificate, is X509_USER_PROXY variable set?"
    sys.stdout.flush()
    sys.stderr.flush()
    sys.exit(SAME.SAME_ERROR)

#client=mdstandalone.MDStandalone('/tmp/')
#client = mdclient.MDClient('localhost', 8833, 'koblitz', 'grobi')
AMGA_HOST = sys.argv[1]
client = mdclient.MDClient(AMGA_HOST, 8822, 'test_user')
client.requireSSL(USER_PROXY_CERTIFICATE, USER_PROXY_CERTIFICATE)
#client.requireSSL()

try:
    print "Creating directory /test ..."
    client.createDir("/test")    
#    client.createDir("/pytest")    
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Adding entries in bulk..."
    client.addEntries(["/test/a", "/test/b", "/test/c"])    
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "cd /test"
    client.cd("/test")
except mdinterface.CommandException, ex:
    print "Error:", ex
        
try:
    print "Adding attribute..."
    client.addAttr(".", "events", "int")    
except mdinterface.CommandException, ex:
    print "Error:", ex


try:
    print "Adding attribute..."
    client.addAttr("/test", "eventGen", "varchar(20)")    
except mdinterface.CommandException, ex:
    print "Error:", ex
    
try:
    print "Adding attribute..."
    client.addAttr("/test", "sinn", "float")    
except mdinterface.CommandException, ex:
    print "Error:", ex
    
try:
    print "Adding attribute..."
    client.addAttr("/test", "l1", "int")    
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Listing attributes..."
    attributes, types=client.listAttr("./t0")
    print attributes
    print types
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Adding entries..."
    for i in range(0,10):
        client.addEntry("/test/t"+str(i),
                        ['events', 'eventGen', 'sinn', 'l1'],
                        [ i*100, 'LHCs Gen', math.sin(float(i)), i%2 ])
#                        [ i*100, 'LHC\'s Gen', math.sin(float(i)), i%2 ])
except mdinterface.CommandException, ex:
    print "Error:", ex


try:
    print "Getting all attributes..."
    client.getattr('/test', ['eventGen', 'sinn', 'events'])
    while not client.eot(): 
	file, values=client.getEntry()
  	print "->",file, values
except mdinterface.CommandException, ex:
    print "Error:", ex

#exit()

try:
    print "Creating directory /test/testdir ..."
    client.createDir("/test/testdir")    
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Listing entries..."
    client.listEntries('.')
    while not client.eot():
        file, type=client.getEntry()
        print "->",file, type[0]
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Creating directory /pylock ..."
    client.createDir("/pylock")    
except mdinterface.CommandException, ex:
    print "Error:", ex
            

try:
   print "Adding attribute id..."
   client.addAttr("/pylock", "id", "int")
except mdinterface.CommandException, ex:
   print "Error:", ex  
   
   
try:
    print "Creating LOCK table..."
    client.addEntry("/pylock/lock", ['id'], [4711])
    client.addEntry("/pylock/lock2", ['id'], [ 100])
except mdinterface.CommandException, ex:
    print "Error:", ex
  

try:
    print "Selecting attributes"
#    client.selectAttr(['.:eventGen', '.:sinn', '.:FILE'], '')
#    client.selectAttr(['/pytest:eventGen', '/pytest:sinn', '/pytest:FILE'], '/pytest:eventGen="t1"')
#    client.selectAttr(['/pytest:eventGen', '/pytest:sinn', '/pytest:events'], '/pytest:events = 100')
    while not client.eot():
        values=client.getSelectAttrEntry()
        print  "selcted ->", values
except mdinterface.CommandException, ex:
    print "Error:", ex


client.cd("..");

#mdparser.DEBUG = True
#mdstandalone.DEBUG = True
try:
    print "Selecting attributes"
    client.selectAttr(['test:eventGen', 'test:sinn', '/test:FILE'], 'test:FILE="my Gen"')
    while not client.eot():
        values=client.getSelectAttrEntry()
        print  "selcted ->", values
except mdinterface.CommandException, ex:
    print "Error:", ex

mdparser.DEBUG = False
mdstandalone.DEBUG = False                             
client.cd("/test");


try:
    print "Updating attributes, adding 1 to every sinn attribute"
    client.updateAttr('t?', ["sinn 'sinn + 1'"], '')
#    client.updateAttr('t?', ["sinn 'sinn + 1'"], '')
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
  print "Setting events to 42..."
  client.setAttr('/test/t?', ['events'], [42])
except mdinterface.CommandException, ex:
  print "Error:", ex

try:
    print "Removing directory /test/testdir ..."
    client.removeDir("/test/testdir") 
except mdinterface.CommandException, ex:
    print "Error:", ex
            

try:
    print "Getting all attributes..."
    client.getattr('/test/*', ['eventGen', 'sinn', 'events'])
    while not client.eot(): 
        file, values=client.getEntry()
        print "->",file, values
except mdinterface.CommandException, ex:
    print "Error:", ex                                

try: 
    print "Removing entries: rm /test/*"
    client.rm("/test/*")
except mdinterface.CommandException, ex:
    print "Error:", ex


try: 
    print "Removing entries: rm /pylock/*"
    client.rm("/pylock/*")
except mdinterface.CommandException, ex:
    print "Error:", ex

try: 
    print "Removing attribute: events ..."
    client.removeAttr(".", "events")
except mdinterface.CommandException, ex:
    print "Error:", ex

try: 
    print "Removing attribute: sinn ..."
    client.removeAttr("/test", "sinn") 
except mdinterface.CommandException, ex:
    print "Error:", ex

try: 
    print "Removing attribute..."
    client.removeAttr("/test", "eventGen")
except mdinterface.CommandException, ex:
    print "Error:", ex

try: 
    print "Removing attribute: l1 ..."
    client.removeAttr("/test", "l1")  
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Removing attribute..."
    client.removeAttr("/pylock", "id")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Creating sequence..."
    client.sequenceCreate("seq", "/test")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Getting next from sequence..."
    print client.sequenceNext("/test/seq")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Getting next from sequence..."
    print client.sequenceNext("/test/seq")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Removing sequence..."
    client.sequenceRemove("/test/seq")    
except mdinterface.CommandException, ex:
    print "Error:", ex

            

try:
    print "Removing directory /test..."
    client.removeDir("/test")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Removing directory /pylock..."
    client.removeDir("/pylock")
except mdinterface.CommandException, ex:
    print "Error:", ex
                                                               
