#! /usr/bin/env python2

# Copyright (c) Members of the EGEE Collaboration. 2004. 
# See http://www.eu-egee.org/partners/ for details on the copyright
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
    filename  : wmsui_checks.py
    author    : Alessandro Maraschini
    email     : egee@datamat.it
    copyright : (C) 2003 by DATAMAT
***************************************************************************
//
// $Id: wmsui_checks.py,v 1.13.2.1.2.3.2.3 2012/05/11 13:13:18 adorigo Exp $
//
"""


import wmsui_utils
from wmsui_utils import info
from wmsui_utils import errMsg
import os
import shutil
import sys
import time
from glite_wmsui_AdWrapper import AdWrapper
from glite_wmsui_UcWrapper import UCredential

# List of Deprecated attributes outside JDL Default Attributes
VIRTUAL_ORGANISATION = "VirtualOrganisation"
RETRYCOUNT = "RetryCount"
SHALLOWRETRYCOUNT = "ShallowRetryCount"
RANK = "rank"
REQUIREMENTS = "requirements"
MYPROXY = "MyProxyServer"
JOB_PROVENANCE = "JobProvenance"
LB_ADDRESS = "LBAddress"
ALLOW_ZIPPED_ISB = "AllowZippedISB"
PU_FILE_ENABLE = "PerusalFileEnable"

deprecatedAttributes = [VIRTUAL_ORGANISATION, RETRYCOUNT, SHALLOWRETRYCOUNT, RANK, REQUIREMENTS, MYPROXY, JOB_PROVENANCE, LB_ADDRESS, ALLOW_ZIPPED_ISB,PU_FILE_ENABLE]

# List of attributes to be used to fill the final configuration AD
JDL_DEFAULT_PROXY_VALIDITY = "DefaultProxyValidity"
JDL_DEFAULT_STATUS_LEVEL = "DefaultStatusLevel"
JDL_DEFAULT_LOGGING_LEVEL = "DefaultLoggingLevel"
JDL_ERROR_STORAGE = "ErrorStorage"
JDL_OUTPUT_STORAGE = "OutputStorage"
JDL_LISTENER_STORAGE = "ListenerStorage"
JDL_LB_SERVICE_DISCOVERY_TYPE = "LBServiceDiscoveryType"
JDL_WMPROXY_SERVICE_DISCOVERY_TYPE = "WMProxyServiceDiscoveryType"
JDL_ENABLE_SERVICE_DISCOVERY = "EnableServiceDiscovery"
JDL_WMPROXY_ENDPOINT = "WmProxyEndPoints"
JDL_DEFAULT_ATTRIBUTES = "JdlDefaultAttributes"
JDL_SOAP_TIMEOUTS = "SoapTimeouts"
JDL_SYSTEM_CALL_TIMEOUT = "SystemCallTimeout"
LB_ADDRESSES = "LBAddresses"
DELEGATION_ID = "DelegationId"

DEFAULT_UI_CLIENTCONFILE = "glite_wmsui.conf"
DEFAULT_UI_CONFILE       = "glite_wms.conf" # kept for compatibility purpose with older versions

configAttributes = [JDL_DEFAULT_PROXY_VALIDITY, JDL_DEFAULT_STATUS_LEVEL, JDL_DEFAULT_LOGGING_LEVEL, JDL_ERROR_STORAGE, JDL_OUTPUT_STORAGE, JDL_LISTENER_STORAGE, JDL_LB_SERVICE_DISCOVERY_TYPE, JDL_WMPROXY_SERVICE_DISCOVERY_TYPE, JDL_ENABLE_SERVICE_DISCOVERY, JDL_WMPROXY_ENDPOINT, JDL_DEFAULT_ATTRIBUTES, JDL_SOAP_TIMEOUTS, JDL_SYSTEM_CALL_TIMEOUT, DELEGATION_ID, LB_ADDRESSES]

VO_SOURCE_NONE = "NONE"
VO_SOURCE_CONFIG_VAR = "CONFIG_VAR"
VO_SOURCE_CONFIG_OPT  = "CONFIG_OPT"
VO_SOURCE_VO_OPT = "VO_OPT"
VO_SOURCE_CERT_EXTENSION = "CERT_EXTENSION"

# String for no default VO
UnspecifiedVO = "unspecified"

def generateVoPath(voName):
  
  # Read the HOME environment variable
  try:
    # Set the User Home directory
    envHome = os.environ['HOME']
  except:
    # Set a empty value due to missing environment variable    
    envHome = ""
    
  # Set the GLITE_WMS_COMMANDS_CONFIG environment variable content as configuration file
  configFile = envHome + os.sep + ".glite" + os.sep + voName.lower() + os.sep + DEFAULT_UI_CLIENTCONFILE

  if not os.path.isfile(configFile):
    # Old approach
    configFile = envHome + os.sep + ".glite" + os.sep + voName.lower() + os.sep + DEFAULT_UI_CONFILE
  
  # Return the Config File
  return configFile

def checkDeprecatedAttributes(ad, path):

  deprecatedWarning = ""
  attributeSeparator = ""
  
  # Show all the warnings for each deprecated attributes
  for deprecatedAttribute in deprecatedAttributes:
  
    # Check if the current deprecated attributes is present outside JDL Default Attributes section
    if(ad.hasKey(deprecatedAttribute)):
      # Add the current attribute to the list of deprecated attributes
      deprecatedWarning += attributeSeparator + deprecatedAttribute;
      
      # Set Comma as attribute separator
      attributeSeparator = ", "
  
  # Show a warning message if deprecated attributes have been found and if it's in debug mode
  if(deprecatedWarning and wmsui_utils.info.debug):

    # Build the debug message
    deprecatedMsg = "Configuration file: " + path + " - " + deprecatedWarning + " attribute(s) no more supported outside JDL Default Attributes section \"JdlDefaultAttributes\""

    #Print message
    wmsui_utils.print_message(False, wmsui_utils.info.logFile, deprecatedMsg)

"""
fill the classad attributes, if missing, from another classad.
The only attributes copied are the configuration ones
"""
def mergeConfigAttributes(sourceAd, destinationAd):

	# Scan all the config attributes inside the AD and if missing
	# find them insede the source AD
	for configAttribute in configAttributes:

		# Check if the attribute is missing in the destination AD
		if(not destinationAd.hasKey(configAttribute)):
		
			# Check if the attribute is present in the source AD
			if(sourceAd.hasKey(configAttribute)):
				
				# Add the Attribute
				destinationAd.setAttributeExpr(configAttribute, sourceAd.delAttribute(configAttribute))

def loadConfiguration(pathUser, pathDefault, voName):

  #Initialise configuration AD	
  info.confAdVo = AdWrapper(1)

  if pathUser:

     # conf file has to be changed from default one
     if not os.path.isfile(pathUser):
        print "Fatal Error: config file not found:\n" + os.path.abspath(pathUser)
        sys.exit (1)
     else:
     	# Create an AD Wrapper for the configuration file
        ad = AdWrapper(1)

        if ad.fromFile(pathUser):
           print "Fatal Error: Unable to parse config file:\n " + os.path.abspath(pathUser)
           sys.exit (1)

	# Dump warning message on deprecated attributes
	checkDeprecatedAttributes(ad, pathUser)

	# Set the Configuration AD	 
	mergeConfigAttributes(ad, info.confAdVo)  
	
	# Set the Configuration file name
        info.confFile = os.path.abspath(pathUser)

  if pathDefault:

     # conf file has to be changed from default one
     if not os.path.isfile(pathDefault):
        print "Fatal Error: config file not found:\n"+os.path.abspath(pathDefault)
        sys.exit (1)
     else:
     	# Create an AD Wrapper for the configuration file
        ad = AdWrapper(1)
        if ad.fromFile(pathDefault):
           print "Fatal Error: Unable to parse config file:\n " + os.path.abspath(pathDefault)
           sys.exit (1)

	# Dump warning message on deprecated attributes
	checkDeprecatedAttributes(ad, pathDefault)

	# Set the Configuration AD	 
	mergeConfigAttributes(ad, info.confAdVo)  
	
  # Override the VO if a valid one has been found
  if(voName):
    # Override in any case the voName
    info.confAdVo.overrideVo(voName);

    err, apiMsg = info.confAdVo.get_error()
    if err:
	    print err

  # Print the configuration file loaded if it's in debug mode
  if info.debug:
    # Print the configuration file loaded
    print "####\nConfiguration file loaded:", info.confFile, info.confAdVo.toLines()


"""
This Method finds where the UI tools have been installed
"""
def check_prefix():
  prefix = ""
  confPrefix=""
  found = 0
  env = ""
  #Those files are mandatory for ui-commandline:
  fileList    = ["glite_wmsui_cmd_var.conf", "glite_wmsui_cmd_err.conf" ]
  #Those files are used by the single Virtual Organisation:
  fileListVo = ["glite_wmsui.conf"]
#  pathList = ['/','/usr/local/etc' , '']

  uipath = ""
  try:
    uipath = os.environ['EMI_UI_CONF']
  except:
    uipath = ""

  pathList = ['/','/usr/local/etc' ,uipath,'']

  etcPath = "etc"
  for path in pathList:

     prefix = path
     if path:
       if path[0] == "$":
           #It's an environment path
           try:
              prefix = os.environ[ path[1:] ]
              env= path

           except:
              pass
     if not prefix:
        pass
     confPrefix = prefix + os.sep + etcPath
#     print "confPrefix=" + str(confPrefix) + "\n"
     #Look for UI configuration files:
     found = 1

#     print "found=" + str(found) + "\n"

     for file in fileList:

#	  print "file=" + str(confPrefix + os.sep + file) + "\n"

          if not os.path.isfile(confPrefix + os.sep + file):
             found = 0
             break
     if found ==1:
        break
  if not found:
      if env:
          errMsg = "Fatal Error: " + env + " environment found but one or more of the following files are missing:\n"
          for file in fileList:
             errMsg = errMsg + file +" "
      else:
          errMsg = "Fatal Error: Unable to find userinterface configuration files"
      print errMsg
      sys.exit(1)
  info.prefix= prefix
  """
  Read version value from 'glite_wmsui_cmd_err'
  """
  try:
  #if not version:
     f=open( confPrefix + os.sep +"glite_wmsui_cmd_err.conf"  ,'r' )
     line=f.readline().strip()
     while line:
       line=line.strip()
       try:
         tag , ver = line.split("=")
         if tag.strip()=="UI_VERSION":
            wmsui_utils.info.version = ver
            break
       except:
         pass
       line=f.readline()
     f.close()
     if wmsui_utils.info.version =="":
        raise AttributeError
  except:
     print "Fatal Error: Unable find Version attribute in Ui  configuration file: \n" + confPrefix + os.sep +"glite_wmsui_cmd_err.conf"
     sys.exit(1)
  ad = AdWrapper(1)
  info.confFile = confPrefix + os.sep +fileList[0]
  if not os.path.isfile (info.confFile ):
      errMsg = "Fatal Error: Unable to find UI  default configuration file:\n " + info.confFile
      print errMsg
      sys.exit(1)
  if ad.fromFile ( info.confFile ):
	err, apiMsg = ad.get_error()
	if err:
		errMsg = "Fatal Error: Unable to parse UI default configuration file:\n " + info.confFile+".\n" + apiMsg
		print errMsg
		sys.exit(1)
  info.confAd = ad
  return prefix


"""
This method parses the command-line option
args  - option typed byt he user (typically sys.args[1:])
short - string made of chars. A semicolon (:) indicates that the previous option has to retreive an additional parameter
        this options have to be typed preceded by "-" in the command line
