#! /usr/bin/env python2

# Copyright (c) Members of the EGEE Collaboration. 2004. 
# See http://www.eu-e.org/partners/ for details on the copyright
# holders.  
# 
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License. 
# You may obtain a copy of the License at 
# 
#     http://www.apache.org/licenses/LICENSE-2.0 
# 
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
# See the License for the specific language governing permissions and 
# limitations under the License.

"""
***************************************************************************
    filename  : wmsui_utils.py
    author    : Alessandro Maraschini
    email     : job management <glite-jobmgmt-devel@lists.infn.it>
    copyright : (C) 2003 by DATAMAT
***************************************************************************
//
// $Id: wmsui_utils.py,v 1.4.2.2.2.6.2.3 2012/04/26 13:03:10 adorigo Exp $
//
"""
import sys
import os
import dl # dl flags used by queryLB
import os.path
import glob #accessing file
import time
import socket #getnameinfo getLB
import random #  random number
import math   #  random
import time   #  random
from glite_wmsui_UcWrapper import UCredential


# set dinamyc library management (dlopen would not work otherwise):
sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)

"""
Signal Handler
"""
def ctc(signo, frame):
	msg = "Keyboard interrupt raised by user, now exiting...\nbye"
	print_message(False, info.logFile , msg , stderror=1 )
	exit(1)


"""
****************************************************
Info Class
This class stores a series of program information
****************************************************
"""


class Info:
####### Initial Check: try to find UI configuration file path
###### Setting of sys.path has to be done before starting import
###### of swig generated modules                                 ##
  def __init__(self):
   # Initial Check: try to find UI configuration file path
#   try:
#     path=os.environ['GLITE_WMS_LOCATION']
#   except:
##      try:
#         path=os.environ['GLITE_LOCATION']
#      except:
#         print "Error: Please set the GLITE_WMS_LOCATION environment variable pointing to the userinterface installation path"
#         sys.exit(1)
   #pathe where the UI has been installed
   #path = "/etc/glite-wms"
   try:
	   path=os.environ['EMI_UI_CONF']+"/usr"
   except:
	   path="/usr"
   
   self.prefix=path
   # name of the script used
   self.prgname =""
   # file where errorcodes are stored
   self.logFile = ""
   # configuration UI file
   self.confFile = ""
   # output info file
   self.outFile = ""
   # configuration VO file
   self.confVoFile = ""
   #Submit message set
   self.msgSet =0
   # interaction
   self.noint  = 0
   self.debug  = 0
   self.setLog = 0 #???
   self.version = ""
   #Stores Adinformation
   self.confAd =0
   self.confAdVo =0
   self.issuer = ""
   self.TYPE = 0

"""
****************************************************
wmsui_utils Methods
****************************************************
"""
#This static global instance stores all program information
info = Info ()

"""
This method returns the name of the error log file
"""
def create_err_log(output):
   try:
      errPath = solvePath( info.confAd.getStringValue ("ErrorStorage")[0] )
      #print "ErrorStorage read from file:" , errPath
   except:
      errPath = os.sep + "tmp"
      #print "Unable to read from file, storing default:" , errPath
   uid=os.getuid()
   pid=os.getpid()
   tm = int(time.time())
   err_log_file=errPath + os.sep + info.prgname  + '_' + repr(uid) + '_' + repr(pid) + '_' + repr(tm)  +'.log'
   return err_log_file

def solvePath( dir  ):
	if dir.find("~" + os.sep) == 0:
		dir= os.environ["HOME"] + dir[1:]
	return os.path.expandvars(dir)

def print_help(msg):   
	if not info.logFile:   
		sys.stderr.write( msg)   
""" 
This Method provides a standard question Y/N
and returns a value n=0, y=1
"""
def questionYN(question):
   keep=1
   question=question+ ' [y/n]n :'
   while keep:
      ans=raw_input(question)
      if (ans=='n')or(ans=='N') or(ans==''):
         return 0
         keep=0
      elif (ans=='y')or(ans=='Y'):
         return 1
         keep=0
