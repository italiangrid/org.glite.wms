#!/usr/bin/env python
import sys
import uuid

# try a local version, the one in ../src and finally the installed one in /usr/lib/pythonx.y/site-packages/amga...
try:
    import mdclient
    import mdinterface
except ImportError, e:
    print "using the source version"
    try:
        sys.path.append("../src/")
        import mdclient
        import mdinterface
    except ImportError, e:
        try:
            from amga import mdclient, mdinterface
        except ImportError, e:
            print "No mdclient in ", sys.path

client = mdclient.MDClient('pcardas02', 8822, 'root')
#client.requireSSL("/home/guest/.globus/userkey.pem", "/home/guest/.globus/usercert.pem")

try:
    client.siteAdd("FZK")
except mdinterface.CommandException, ex:
    print "Error:", ex
  
try:      
    client.siteAdd("CERN")
except mdinterface.CommandException, ex:
    print "Error:", ex
    
try:
    client.siteAdd("SARA")
except mdinterface.CommandException, ex:
    print "Error:", ex

try:
    for i in range(0, 100):
        l = []
        for j in range(0, 3):
            l.append((str(uuid.uuid1()), 'FZK'))
            l.append((str(uuid.uuid1()), 'CERN'))
            l.append((str(uuid.uuid1()), 'SARA'))
        client.replicaRegister(l)

except mdinterface.CommandException, ex:
    print "Error:", ex
                                                               
try:
    sites = client.siteList()
    print sites
        
    res = client.replicaList(['0ffe7c3e-50b2-48e5-82fe-2fe5896c60cc', '323d9ec4-1017-dc11-b423-000e0c099ae8',
                              'd26c9af6-4ada-db11-a901-001125c45e4e', 'e43eb51a-fbc0-db11-a4e1-00144f283370'])
    print res
    
except mdinterface.CommandException, ex:
    print "Error:", ex
