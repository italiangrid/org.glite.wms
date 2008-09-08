#!/usr/bin/env python
import mdstandalone
import mdinterface
import mdclient
import mdparser
import time
import math

#client=mdstandalone.MDStandalone('/tmp/')
client = mdclient.MDClient('localhost', 8833, 'koblitz', 'grobi')
#client.requireSSL()

try:
    print "Creating directory /pytest ..."
    client.createDir("/pytest")    
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Adding entries in bulk..."
    client.addEntries(["/pytest/a", "/pytest/b", "/pytest/c"])    
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "cd /pytest"
    client.cd("/pytest")
except mdinterface.CommandException, ex:
    print "Error:", ex
        
try:
    print "Adding attribute..."
    client.addAttr(".", "events", "int")    
except mdinterface.CommandException, ex:
    print "Error:", ex


try:
    print "Adding attribute..."
    client.addAttr("/pytest", "eventGen", "varchar(20)")    
except mdinterface.CommandException, ex:
    print "Error:", ex
    
try:
    print "Adding attribute..."
    client.addAttr("/pytest", "sinn", "float")    
except mdinterface.CommandException, ex:
    print "Error:", ex
    
try:
    print "Adding attribute..."
    client.addAttr("/pytest", "l1", "int")    
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
        client.addEntry("/pytest/t"+str(i),
                        ['events', 'eventGen', 'sinn', 'l1'],
                        [ i*100, 'LHCs Gen', math.sin(float(i)), i%2 ])
#                        [ i*100, 'LHC\'s Gen', math.sin(float(i)), i%2 ])
except mdinterface.CommandException, ex:
    print "Error:", ex


try:
    print "Getting all attributes..."
    client.getattr('/pytest', ['eventGen', 'sinn', 'events'])
    while not client.eot(): 
	file, values=client.getEntry()
  	print "->",file, values
except mdinterface.CommandException, ex:
    print "Error:", ex

#exit()

try:
    print "Creating directory /pytest/testdir ..."
    client.createDir("/pytest/testdir")    
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
    client.selectAttr(['pytest:eventGen', 'pytest:sinn', '/pytest:FILE'], 'pytest:FILE="my Gen"')
    while not client.eot():
        values=client.getSelectAttrEntry()
        print  "selcted ->", values
except mdinterface.CommandException, ex:
    print "Error:", ex

mdparser.DEBUG = False
mdstandalone.DEBUG = False                             
client.cd("/pytest");


try:
    print "Updating attributes, adding 1 to every sinn attribute"
    client.updateAttr('t?', ["sinn 'sinn + 1'"], '')
#    client.updateAttr('t?', ["sinn 'sinn + 1'"], '')
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
  print "Setting events to 42..."
  client.setAttr('/pytest/t?', ['events'], [42])
except mdinterface.CommandException, ex:
  print "Error:", ex

try:
    print "Removing directory /pylock/testdir ..."
    client.removeDir("/pytest/testdir") 
except mdinterface.CommandException, ex:
    print "Error:", ex
            

try:
    print "Getting all attributes..."
    client.getattr('/pytest/*', ['eventGen', 'sinn', 'events'])
    while not client.eot(): 
        file, values=client.getEntry()
        print "->",file, values
except mdinterface.CommandException, ex:
    print "Error:", ex                                

try: 
    print "Removing entries: rm /pytest/*"
    client.rm("/pytest/*")
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
    client.removeAttr("/pytest", "sinn") 
except mdinterface.CommandException, ex:
    print "Error:", ex

try: 
    print "Removing attribute..."
    client.removeAttr("/pytest", "eventGen")
except mdinterface.CommandException, ex:
    print "Error:", ex

try: 
    print "Removing attribute: l1 ..."
    client.removeAttr("/pytest", "l1")  
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Removing attribute..."
    client.removeAttr("/pylock", "id")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Creating sequence..."
    client.sequenceCreate("seq", "/pytest")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Getting next from sequence..."
    print client.sequenceNext("/pytest/seq")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Getting next from sequence..."
    print client.sequenceNext("/pytest/seq")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Removing sequence..."
    client.sequenceRemove("/pytest/seq")    
except mdinterface.CommandException, ex:
    print "Error:", ex

            

try:
    print "Removing directory /pytest..."
    client.removeDir("/pytest")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    print "Removing directory /pylock..."
    client.removeDir("/pylock")
except mdinterface.CommandException, ex:
    print "Error:", ex
                                                               