"""
This Method cleans the Error_log_file if it exists
"""
def err_log_clean(prgname):
   MAXTIME = 1
   info.prgname = prgname
   maxtime=MAXTIME*60*60
   err_log_file=create_err_log(info.prgname)
   ind = err_log_file.rfind("_")
   err_log_file=err_log_file[:ind] #cut the PID
   avail_files=glob.glob(err_log_file +"*")
   if avail_files:
     now=time.time()
     for f in avail_files:
       try:
         last_modify=os.path.getmtime(f)
         if now - last_modify> maxtime:
           os.remove(f)
       except:
         pass
"""
Create the Standard help message
starting from the available
"""
def createErrMsg(cmd , extra,opts):
   errMsg = "WMS User Interface version " + info.version
   errMsg=errMsg + "\nCopyright (C) 2003 by DATAMAT SpA"
   errMsg=errMsg + "\n\nUsage: "+ cmd +"  [options]"
   if extra:
       errMsg=errMsg + "  <"+ extra+">"
   errMsg=errMsg + "\nOptions:"
   param = ""
   short=""
   for opt in opts:
       param = ""
       short=""
       if opt[0] =="=":
          #Short option allowed
          opt=opt[1:]
          short=", -"+ opt[0]
       if opt[-1]=="=":
           #Param value needed by option
           opt=opt[:-1]
           param= "<"+opt+" value>"
       elif opt[-1]=="$":
           #Param value needed by option
           opt=opt[:-1]
           param= "<"+opt+" file>"
       if opt.find("from")!=-1 or opt.find("to")!=-1:
           #verbosity value needed by option
           param= "[MM:DD:]hh:mm[:[CC]YY]"
       if opt.find("user-tag")!=-1:
           #userTag implementation value needed by option
           param= "<tag name>=<tag value>"
       if opt.find("verbosity")!=-1:
           #verbosity value needed by option
           param= "[0|1|2|3]"
       if opt.find("valid")!=-1:
           #verbosity value needed by option
           param= "<hours>:<minutes>"
       if opt=="@":
          #Special line-separator char
          opt="\n"
       else:
          opt="\n   --"+opt+short
       errMsg=errMsg + opt.ljust(24) +param
   errMsg=errMsg + "\n\n Please report any bug at:\n     job management <glite-jobmgmt-devel@lists.infn.it>\n"
   return errMsg


"""
 This method prints a message on the Log file
 it appends the message if the file exists from less then one day
 it writes a message on a new file if the file doesn't exist
"""
def print_message(json, log_file , message , stderror=0 ):
	if log_file:
		if (info.setLog) or (info.debug) or (info.noint):
			# --logFile or --debug active: print to screen too (if it is a message)
			if log_file != info.outFile:
				# In this case it is not a message to be printed to screen too. but only in the file
				if stderror:
					if json == False:
						sys.stderr.write ( message+ "\n" )
					else:
						sys.stderr.write ( message )
				else:
					if json == False:
						print message
					else:
						print message,
		try:
			if os.path.isfile(log_file):
				f=open(log_file,'a+')
				try:
					f.seek(-22,2)#placing the cursor at last line
					line = f.readline().strip()
					if line=="---No Errors found---":
						f.seek(-22,2) #Place the cursor in order to delete last line
				except:
					#do nothing, the file has no such characters
					pass
			else:
				f=open(log_file,'w')
			f.write(message+"\n")
			f.close()
		except:
			errMsg('Error','UI_WRITE_FILE',log_file)
			exit(1)
	else:
		if stderror:
			if json == False:
				sys.stderr.write ( message+ "\n" )
			else:
				sys.stderr.write ( message )
		else:
			if json == False:
				print message
			else:
				print message,


"""
Print the Error Message corresponding to
the Error code
"""
def dbgMsg( fName , *arg ):
 if not info.debug:
    return
 message = "#### "+ time.ctime() + " Debug API ####\nThe function '"+fName +"' has been called"
 if not arg:
    message= message +" (it doesn't require any parameter)"
 else :
   message = message + " with the following parameter(s):"
   for ar in arg:
      if type(ar) != type (" "): #it's not a string, a repr is necessary
         ar = repr(ar)
      message = message + "\n>> " + ar
   message = message + "\n#### End Debug ####"
 message = message +"\n"
 print_message(False, info.logFile , message) #print on the screen  (only if -noint is NOT selected)


