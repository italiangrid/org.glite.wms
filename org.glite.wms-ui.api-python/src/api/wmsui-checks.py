#! /usr/bin/env python2.2
"""
***************************************************************************
    filename  : wmsui-checks.py
    author    : Alessandro Maraschini
    email     : egee@datamat.it
    copyright : (C) 2003 by DATAMAT
***************************************************************************
//
// $Id$
//
"""


import wmsui-utils
from wmsui-utils import info
from wmsui-utils import errMsg
import os #check conf
import shutil #copyfile
import sys
import time #printHeader
from glite_wmsui_AdWrapper import AdWrapper
from glite_wmsui_UcWrapper import UCredential


# String for no default VO
UnspecifiedVO = "unspecified"

"""
This Method finds where the UI tools have been installed
"""
def check_prefix():
  sep="/"
  prefix = ""
  confPrefix=""
  found = 0
  env = ""
  #Those files are mandatory for ui-commandline:
  fileList    = ["glite_wmsui_cmd_var.conf", "glite_wmsui_cmd_err.conf" ]
  #Those files are used by the single Virtual Organisation:
  fileListVo = ["glite_wmsui.conf"]
  pathList = ['$GLITE_WMS_LOCATION' , '$GLITE_LOCATION' , '/opt/glite','/usr/local' , '']
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
     confPrefix = prefix + sep + etcPath
     #Look for UI configuration files:
     found = 1
     for file in fileList:
          if not os.path.isfile(confPrefix+sep+file):
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
     f=open( confPrefix + sep +"glite_wmsui_cmd_err.conf"  ,'r' )
     line=f.readline().strip()
     while line:
       line=line.strip()
       try:
         tag , ver = line.split("=")
         if tag.strip()=="UI_VERSION":
            wmsui-utils.info.version = ver
            break
       except:
         pass
       line=f.readline()
     f.close()
     if wmsui-utils.info.version =="":
        raise AttributeError
  except:
     print "Fatal Error: Unable find Version attribute in Ui  configuration file: \n" + confPrefix + sep +"glite_wmsui_cmd_err.conf"
     sys.exit(1)
  ad = AdWrapper(1)
  info.confFile = confPrefix + sep +fileList[0]
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
configuration file prioritiy:
0).	(high priority)	DefaultVoName (from proxy certificate)
1) 	--config or --vo option (exclusive)
2) 	environment variable
3) 	VirtualOrganisation attribute specified in JDL (if submit or list match)
4) 	default value (from UI config file)
eventually returns the value of the found VO
"""
def checkConfVo(conf , virtualOrg, *jobad):
	"""
	Override flag, 0 do not override, 1 override
	"""
	override = 0
	#STATIC Variables
	sep = "/"
	voDefaultName = "glite_wmsui.conf"
	voErrMsg ="Error"
	"""
	- Name of Vo
	- Directory
	- full path of VO file
	"""
	voName =""
	confDir=""
	final = ""
	"""
	Default VirtualOrganisation from existing Certificate (if present)
	"""
	defaultVo = ""
	defaultAd =""
	vomsrc=""
	"""
	Initialisations and Checks
	"""
	if conf and virtualOrg:
			# those options   cannot match togheter
			errMsg('Warning', "UI_ARG_EXCLUSIVE" , "--config-vo", "--vo")
			wmsui-utils.exit(1)
	#JobAd init
	if jobad:
		jobad=jobad[0]
	#This value is stored in ExpDagAd class
	if wmsui-utils.info.TYPE==0:
		#Normal Job
		VIRTUAL_ORGANISATION = "VirtualOrganisation"
	else:
		#Dag
		VIRTUAL_ORGANISATION = 1
	if jobad:
		jobadHasKey = jobad.hasKey("VirtualOrganisation")
	else:
		jobadHasKey=0
	#   ENV var init
	try:
			env = os.environ['GLITE_WMSUI_CONFIG_VO']
	except:
			env = ""

	# PROXY Certificate File
	try:
			proxy_file_name = os.environ['X509_USER_PROXY']
	except:
			proxy_file_name = '/tmp/x509up_u'+ repr(os.getuid())
	if os.path.isfile(proxy_file_name):
		#Try to look for the default Vo inside the user proxy
		printERRORS=1
		err, defaultVo=checkVomsExtension(proxy_file_name, printERRORS)
	"""
	Options Parsing
	"""
	# --vo option specified
	if virtualOrg:
		confDir = wmsui-utils.info.prefix + sep + "etc" + sep +virtualOrg.lower()
		final = confDir +  sep + voDefaultName
		vomsrc = "--vo option"
		voName=virtualOrg
	#   --configVo option specified
	elif conf:
			vomsrc = "--config-vo option"
			err , voName , ad = parseVo (voName , confDir , conf , override, "Warning")
			if not err:
				final = conf
	# ENV variable used to specify VO file
	elif env:
			vomsrc = "GLITE_WMSUI_CONFIG_VO env variable"
			err , voName , ad = parseVo (voName , confDir , env , override, "Warning")
			if not err:
				final = env
	#   VirtualOrganisation attribute specified in JDL file
	elif jobad and jobadHasKey:
		if wmsui-utils.info.TYPE==0:
			#Normal Job
			voName = jobad.getStringValue(VIRTUAL_ORGANISATION )[0]
		else:
			#Dag
			voName = jobad.getStringValue(VIRTUAL_ORGANISATION)
		confDir = wmsui-utils.info.prefix + sep + "etc" + sep +voName.lower()
		final = confDir +  sep + voDefaultName
		vomsrc="JDL"

	# DefaultVo attribute specified in configuration file (last option)
	elif info.confAd.hasKey("DefaultVo"):
		try:
			VoFromConf = info.confAd.getStringValue("DefaultVo")[0]
			if (not defaultVo) or (VoFromConf != UnspecifiedVO):
				voName = VoFromConf
				confDir = wmsui-utils.info.prefix + sep + "etc" + sep + voName.lower()
				final = confDir +  sep + voDefaultName
				vomsrc = "UI conf file"
		except:
			err, erMsg = info.confAd.get_error()
			if err:
				errMsg('Warning','UI_JDL_ADD_ERROR', erMsg)
	"""
	Default VO Parsing
	"""
	if defaultVo:
		# Set Vo file's properties if not yet done
		if not final:
			voName=defaultVo
			confDir= wmsui-utils.info.prefix + sep + "etc" + sep +defaultVo.lower()
			final=confDir +sep + voDefaultName
			vomsrc = "proxy certificate extension"
	elif voName==UnspecifiedVO:
		# Only a not allowed vo has been found
		errMsg( "Error"  , "UI_NO_VOMS" , "Unable to determine a valid user's VO" )
		wmsui-utils.exit(1)
	"""
	Final Check
	"""
	if final :
		# Override message prompt to user only if:
		# Vo has been specified somehow (vomsrc)
		# defaultVo has been found (defaultVo)
		# VO name exists but is different from DefaultVo (voName!=defaultVo)
		if  (vomsrc ) and (defaultVo )and (voName) and ( voName.lower()!=defaultVo.lower() ):
			errMsg('Warning', "UI_VOMS_OVERRIDE" , voName, vomsrc , defaultVo )
			if wmsui-utils.info.noint:
					wmsui-utils.exit(0)
			elif vomsrc == "UI conf file":
				# no need to ask question
				pass
			else:
				question = "Do you want to continue?"
				answ = wmsui-utils.questionYN(question)
				if not answ: #NO answered
					print "bye"
					wmsui-utils.exit(0)
			if (vomsrc == "--config-vo option") or (vomsrc == "GLITE_WMSUI_CONFIG_VO env variable") :
				override = 1
			else:
				# Change name to default ONE
				confDir= wmsui-utils.info.prefix + sep + "etc" + sep +defaultVo.lower()
				final=confDir +sep + voDefaultName
		if defaultVo:
			voName=defaultVo
			vomsrc = "proxy certificate extension"
		# PARSE EVENTUALLY VO FILE:
		err , voName , info.confAdVo = parseVo (voName , confDir , final , override, "Error")
		if err:
			wmsui-utils.exit(1)
	else:
		errMsg( "Error"  , "UI_NO_VOMS" , "Unable to determine a valid user's VO" )
		wmsui-utils.exit(1)

	# Successfully Update needed Info
	msg = "Selected Virtual Organisation name (from "+ vomsrc+ "): "+ voName
	if wmsui-utils.info.debug:
		msg ="#### "+ time.ctime() + " Debug Message ####\n" + msg +"\nVOMS configuration file successfully loaded:\n" \
		+ final + info.confAdVo.toLines() +  "\n#### End Debug ####\n"
	#Print VOMS message
	wmsui-utils.print_message( wmsui-utils.info.logFile,   msg )
	# Cohexistence between properties and VO
	if jobad:
		if jobad.hasKey("VirtualOrganisation"):
			jobad.removeAttribute (VIRTUAL_ORGANISATION)
		jobad.setAttributeStr (VIRTUAL_ORGANISATION , voName)
	return voName

"""
This Method is used by checkConfVo
it parses the file and prompt for info
Definite VO Configuration file name found: check for file properties:
- Existence
- Ad parsing
- Semantic cohexistence between file name/path and VO file attribute ( if present )
- Semantic cohexistence between file properties and (if specified) JDL VO attribute
"""
def parseVo( voName , confDir , final , override, eMsg ):
	# Look for configuration Vo file
	if confDir:
		if not os.path.isdir( confDir ):
			errMsg(eMsg,'UI_NO_VO_CONF_INFO', voName )
			return [1,0,0]
	if not os.path.isfile( final ):
			errMsg(eMsg,'UI_FILE_NOT_FOUND', final )
			return [1,0,0]
	# Parsing Ad VO file
	ad = AdWrapper(1)
	if ad.fromFile ( final):
		errMsg(eMsg , "UI_JDL_ADD_ERROR", "Unable to parse Vo conf file (not a valid classad):\n " +final )
		return [1,0,0]
	#The VirtualOrganisation Attribute must match with the specified voName
	val = ad.getStringValue ( "VirtualOrganisation")
	if val:
		val = val[0]
		if voName:
			if val.lower()!=voName.lower():
				if override == 1:
					val = voName
				else:
					errMsg(eMsg,'UI_JDL_VO_MATCH', voName , val ,final )
					return [1,0,0]
		# SUCCESS
		return [0, val , ad ]
	else:
		errMsg(eMsg,'UI_JDL_ADD_ERROR', ad.get_error()[1] )
		return [1,0,0]

"""
                 checkConf
 This Method check the existence of the Group file
 if not found then tries to find the default cfg file
 it returns a list as follows:
 [1, <configure file>] if not found
 [0, <configure file>] on success

 Config file looking for priority:
  1)	--config <file>       option
  2)	'GLITE_WMSUI_CONFIG_VAR'    environment variable
  3)	default configuration file (   <prefix>/etc/glite_wmsui_cmd_var.conf   )

