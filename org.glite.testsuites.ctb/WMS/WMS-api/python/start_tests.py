#!/usr/bin/python
'''
This script reads configuration file WMProxy_API.conf.
If configuration item is equal to True, test is run.
If the the option  <filename> is given the output should be written to the file <filename>.
example:   ./start_tests.py lxb2054.cern.ch std.out 
The WMSLB_HOST name is obligatory commandline argument. 
Author Liudmila Stepanova < Lioudmila.Stepanova@cern.ch >
 
'''
import ConfigParser 
import sys,os
if len(sys.argv) < 2 :
    print "The WMSLB_HOST name is not defined"
    print "example:     ./start_tests.py lxb2054.cern.ch " 
    sys.exit(1)
node = sys.argv[1] 
out = ""
ERR = 0
Config = ConfigParser.ConfigParser()
Config.read("WMProxy_API.conf")
sections=Config.sections()
if len(sys.argv) > 2 :
  out = sys.argv[2]
  handle = open(out,"w")
for x in sections[:]:
           list= Config.items(x)
           for i in list[:]:
                print i[0],i[1]
                if i[1]=="True" :
                    funk="python " + "tests/" + i[0]  + " " + node + " "
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