"""
This method prints the Error Message corresponding to
the Error code
"""
def errMsg(errType,strDef,*arg):
 #Read Error file
 path = info.prefix
 if path:
   errFile = path + os.sep + "etc" + os.sep + "glite_wmsui_cmd_err.conf"
 else:
   print "Fatal Error: Unable to find userinterface configuration error file"
   exit (1)
 message = ''
 try:
   op = open(errFile,'r')
 except:#The environment variable has changed
   print "\nFatal Error: Unable to find the file "+errFile
   exit(1)

 keep = 0  #this flag indicates that the error message is spread on more lines
 lr = op.readlines()
 op.close()
 for l in lr:
    l=l.strip()
    if l:
     if not (l.startswith("#") or l.startswith("//")):  #comment check
      if keep: #previous line continues
       errSTR = l.strip()
       if message:
         message=message[:-1]
         message=message + "\n" + l.strip()
      else:

         try:
           errCOD,errSTR = l.split('=',1)
         except:
           print l+':corrupted line in CFG error file'
           exit(1)
         errCOD=errCOD.strip()
         errSTR=errSTR.strip()
         if (errCOD == strDef):#Error message found
            message = errSTR.strip()

      #Check if the line continues
      lon=len(errSTR)-1
      if errSTR and errSTR[lon]=='\\':
        keep=1
      else:
         if message:
            break
         else:
            keep=0

 if arg:
     for ar in arg:
        if type(ar) != type(""): #it's not a string
          ar=repr(ar)
        message = message.replace('%',ar,1)
 op.close()
 if message:
   message = "**** "+errType + ': ' + strDef + ' ****  \n' + message + '\n'
 else:
   #This message should never be activated
   message = errType + ' ' + strDef + ':  ' + '(Error Message not available)' + '\n'
 print_message( False, info.logFile , message , stderror=1)



"""
Finds a value among the specified options
 usage mode: userTags = wmsui_utils.findVal(["--user-tag"], options, values)
listVals is a list of stirngs containing all the queried
options is the list of all parameter options: ['-i', '--user-tag', '--user-tag']
values is the list of all parameter values: ['jobid.list', 'user=value', 'group=datamat' ]
"""
def findVal(listVals,options,values, *duplicate):
	result=[]
	i = 0
	for key in listVals:
		if  (key in options):
			#===Key found===#
			for i in range(  len(options)  ):
				if key == options[i]:
					if duplicate: #Duplicate values are allowed
						result.append( values[i] )
					elif result:
						raise  SyntaxError, listVals[0]
					else:
						result = values[i]
			i+=1
	return result

"""
 This method prints informations about error log files
 And exit
"""
def exit(value):
  if (info.msgSet):
    if value==1:
      print info.logFile
  #elif os.path.isfile(log_file):
  elif info.logFile:
      print "\n                           *** Log file created ***"
      print "Possible Errors and Debug messages have been printed in the following file:\n"+ info.logFile +"\n"
  sys.exit(value)


#NS / LB METHODS:

"""
Retreve LB list from configuration file
"""
def getLBs (  *nsNum  ):
	try:
		nsNum=nsNum[0]
	except:
		nsNum=-1   # Default Value (no LB specific)
	# check values
	lbs =  info.confAdVo.getStringList ("LBAddresses")
	lbsToCheck = []
	discardedLBS=[]
	outputLb=[]
	i = 0
	for lb in lbs:
		if not lb:
			# it's nextLB signal
			i+=1
		elif i==nsNum:
			# ns Num has been specified and equal to LB index
			lbsToCheck.append( lb)
		elif nsNum==-1:
			#nsNum has NOT been specified(cancel or status)
			lbsToCheck.append( lb)
		else:
			#nsNum specified BUT different to LB index
			discardedLBS.append(lb)
	if i==1:
		# never met a nextLB signal
		if not lbsToCheck:	
			lbsToCheck=discardedLBS
	if not  lbsToCheck:
		errMsg('Warning','UI_CONFIG_ATTRIBUTE', "LBAddresses","Unable to find any")
	else:
		for lb in lbsToCheck:
			err , lbAdd = checkLb (lb)
			if not err:
					outputLb.append (lbAdd)
		if not lbs:
			errMsg('Warning','UI_CONFIG_ATTRIBUTE', "LBAddresses", "Unable to find any")
	return outputLb