"""
def checkConf( conf , logPath ):
  #This point to the final conf file
  sys_exit= 0
  final = ""
  #   ENV var:
  try:
    env = os.environ['GLITE_WMSUI_CONFIG_VAR']
  except:
    env = ""
  #   --config found
  if conf:
     final = conf
  #   Env found
  elif env:
    final = env
  # Do Nothing, keeps with default one (already set???) TBD CHECK
  else:
    pass
  if final:
     # conf file has to be changed from default one
     if not os.path.isfile( final ):
        print "Fatal Error: config file not found:\n"+os.path.abspath(final)
        sys.exit (1)
     else:
        ad = AdWrapper(1)
        if ad.fromFile ( final):
           print "Fatal Error: Unable to parse config file:\n " + os.path.abspath(final)
           sys.exit (1)
        info.confAd  = ad
        info.confFile = os.path.abspath( final )
        if info.debug:
           print "####\nConfiguration file loaded:" , final ,ad.toLines()

  if logPath:
     logFile = os.path.abspath (logPath)
     if os.path.isfile ( logFile):
         try:
           os.remove(logFile)
         except:
           errMsg ( "Warning" , "UI_RM_FILE", logFile )
  else :
        logFile = wmsui-utils.create_err_log ( info.prgname )

  # Rename the old logFile with the new value (if needed)
  if info.logFile:
   if info.logFile != logFile:
     try:
       shutil.copyfile( info.logFile, logFile  )
     except:
       errMsg('Error','UI_WRITE_FILE' , logFile   )
       wmsui-utils.exit(1)
     try:
       os.remove(info.logFile)
     except:
       errMsg("Warning",'UI_RM_FILE' , info.logFile   )
     info.logFile = logFile




"""
def check_outFile(out_file,*submit)
check The specified output file target
"""
def check_outFile(out_file,*submit):
  TBremoved=0
  out_file=os.path.abspath(out_file)
  sind=out_file.rfind('/')
  if not os.path.isdir(out_file[:sind]): #the path doesn't exist
          wmsui-utils.errMsg("Error","UI_DIR_NOT_FOUND",out_file[:sind])
          wmsui-utils.exit(1)
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
    wmsui-utils.errMsg("Warning","UI_FILE_EXISTS",out_file)
    if info.noint: #No-int active
       wmsui-utils.errMsg("Warning","UI_SKIP_QUESTION","overwriting the File "+out_file)
       return [0,out_file,1] #the file will be removed
    else:
       question="Do you want to overwrite?"
       answ=wmsui-utils.questionYN(question)
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
	file_path_sep = '/'
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
		proxy_file_name = '/tmp/x509up_u'+repr(uid)
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
			wmsui-utils.exit (1)
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
	if wmsui-utils.info.debug and printERRORS:
		# No voms in certificate: print error
		printableError=uc.get_error()
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
			wmsui-utils.exit (1)
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
				valueToCheck= (  time.timezone + int (time.mktime(  time.strptime( finalString  , formatString)  )   ) )
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
    info.logFile=wmsui-utils.create_err_log(info.prgname)
  if ("--noint" in argv ) or ("-noint" in argv ):
    set=2
    info.noint = 1
    info.logFile=wmsui-utils.create_err_log( info.prgname)
  else:
    print "" #Write an empty line
  if ( ("--nomsg" in argv) or ("-nomsg" in argv) ):
#     if output=="glite-job-submit":
     if info.prgname=="glite-job-submit":
        set=3
        info.logFile=wmsui-utils.create_err_log(info.prgname)
  #Redirect the error/debug logFile into a specified path
  if  ("--logfile" in argv) or ("-logfile" in argv):
       info.setLog=1
       info.logFile=wmsui-utils.create_err_log(info.prgname)
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