long  - list made of string. An equal char at the end of the string indicates that this option has to retreive an additional parameter
        this options have to be typed preceded by "--" in the command line
"""
def checkOpt(arg , short , long):
   #Make a deep copy of the arg
   args =[]
   for i in arg:
      args.append(i)
   #Pre-Processing param: help and version
   for preOpt in ["--help", "--version"]:
      if  preOpt in args:
          return [0,[preOpt],[], [] ]
   optNul = []
   optPar  = []
   #SHORT options check
   while (short):
      next=""
      #take the option letter and check
      opt=short[0]
      short=short[1:]
      if short:
         #cut the letter
         next=short[0]
      if (opt <"a") or (opt>"z"):
         return [   1,[],[],[]   ]
      if next==":":
         #option required
         short=short[1:]
         optPar.append("-" + opt)
      else:
         #option not required
         optNul.append("-" + opt)
   #LONG options check
   for opt in long:
      if opt[-1] == "=" :
         #option required
         optPar.append("--"+opt[:-1])
      else:
         optNul.append("--" + opt)
   options = []  #list of options required
   value   = []  #list of values typed for each required option
   extra   = []  #list of extra parameters found
   error   = []
   repeated   = []  #list of options repeated
   missing    = []     #list of options specified without required par
   unknown    = []  #list of unknown options
   sysExit=0
   while (args):
      next=""
      short="---"
      #get the item on the list
      opt=args.pop(0)
      if opt.startswith("-"):
         #it's an option
         if (opt.startswith("--")) and (len(opt)>2):
            short=opt[1:3]
         if (len(opt)>2) and opt[1]!="-":
            opt="-"+opt
         if args:
            next=args[0]
            if next.startswith("-"):
               #It's an option, not a value
               next=""
         #check if option repeated
         #if (opt in options) or (short in options):
         #check if option allowed
         if opt in  optNul: #option without par
             options.append(opt)
             value.append("")
         elif opt in optPar: #option with par
            if next: #syntax OK
               options.append(opt)
               value.append(next)
               args=args[1:]
            else:    #syntax error: required parameter
               missing.append(opt)
               sysExit=1
               break
         else:
            unknown.append(opt)
            sysExit=1
            break
      else:
         #Extra option typed: no-more option allowed
         if args:
            next=args[0]
            if next.startswith("-"):
               #It's an option, not allowed after an extra
               error.append(next)
               args=args[1:]
               sysExit=1
               break
         extra.append(opt)
      #check for item value (required??) AND (if required .... is present???)
   for rep in  repeated:
      errMsg('Error','UI_REPEATED_OPT',rep)
   for mis in missing:
       errMsg('Error','UI_ARG_MISS',mis)
   for un in unknown:
       errMsg('Error','UI_WRONG_OPT', un)
   for err in error:
       errMsg('Error','UI_OPT_POS',err)
   return sysExit, options, value, extra

"""
This Method parses the file and prompt for info
Definite VO Configuration file name found: check for file properties:
- Existence
- Ad parsing
- Semantic cohexistence between file name/path and VO file attribute ( if present )
- Semantic cohexistence between file properties and (if specified) JDL VO attribute
"""
def parseVo(voSrc, configFile, voName, eMsg, override):

	# Check the source of the VO Name
	if (voSrc == VO_SOURCE_CERT_EXTENSION or voSrc == VO_SOURCE_VO_OPT) and not configFile :
		# Only vo is provided, generate file name:
		configFile = generateVoPath(voName)	

	# Retrieve the configuration directory
	confDir = os.path.dirname(configFile)

	# Look for configuration Vo file
	if not os.path.isfile(configFile):
	
		if voSrc == VO_SOURCE_CERT_EXTENSION or voSrc == VO_SOURCE_VO_OPT:	
			# In these cases vo file was autogenerated user config file may not be there
			return [0, voName, 0, configFile]

		else: 
			errMsg(eMsg,'UI_FILE_NOT_FOUND', configFile)
			return [1, "", 0, configFile]
	
	# Parsing Ad VO file
	ad = AdWrapper(1)
	if ad.fromFile(configFile):
		errMsg(eMsg , "UI_JDL_ADD_ERROR", "Unable to parse Vo conf file (not a valid classad):\n " + configFile )
		return [1, "", 0, configFile] 

		
	#The VirtualOrganisation Attribute must match with the specified voName
	virtualOrganisation = ad.getVirtualOrganisation();
		
	# Check if the Virtual Organisation is present inside the AD
	if virtualOrganisation:

		# Check if the VO name has been set
		if voName:
			#The VirtualOrganisation Attribute must match with the specified voName
			if virtualOrganisation.lower()!=voName.lower():
				if override == 1:
					virtualOrganisation = voName
				else:
					errMsg(eMsg,'UI_JDL_VO_MATCH', voName , val ,configFile)
					return [1, "", 0, configFile]
		# SUCCESS
		return [0, virtualOrganisation , ad, configFile ]
	else:
		errMsg(eMsg,'UI_JDL_ADD_ERROR', "Missing VirtualOrganisation attribute inside JDL Default Attributes section \"JdlDefaultAttributes\" of configuration file " + configFile )
		return [1, "", 0, configFile]

"""
 This Method check the existence of the Group file
 if not found then tries to find the default cfg file
 it returns a list as follows:
 [1, <configure file>] if not found
 [0, <configure file>] on success

 Config file looking for priority:
  1)	--config <file> option
  2)	'GLITE_WMS_COMMANDS_CONFIG' environment variable
  3)	default configuration file (/home/<user>/.glite/<vo_name>)