"""
Check the validity of the specified Lb address
"""
def checkLb( lb):
	DEFAULT_LB_PROTOCOL = "https://"
	DEFAULT_LB_PORT = 9000
	prot_ind = lb.find( "://" )
	if prot_ind<0:
		prot_ind = 0
		prot = DEFAULT_LB_PROTOCOL
	else:
		prot = lb[:prot_ind]
		lb= lb[prot_ind+3:]
	port_ind = lb.find (":")
	if port_ind <0:
		port = DEFAULT_LB_PORT
		port_ind= len (lb)
	else:
		port =   lb  [  port_ind+1:  ]
	#Check the port:
	try:
		port = int (  port)
		lb=resolveHostName(lb[prot_ind : port_ind])
	except:
		errMsg('Warning','UI_CONFIG_ATTRIBUTE', "LBAddresses, host not found: "+lb)
		return [1, ""]
	address = lb
	return [0   , [address , port]   ]

def resolveHostName(host):
	for res in socket.getaddrinfo(host, "", socket.AF_UNSPEC, socket.SOCK_STREAM):
		address_family, socket_type, proto, canon_name, socket_address = res
	hostname, port_number = socket.getnameinfo (socket_address, 0)
	return hostname

def querySd(voName, serviceType):
	"""
	Used by queryLB
	"""
	from glite_wmsui_SdWrapper import ServiceDiscovery
	dbgMsg ("ServiceDiscovery::lookForServices", voName, serviceType)
	sd=ServiceDiscovery()
	foundAddresses=sd.lookForServices(voName, serviceType)
	if not foundAddresses and voName:
		# if VO specific not found, try and contact service for ANY VO:
		dbgMsg ("ServiceDiscovery::lookForServices", serviceType)
		foundAddresses=sd.lookForServices("", serviceType)
	if not foundAddresses:
		errMsg('Warning','SD_ERROR', sd.get_error() + " for "+serviceType)
	return foundAddresses

def queryLBs (voName, alreadyTested):
	"""
	Retreve LB list from service Discovery
	return the list of found LB addresses
	"""
	LB_SERVICE_TYPE="org.glite.lb.Server"
	lbs=[]
	# prompt user
	if info.noint:
		# Authomatically query NS
		errMsg("Warning","UI_SKIP_QUESTION","Authomatically query the Service Discovery for more Logging and Bookeeping Servers")
	elif questionYN("Do you wish to query the Service Discovery for more Logging and Bookeeping Servers?"):
		# Prompt user for decision
		pass # continue
	else:
		# Do not query SD, return empty vector
		return lbs
	#YES replied
	lbsToCheck = querySd(voName,LB_SERVICE_TYPE)
	for lb in lbsToCheck:
		if not lb in alreadyTested:
			err , lbAdd = checkLb (lb)
			if not err:
				lbs.append (lbAdd)
	return lbs


"""
Print full command detailed help information
"""
def printFullHelp(msg, noPager):
     #Read Help File
     noPager=1
     path =info.prefix
     helpFile = path + os.sep + "etc" + os.sep + "glite_wmsui_cmd_help.conf"
     try:
         f = open(helpFile,'r')
         helpLines=f.readlines()
     except:#The environment variable has changed
         print "Warning: Unable to print full-help information.\nUnable to find the file:   "+helpFile
         sys.stderr.write (msg)
         return
     #Read Command specific Help
     cmd="Job"+ info.prgname [13:]
     #Find the command description
     
     try:
         start = helpLines.index('%%begin'+cmd+'%%\n')
     except:
         print "Warning: Unable to print full-help information.\nHelp file seems to be corrupted:   "+helpFile
         print sys.stderr.write (msg)
         return

     end = helpLines.index('%%end'+cmd+'%%\n')
     helpList = helpLines[start+1:end]
     for i in range(len(helpList)  ):
         line=helpList[i]
         print line[:-1]
         if not noPager:
           if (i+1)%30 ==0:
              print "                <Press ENTER to continue or q to exit>"
              a=raw_input()
              if a.lower()=="q":
                break
     print ""
