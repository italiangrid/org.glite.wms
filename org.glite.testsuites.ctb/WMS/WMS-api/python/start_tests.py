#!/usr/bin/python
'''
This script reads configuration file WMProxy_API.conf.
If configuration item is equal to True, test is run.
The WMProxy_HOST name is obligatory commandline argument. 
Author Liudmila Stepanova < Lioudmila.Stepanova@cern.ch >
 
'''
import ConfigParser 
import sys,os
len = len(sys.argv)
if len < 2 :
    print "The WMProxy host name is not defined"
    print "./start_tests.py [options]  < WmsProxy_id >"
    print "options:"
    print "        -r    <ce_id>"
    print "        -o    <file_path>" 
    sys.exit(1)

out = ""
req = ""
ERR = 0

if len == 2 :
      node = sys.argv[1]
if len >2 :
      for i in range(len)  :
          if sys.argv[i] == "-r" :
                   i = i + 1
                   req = sys.argv[i]
          else:
                   if sys.argv[i] == "-o": 
                       i = i + 1
                       out = sys.argv[i]
      node =  sys.argv[i]
Config = ConfigParser.ConfigParser()
Config.read("WMProxy_API.conf")
sections=Config.sections()
if out != "" :  
  handle = open(out,"w")
for x in sections[:]:
           list= Config.items(x)
           for i in list[:]:
                print i[0],i[1]
                if i[1]=="True" :
                    funk="python " + "tests/" + i[0]  + " " + node + " " + req + " "
                    ifile,ofile=os.popen4(funk)
                    for line in ofile.readlines():
                                         if out != "" :       
                                                handle.write(line)
                                         else:
                                                print line
                    if line.find("ERROR")>=1:
                                           ERR = 1
                    if out != "" :              
                                  print line
if out != "" :
                handle.close()
sys.exit(ERR)                   