"""
def checkConf(conf, virtualOrg, logPath):

  override = 1

  voName = UnspecifiedVO

  vomsrc = ""
  
  src = VO_SOURCE_NONE;

  configFile = ""
  cfDefault = ""
  
  sys_exit = 0

  # Trace 
  if(conf and virtualOrg):
  
	wmsui_utils.errMsg("The following options cannot be specified together:\n" + \
		 conf + "\n" + \
		 virtualOrg + "\n\n", wmsui_utils.info.logFile)
        wmsui_utils.exit(1)
  
  # Read the GLITE_WMS_COMMANDS_CONFIG environment variable
  try:
    # Set the WMS Commands Config file
    envCommandsConfig = os.environ['GLITE_WMS_COMMANDS_CONFIG']
  except:
    # Set a empty value due to missing environment variable    
    envCommandsConfig = ""

  # Read the X509_USER_PROXY environment variable
  try:
    # Set the User Proxy filename from the environment variable
    proxy_file_name = os.environ['X509_USER_PROXY']
  except:
    # Set the default User Proxy filename 
    proxy_file_name = os.sep + "tmp" + os.sep + "x509up_u"+ repr(os.getuid())

  # Read the Proxy file if it exists		  
  if os.path.isfile(proxy_file_name):

    #Try to look for the default Vo inside the user proxy
    printERRORS=1
    err, voName = checkVomsExtension(proxy_file_name, printERRORS)
    
    # Check for errors
    if err:
	wmsui_utils.errMsg(err, wmsui_utils.info.logFile)
        wmsui_utils.exit(1)

    if voName:
      # Set the source of the voName
      src = VO_SOURCE_CERT_EXTENSION
      vomsrc = "proxy certificate extension"
      
  """
  Options Parsing
  """
  if(virtualOrg and src != VO_SOURCE_NONE):
  
    # VO name forcing ignored
    wmsui_utils.print_message (False, wmsui_utils.info.logFile, "Warning - --vo option ignored" )
    
  elif(virtualOrg):
  
    # SCR is definitely NONE
    voName = virtualOrg
    
    src = VO_SOURCE_VO_OPT
    vomsrc = "--vo option"

    # Print a debug message    
    if wmsui_utils.info.debug:
      debugMsg = "#### "+ time.ctime() + " Debug Message ####\n" + \
	         "VO Read from --vo option\n\n#### End Debug ####\n"
   
      #Print message
      wmsui_utils.print_message(False, wmsui_utils.info.logFile, debugMsg)
    
      # Set the config file
      configFile = wmsui_utils.info.prefix + os.sep + "etc" + os.sep + virtualOrg.lower() + os.sep + DEFAULT_UI_CLIENTCONFILE
    
  elif(conf):
  
    if (src == VO_SOURCE_NONE and wmsui_utils.info.debug):
      debugMsg = "#### "+ time.ctime() + " Debug Message ####\n" + \
	         "VO Read from --config option\n\n#### End Debug ####\n"
   
      #Print message
      wmsui_utils.print_message(False, wmsui_utils.info.logFile, debugMsg)
    
    # Set the config file
    configFile = conf
	
    src = VO_SOURCE_CONFIG_OPT
    vomsrc = "--config option"
    
  elif envCommandsConfig:

    if (src == VO_SOURCE_NONE and wmsui_utils.info.debug):
      debugMsg = "#### "+ time.ctime() + " Debug Message ####\n" + \
	         "VO Read from ENV option\n\n#### End Debug ####\n"
   
      #Print message
      wmsui_utils.print_message(False, wmsui_utils.info.logFile, msg)

    src = VO_SOURCE_CONFIG_VAR;
    vomsrc = "GLITE_WMSUI_CONFIG_VO env variable"
   
    # Set the GLITE_WMS_COMMANDS_CONFIG environment variable content as configuration file
    configFile = envCommandsConfig

  elif(src == VO_SOURCE_NONE):	
  
    print "checkConf\nEmpty value: Unable to find any both VirtualOrganisation and any configuration file"
    sys.exit(1)
  
  err, voName, info.confAdVo, configFile = parseVo(src, configFile, voName, "Error", override)
  
  if err == 0 and not info.confAdVo:
  	configFile = ""
  
  if wmsui_utils.info.debug and voName:
    # Print Info
    msg =  "VirtualOrganisation value :" + voName
    wmsui_utils.print_message(False, wmsui_utils.info.logFile, msg)

  # Build the default config file it a VO name has been found
  if(voName):	  
	  
    # Build the default config file 
    cfDefault = wmsui_utils.info.prefix + os.sep + "etc" + os.sep + voName.lower() + os.sep + DEFAULT_UI_CLIENTCONFILE

    # Check if the file exists
    if not os.path.isfile (cfDefault):
      # Remove the path
      cfDefault = ""

  # Load the configuration
  loadConfiguration(configFile, cfDefault, voName)
  
  if wmsui_utils.info.debug:
	  # Successfully Update needed Info
	  msg = "Selected Virtual Organisation name (from "+ vomsrc + "): " + voName

	  msg = "#### "+ time.ctime() + " Debug Message ####\n" + msg +"\nVOMS configuration file successfully loaded:\n" \
	  + configFile + info.confAdVo.toLines() +  "\n#### End Debug ####\n"

	  #Print VOMS message
	  wmsui_utils.print_message(False, wmsui_utils.info.logFile, msg)

  # Check if the log path has been set
  if logPath:
  
    # Create the log filename
    logFile = os.path.abspath (logPath)

    # Remove the log file if it exists	
    if os.path.isfile ( logFile):
      try:
        # Remove the log file
        os.remove(logFile)
      except:
        errMsg ( "Warning" , "UI_RM_FILE", logFile )
  else :
    # Create a new log file
    logFile = wmsui_utils.create_err_log ( info.prgname )

  # Rename the old logFile with the new value (if needed)
  if info.logFile:
   if info.logFile != logFile:
     try:
       shutil.copyfile( info.logFile, logFile  )
     except:
       errMsg('Error','UI_WRITE_FILE' , logFile   )
       wmsui_utils.exit(1)
     try:
       os.remove(info.logFile)
     except:
       errMsg("Warning",'UI_RM_FILE' , info.logFile   )
     info.logFile = logFile

  # Return the Virtual Organisation name
  return voName
  
"""
def check_outFile(out_file,*submit)
check The specified output file target
"""
def check_outFile(out_file,*submit):
  TBremoved=0
  out_file=os.path.abspath(out_file)
  sind=out_file.rfind(os.sep)
  if not os.path.isdir(out_file[:sind]): #the path doesn't exist
          wmsui_utils.errMsg("Error","UI_DIR_NOT_FOUND",out_file[:sind])
          wmsui_utils.exit(1)
  info.outFile = out_file
  if os.path.isfile(out_file): #the file already exists
    TBremoved=1
    if submit:
       f=open(out_file)
       line=f.readline()
       line = line.strip()
       f.close()
       if line == "###Submitted Job Ids###":
           return [0,out_file,0]
    wmsui_utils.errMsg("Warning","UI_FILE_EXISTS",out_file)
    if info.noint: #No-int active
       wmsui_utils.errMsg("Warning","UI_SKIP_QUESTION","overwriting the File "+out_file)
       return [0,out_file,1] #the file will be removed
    else:
       question="Do you want to overwrite?"
       answ=wmsui_utils.questionYN(question)
       print ""
       if not answ: #NO answered
           print "bye"
           return [0,"",0]
  #if submit: #The file won't be removed
  #     TBremoved=1
  return [0,out_file,TBremoved]



#############
#This method checks the user proxy certificate
#and if necessary creates it
#############
def check_proxy(*valid) :
	if valid:
		valid=valid[0]
		hours, minutes  = valid.split(":")
		secondsLeft = int(hours)*60*60 + int(minutes)*60
	uid = os.getuid()
	pid = os.getpid()
	proxy_file_name = ''
	INST = info.prefix
	if not INST:
		exit(1)
	#Initialize proxy certificate file name
	try:
		proxy_file_name = os.environ['X509_USER_PROXY']
	except:
		proxy_file_name = os.sep + "tmp" + os.sep+ "x509up_u" + repr(uid)
	#Check if Proxy certificate already exists
	#voms =  info.confAdVo.getStringValue("VirtualOrganisation")[0]
	if os.path.isfile(proxy_file_name):
		#Retrieve Proxy Information ( new Approach )
		uc = UCredential (proxy_file_name)
		timeleft = uc.getExpiration()  - int( time.time() ) -time.timezone
		err = uc.get_error()
		if err != "":
			#ERROR
			errMsg ( "Error" , "API_NATIVE_ERROR" , "UcWrapper::getExpiration"  , err )
			wmsui_utils.exit (1)
		info.issuer =uc.getIssuer()
		if not valid:
			# timeleft should be at least 20 minutes
			secondsLeft = 20*60
			valid = "00:20"
		if timeleft>secondsLeft:
			printERRORS=0
			return checkVomsExtension(proxy_file_name,printERRORS)[0]
		elif timeleft>0:
			if info.prgname=="glite-job-submit":
				errMsg("Error",'UI_PROXY_DURATION' , valid )
				return 1
		else:
			errMsg("Error",'UI_PROXY_EXPIRED' )
			return 1

	else:
		# The Proxy Does not exist
		errMsg("Error",'UI_PROXY_NOT_FOUND')
		return 1

"""
Look fot the VOMS (This method is called bt Check_proxy)
"""
def checkVomsExtension  (proxyfile, printERRORS):
	"""
	Retrieve vomses from proxy and check whether it is contained
	"""
	uc = UCredential(proxyfile)
	vomses = uc.getVoNames()
	
	if wmsui_utils.info.debug and printERRORS:
		# No voms in certificate: print error
		printableError = uc.get_error()
		if printableError:
			errMsg( 'Warning' , "UI_NO_VOMS",printableError)
	if vomses:
		return [0, vomses[0]]
	else:
		return [0,""]

"""
Checks for native external API
"""
def checkError (apiName , inst, exit = 0):
	err , apiMsg = inst.get_error ()
	if err==0:
		return err
	em = "Error"
	if exit==0:
		em="Warning"
	if err==1:
		#ERROR
		errMsg ( em , "API_NATIVE_ERROR" , apiName  , apiMsg )
		if exit:
			wmsui_utils.exit (1)
	else:
		#WARNING
		errMsg ( em , "API_NATIVE_ERROR" , apiName  , apiMsg )
	return err


"""
check and parse values passed with "--from" and "--to" options
"""
def checkFromTo ( fromT , toT):
	parsed =[]
	# time.time expresses the time in UTC
	# now is expressed in DST
	now = time.time()
	opt = "--from"
	for val in [fromT , toT]:
		if val:
			# Those Values are expressed in DST
			h = m = "00"
			M = time.strftime("%m")
			D = time.strftime("%d")
			Y = time.strftime("%Y")
			seps = val.split(":")
			l = len (seps)
			if l ==2:
				h, m = seps
			elif l ==4:
				M,D,h,m = seps
			elif l ==5:
				M ,D ,h, m ,Y = seps
			else:
				errMsg ( "Error" , "UI_ARG_MISMATCH" , opt   )
				return []
			if len(Y)==2:
				#Adjust Year value (if needed)
				try:
					Y =str (  time.strptime( Y,"%y")[0]  )
				except:
					errMsg ( "Error" , "UI_ARG_MISMATCH" , opt  )
					return []
			#Checking values lengths
			if len(h)!=2 or len(m)!=2 or len(M)!=2 or len(D)!=2 or len(Y)!=4:
				errMsg ( "Error" , "UI_ARG_MISMATCH" , opt )
				return []
			finalString = M + D + h +  m + Y
			formatString = "%m%d%H%M%Y"
			try:
				# value limits to be checked in DST
				valueToCheck= ( int (time.mktime(  time.strptime( finalString  , formatString)  )   ) )
				# Append the value to query in UTC
				parsed.append( valueToCheck )
			except ValueError:
				errMsg ( "Error" , "UI_ARG_MISMATCH" , opt )
				return []
		else:
			parsed.append( 0 )
		opt="--to"
	if parsed[0]>now:
		# FROM is bigger than NOW
		errMsg ( "Error" , "UI_ARG_OUT_OF_LIMIT" , "--from" )
		return []
	elif parsed[0]>parsed[1] and parsed[1]:
		# FROM is bigger than TO
		errMsg ( "Error" , "UI_ARG_OUT_OF_LIMIT" , "--from" )
		return []
	else:
		return parsed



"""
This Method inizializes the ouput variables
checking wheter -noint option has been set or not
"""
def check_noint(argv):
  set=0
  #There is the following set cases
  if  ("--debug" in argv) or("-debug" in argv):
    set=1
    info.debug=1
    info.logFile=wmsui_utils.create_err_log(info.prgname)
  if ("--noint" in argv ) or ("-noint" in argv ):
    set=2
    info.noint = 1
    info.logFile=wmsui_utils.create_err_log( info.prgname)
  else:
    print "" #Write an empty line
  if ( ("--nomsg" in argv) or ("-nomsg" in argv) ):
#     if output=="glite-job-submit":
     if info.prgname=="glite-job-submit":
        set=3
        info.logFile=wmsui_utils.create_err_log(info.prgname)
  #Redirect the error/debug logFile into a specified path
  if  ("--logfile" in argv) or ("-logfile" in argv):
       info.setLog=1
       info.logFile=wmsui_utils.create_err_log(info.prgname)
       set= 1
  if set>0:  #Could be debug, noint or nomsg (submit)
       #Print Information program inside the file
       printHeader(argv)

"""
This method is used by check_noint
This Method Prints the Header in the Log File
"""
def printHeader( argv):
    message="*****************************\n"
    message= message + time.asctime(time.gmtime(time.time()))
    message= message +"\n\nFunction Called:"
    message= message + "\n"+argv[0]
    message= message +"\n\nOptions specified:"
    for i in range (1,len(argv)):
      if (    (argv[i-1])[0]!="-"  ) or  (   (argv[i])[0]=="-"    )  :
         message= message +"\n"
      message= message +argv[i]+"   "
    message = message + "\n*****************************"
    message = message + "\n---No Errors found---"
    if info.logFile:
      try:
         f=open(  info.logFile ,'w')
         f.write(message + "\n")
      except:
	logFile = info.logFile
	info.logFile = ""
	errMsg('Error','UI_WRITE_FILE',   logFile )
	sys.exit(1)
