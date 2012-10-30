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
    filename  : wmsui_api.py
    author    : Alessandro Maraschini, Alvise Dorigo
    email     : alvise.dorigo@pd.infn.it
    copyright : (C) 2003 by DATAMAT
***************************************************************************
//
// $Id: wmsui_api.py,v 1.7.2.1.2.5.2.5 2012/04/21 09:39:51 adorigo Exp $
//
"""
import os
import time
from threading import Thread
#import Thread
import socket
import wmsui_utils
from wmsui_utils import exit
from wmsui_utils import errMsg
from wmsui_utils import dbgMsg
from glite_wmsui_AdWrapper import AdWrapper
from glite_wmsui_AdWrapper import DagWrapper
from glite_wmsui_LbWrapper import Eve
from glite_wmsui_LbWrapper import Status
import random # for  TCP port range

"""
Initialise the LB Events Names variables
"""
events_names = [eventName.replace("_"," ").capitalize() for eventName in Eve.getEventsNames()]
EVENT_ATTR_MAX = len(events_names)

"""
Initialise the LB Events Codes variables
"""
events_codes = [eventCode.replace("_"," ").capitalize() for eventCode in Eve.getEventsCodes()]
EVENT_CODE_MAX = len(events_codes)

"""
Initialise the LB States Names variables
"""
states_names = [statusName.replace("_"," ").capitalize() for statusName in Status.getStatesNames()]
states_names.append("Status")
states_names.append("Status Code")
states_names.append("Hierarchy")
STATE_ATTR_MAX = len(states_names)

"""
Initialise the LB States Codes variables
"""
states_codes = [statusCode.replace("_"," ").capitalize() for statusCode in Status.getStatesCodes()]
STATE_CODE_MAX = len(states_codes)

# Separator between jdl attributes:
sep ="==========================================================================\n"
colsep = "\n      "
DSTime = " " + time.strftime("%Z")
evesep = "\n".ljust(14)  +  "***\n"

def getEvents(jobid, level):
	"""getEvents: Return a list of Events for a single Job ID"""

	# Initialise the Events output array
	events = []

	# Create the LB Wrapper for the Job ID
	wrapEvent = Eve(jobid.jobid)
	
	# Check for Errors
	err , apiMsg = wrapEvent.get_error ()
	if err:
		# Print the error and terminate
		raise Exception(apiMsg)
		
	# Retrieve the number of logged events 
	eventsNumber = wrapEvent.getEventsNumber()

	# Retrieve the list of attributes for each logged event
	for eventNumber in range(eventsNumber):
	
		#Retrieve the list of attributes for the current event
		eventAttributes = wrapEvent.getEventAttributes(eventNumber)
		
		# Check for Errors
		err , apiMsg = wrapEvent.get_error ()
		if err:
			# Print the error and terminate
			wmsui_utils.errMsg("**** Error: API_NATIVE_ERROR ****", "Error while calling the \"Eve::getEventAttributes\" native api", apiMsg)
			exit(1)
		
		# Create a new Event
		event = Event(wrapEvent.getEventName(eventNumber), eventAttributes, level)

		# Store the Event created
		events.append(event)
		
	# Return all the Events of the Job ID		
	return events;		

def queryEvents(jobids, includes, excludes, userTags, issuer, fromT, toT, level, suppressError=False):
	"""
	queryEvents: perform a query on the Events logged on the LB Servers.
	Returns a dictionary with the JOB ID as a key and the value associted is the array of Events of the specific JOB ID
	"""
	
	
	
	
	
	# Initialise the Job Id / Events dictionary
	queriedEvents = {}
	
	# Initialise the LB Host / Port dictionary
	lbDict={}

	# Fill the LB Host / Port dictionary starting from the retrieved Job Id's
	for jobid in jobids:

		# Check if the LB Host key is already present inside the dictionary
		if lbDict.has_key(jobid.lbHost):

			# Check if the LB Port key is already present inside the dictionary
			if lbDict[jobid.lbHost].has_key(jobid.lbPort):
				# Append the Job ID (Lb Host & port already exist)
				lbDict[jobid.lbHost][jobid.lbPort].append(jobid.jobid)
			else:

				# Append the Job ID (Lb Host already exists, port is new)
				lbDict[jobid.lbHost][jobid.lbPort]=[jobid.jobid]
		else:
			#Lb Host&port NEW
			lbDict[jobid.lbHost]={}
			lbDict[jobid.lbHost][jobid.lbPort]=[jobid.jobid]
	
	# Scan all LB Hosts e Ports to retrieve Events
	
	
	for lbHost in lbDict.keys():
	
		# Scann all LB Ports
		for lbPort in lbDict[lbHost].keys():
		
			# Debug Message
			dbgMsg ("Eve::queryEvents", lbDict[lbHost][lbPort], lbHost, lbPort, \
				userTags.keys(), userTags.values(), excludes, includes, issuer, fromT, toT)

			# Perform LB Query with the current LB Host and Port
			
			wrapEvent = Eve(lbDict[lbHost][lbPort], \
			                lbHost, \
					lbPort, \
					userTags.keys(), \
					userTags.values(), \
					excludes, \
					includes, \
					issuer, \
					fromT, \
					toT)
					
			# Check for errors
			err, apiMsg = wrapEvent.get_error ()
			
			if err and not suppressError:
				errMsg ("Warning", "API_NATIVE_ERROR", "Eve::queryEvents", apiMsg)
				sys_exit= -1
			else:		
				# Retrieve the number of logged events 
				eventsNumber = wrapEvent.getEventsNumber()

				# Retrieve the list of attributes for each logged event
				for eventNumber in range(eventsNumber):
	
					#Retrieve the list of attributes for the current event
					eventAttributes = wrapEvent.getEventAttributes(eventNumber)
		
					# Check for Errors
					err , apiMsg = wrapEvent.get_error ()
					if err:
						# Print the error and terminate
						wmsui_utils.errMsg("**** Error: API_NATIVE_ERROR ****", "Error while calling the \"Eve::getEventAttributes\" native api", apiMsg)
						exit(1)
		
					# Create a new Event
					event = Event(wrapEvent.getEventName(eventNumber), eventAttributes, level)
					
					# Retrieve the Job ID of the queried event
					jobid = event.getJobId()

					# Insert the events array inside the dictionary
					if not queriedEvents.has_key(jobid):

						# Create a new entry inside the dictionary
						queriedEvents[jobid] = []
					
					# Store the Event created
					queriedEvents[jobid].append(event)

	# Return all the Queried Events
	return queriedEvents

def queryCodeEvents(jid, eventAttribute, minLog, level):
	"""
	Retrieve all required logged event reasons, cutting possible null (nil) reasons
	minLog is the lowest number of events taken into consideration
	i.e. for DONE events at least 2 events are needed (one is already displayed)
	"""

	reasons=[]
	jobid = JobId(jid)
	
	queriedEvents = queryEvents([jobid], [events_codes.index(eventAttribute)], [], {}, "",0,0, level, True)

	if queriedEvents:

		# Parse reasons
		if len(queriedEvents[jid])>=minLog:
			for ev in range (len(queriedEvents[jid])):
				reasonStr=queriedEvents[jid][ev].getAttribute(events_names.index("Reason")).strip()
				if reasonStr!="(nil)":
					reasons.append(reasonStr)
	return reasons

def printEvents(jobid, eventArray, json, pprint):
	
	"""
	printEvents: print all the Events of a specific Job ID"
	"""

	message=""
	thisMessage = ""
	quote = "\""
	carriage = ","
	ccarriage = ""
	tab = ""
	
	#print "******* jobid=" + jobid.jobid + "\n"
	
	if json is True:
		if pprint is True:
			quote = ""
			carriage = "\n"
			tab = "\t"
			ccarriage = "\n"
		
	if json is False:
		message += "\n"
		message += "LOGGING INFORMATION:\n\n"
		message += "Printing info for the Job : " + jobid.jobid
		message += "\n \n"
	else:
		message += ccarriage + tab + quote + "JobID" + quote + ": " + quote + jobid.jobid + quote + carriage + tab + quote + "Events" + quote + ": {" + ccarriage
		
		
	elements = [];
		
	for singleEvent in eventArray:
		thisMessage = tab + tab + singleEvent.printEvent(json, pprint) + ccarriage
		
		elements.append( thisMessage );
		
	if json is True:
		if pprint is True:	
			message += "".join( elements )
		else:
			message += ", ".join( elements )
	else:
		message += "".join(elements)
		
	if json is True:
		if pprint is True:
			message += tab + "}\n"
		else:
			message += "}";
		
	return message 

	for event in queriedEvents[jobid]:
		message += event.printEvent()

	return message

def printQueriedEvents(jobids, queriedEvents, json, pprint):
	"""
	printQueriedEvents: print all the Events of the list of Job ID's using a dictionary built from queryEvents function"
	"""

	message = ""
	errorMessage="Unable to retrieve any matching information for jobid: "
	em = "Warning"
	
	# Scan all Job ID retrieved
	for jobid in jobids:
	
		try:
			# Build the output message
			message += printEvents(jobid, queriedEvents[jobid.jobid], json, pprint)
		except KeyError, err :
			errMsg ( em , "API_NATIVE_ERROR" , "Eve::queryEvents",errorMessage+jobid.jobid)

	return message

def getStatus(jobid, level):
	"""getStatus: Return the status of a single Job ID"""

	# Create the LB Wrapper for the Job ID
	wrapStatus = Status(jobid.jobid, level)
	
	# Check for Errors
	err , apiMsg = wrapStatus.get_error ()
	if err:
		# Print the error and terminate
		raise Exception(apiMsg)
		
	# Retrieve the number of status 
	statesNumber = wrapStatus.getStatusNumber()
	
	# Retrieve the list of attributes for each logged event
	for statusNumber in range(statesNumber):
	
		#Retrieve the list of attributes for the current event
		statusAttribute = wrapStatus.getStatusAttributes(statusNumber)
		
		# Check for Errors
		err , apiMsg = wrapStatus.get_error ()
		if err:
			# Print the error and terminate
			wmsui_utils.errMsg("**** Error: API_NATIVE_ERROR ****", "Error while calling the \"Eve::getEventAttributes\" native api", apiMsg)
			exit(1)
		
		# Create a new Event
		status = JobStatus(wrapStatus.getStatusName(statusNumber), statusAttribute, level)
		
	# Return all the Events of the Job ID		
	return status;		

def queryStates(jobids, voName, includes, excludes, userTags, issuer, fromT, toT, level):
	"""
	queryStates: perform a query on the States logged on the LB Servers.
	Returns a dictionary with the JOB ID as a key and the value associted is Status of the specific JOB ID
	"""
	
	# Initialise the Job Id / States dictionary
	queriedStates = {}
	
	# Initialise the LB Host / Port dictionary
	lbDict={}
	
	# Check if the LB dictionary has to be created starting from a list of Job ID's
	if jobids:
	
		# Fill the LB Host / Port dictionary starting from the retrieved Job Id's
		for jobid in jobids:

			# Check if the LB Host key is already present inside the dictionary
			if lbDict.has_key(jobid.lbHost):

				# Check if the LB Port key is already present inside the dictionary
				if lbDict[jobid.lbHost].has_key(jobid.lbPort):
					# Append the Job ID (Lb Host & port already exist)
					lbDict[jobid.lbHost][jobid.lbPort].append(jobid.jobid)
				else:

					# Append the Job ID (Lb Host already exists, port is new)
					lbDict[jobid.lbHost][jobid.lbPort]=[jobid.jobid]
			else:
				#Lb Host&port NEW
				lbDict[jobid.lbHost]={}
				lbDict[jobid.lbHost][jobid.lbPort]=[jobid.jobid]
	else:
		# Retrieve from the configuration the LB Addressis
		lbAddresses = wmsui_utils.getLBs()

		if not lbAddresses:
			# TODO: Pass these two parameters from outside
			wmsui_utils.queryLBs(voName, lbAddresses)

		for addr in lbAddresses:
			lbHost , lbPort = addr
			
			# Check if the LB Host key is already present inside the dictionary
			if lbDict.has_key(lbHost):

				# Check if the LB Port key is already present inside the dictionary
				if not lbDict[lbHost].has_key(lbPort):
					# Append the Job ID (Lb Host & port already exist)
					lbDict[lbHost][lbPort] = []
			else:
				#Lb Host&port NEW
				lbDict[lbHost]={}
				lbDict[lbHost][lbPort]=[]
	
	# Scan all LB Hosts e Ports to retrieve Events
	for lbHost in lbDict.keys():
	
		# Scann all LB Ports
		for lbPort in lbDict[lbHost].keys():
		
			wait_msg="\nRetrieving Information from LB server "+ \
				 lbHost+":"+ repr(lbPort) + \
				 "\nPlease wait: this operation could take some seconds.\n"
				 
			print wait_msg

			# Debug Message
			dbgMsg ("Status", lbDict[lbHost][lbPort], lbHost, lbPort, \
				userTags.keys(), userTags.values(), excludes, includes, issuer, fromT, toT, level)

			# Perform LB Query with the current LB Host and Port
			wrapStatus = Status(lbDict[lbHost][lbPort], \
			                lbHost, \
					lbPort, \
					userTags.keys(), \
					userTags.values(), \
					excludes, \
					includes, \
					issuer, \
					fromT, \
					toT,
					level)
					
			# Check for errors
			err, apiMsg = wrapStatus.get_error ()
			if err:
				errMsg ("Warning", "API_NATIVE_ERROR", "Status::queryStatus", apiMsg)
				sys_exit= -1
			else:		

				# Retrieve the number of logged events 
				statusNumber = wrapStatus.getStatusNumber()

				# Retrieve the list of attributes for each logged event
				for stateNumber in range(statusNumber):
	
					#Retrieve the list of attributes for the current event
					statusAttributes = wrapStatus.getStatusAttributes(stateNumber)

					# Check for Errors
					err , apiMsg = wrapStatus.get_error ()
					if err:
						# Print the error and terminate
						wmsui_utils.errMsg("**** Error: API_NATIVE_ERROR ****", "Error while calling the \"Eve::getEventAttributes\" native api", apiMsg)
						exit(1)
		
					# Create a new Event
					status = JobStatus(wrapStatus.getStatusName(stateNumber), statusAttributes, level)

					# Retrieve the Job ID of the queried event
					jobid = status.getJobId()

					# Insert the events array inside the dictionary
					if queriedStates.has_key(jobid):

						# Print the error and terminate
						raise Exception("Status for the same jobid " + jobid + " has been retrieved already retrieved")
					
					# Store the Event created
					queriedStates[jobid] = status

	# Return all the Queried States
	return queriedStates
	
"""

"""
class JobState:
  STATE_ID = "StateId"
  CURRENT_STEP="CurrentStep"
  USER_DATA="UserData"
  JOB_STEPS="JobSteps"
  GLITE_JOBID = "edg_jobid"
  state = ""
  """
  Initialise the JobState
  """
  def  __init__(self , state = ""):
    	self.state = AdWrapper( 1 )
  """
  Check the syntax of the State
  """
  def check (self):
	errors = ""
	s_id     = self.state.getStringValue (self.STATE_ID)
	# STATE_ID check
	if len(s_id) >1:
		errors+= self.STATE_ID +": List not allowed\n"
	elif len( s_id) ==1:
		jobid = JobId  ( s_id[0] )
		if not jobid.set:
			errors+= self.STATE_ID +": Wrong Id value\n"
	#  CURRENT_STEP check
	s_tep   = self.state.getStringValue (self.CURRENT_STEP)
	#  USER_DATA check
	u_data = self.state.getStringValue (self.USER_DATA)
	#  JOB_STEPS check
	j_steps = self.state.getStringValue (self.JOB_STEPS)
	if errors:
		wmsui_utils.errMsg("Error","UI_JDL_ERROR", errors)
		exit(1)
  """
  Load a chkpt file
  """
  def fromFile( self ,stateFile ):
	if not os.path.isfile(   stateFile ):
			wmsui_utils.errMsg("Error","UI_FILE_NOT_FOUND",submissionFile)
			exit(1)
	elif ( self.state.fromFile(stateFile) ):
			exit(1)
  """
  Generate a JobState from a string
  """
  def fromString(self, stateStr):
	#self.state.get_error() ;
	self.state.fromString( stateStr.strip() )
	err , apiMsg = self.state.get_error() ;
	if err:
		wmsui_utils.errMsg("Error","UI_JDL_ERROR", "Unable to convert into a jobstate instance.\n"+ apiMsg )
		exit(1)
  """
  toString: convert the State instace into string representation
  """
  def toString( self, multi = 0):
	str = self.state.toLines()
	if not multi:
		str = str.replace ("\n" , " " )
	return str
  """
  Insert the dg_jobid (remove the previous if needed)
  """
  def setId( self, jobid):
	if self.state.hasKey(self.STATE_ID ):
		self.state.removeAttribute( self.STATE_ID )
	self.state.setAttributeStr ( self.STATE_ID , jobid )
  """
  Initialise default JobState from a ready-to-submit jobad
  """
  def initialise ( self, jobad ):
	if  jobad.hasKey ( self.GLITE_JOBID ):
			self.state.setAttributeStr (self.STATE_ID, jobad.getStringValue(self.GLITE_JOBID)[0])
	if  jobad.hasKey ( self.CURRENT_STEP ):
			self.state.setAttributeInt (self.CURRENT_STEP, jobad.getIntValue(self.CURRENT_STEP)[0])
	else:
		self.state.setAttributeInt(self.CURRENT_STEP, 1)
	self.state.setAttributeExpr ( self.USER_DATA , "[]" )
	#### JOB Steps special attribute managing
	if  jobad.hasKey (self.JOB_STEPS ):
			val = jobad.getStringValue (self.JOB_STEPS )
			if val:
				for va in val:
					self.state.addAttributeStr ( self.JOB_STEPS ,  va )
			else: #it's not a string. could be an int
				val =jobad.getIntValue (self.JOB_STEPS )
				if val:
					self.state.setAttributeInt ( self.JOB_STEPS ,  val[0] )
				else: #It's neither a string or an integer but it's present in the jdl: error
					wmsui_utils.errMsg("Error","UI_JDL_ERROR", "Wrong type caught for " + self.JOB_STEPS  )

"""
####################################
                         JobId Class
####################################
"""
class JobId:
  lbHost = ""
  lbPort = 0
  unique = ""
  jobid = ""
  set = 0
  def  __init__(self, jobid=""):
     if jobid:
        self.fromString (jobid)
  #def  __init__(self):
  #   self.set= 0
  """
  create a jobId instance from string
  """
  def  fromString (self, jobid):
    LB_DEFAULT_PROTOCOL = "https://"
    avail_prots=["http","https"]
    output=[]
    if jobid:
     #extract information from jobid, using ':#' as token
     try: #cut http:# from the LB
        prot , body = jobid.split('://',1)
        if not prot in avail_prots:
                return 1
        prot= prot+"://"
        #separate address and  unique string
        lbAddr , unique = body.split('/',1)
        self.lbHost , self.lbPort = lbAddr.split (":" , 1 )
        self.lbPort = int (self.lbPort)
        if  not unique.strip():
               return 1
        self.unique=unique.strip()
        self.set = 1
        self.jobid = jobid

        return 0
     except:
        # display the error message
        return 3
     return 1
"""
  prompt the jobis to the user and let him select among them
  """
def selectJobId (jobid_list , strjobs, multi = 1):
     if len(jobid_list) ==1:
        return jobid_list
     indJ = 0
     indU = 0
     print "------------------------------------------------------------------"
     for jobus in strjobs:
        indJ = indJ +1
        if ( strjobs[indJ-1].find ('#*') == 0):
           comment = strjobs[indJ-1]
           size = len(comment)
           comment = comment[2:size]
           print "      ", comment
        else:
           indU = indU +1
           print repr(indU).ljust(2) + ':', strjobs[indJ-1]
     if multi:
        print 'a'.ljust(2) + ':','all'
     print 'q'.ljust(2) + ':','quit'
     keep=1
     print "------------------------------------------------------------------"
     if multi:
        question= '\nChoose one or more jobId(s) in the list - [1-'+str(indU)+']all:'
     else:
        question= '\nChoose one jobId in the list - [1-'+str(indU)+']:'
     jobChoose = []
     while keep:
       ans = ""
       try:
          ans=raw_input(question)
       except KeyboardInterrupt:
          print "\nKeyboard interrupt raised by user, now exiting...\nbye"
          exit(1)
       ansSplit=ans.split(',')
       anslen = len(ansSplit)
       if (  ans.find(",")* ans.find("-") !=1 ) and not multi:
          keep = 1
       elif (ans==""):
         jobChoose = jobid_list
         keep =0
       elif (ans=="q"):
        print 'bye'
        wmsui_utils.exit(0)
       elif (ans == "a"):
         if multi:
           keep = 0
           jobChoose = jobid_list
       else:
         if (anslen == 1):
           trat = ans.split('-')
           tratlen = len(trat)
           if (tratlen == 1):
             try:
              ind = int(ans)
              if ind in range(1,indJ+1):
               jobChoose.append(jobid_list[ind-1])
               keep=0
              else:
               jobChoose = []
               keep=1
             except:
              jobChoose = []
              keep = 1
           elif (tratlen == 2):
            try:
             s=int(trat[0])
             e=int(trat[1])
             ran = range(s,e+1)
             for k in ran:
              if k in range(1,indJ+1):
                 jobChoose.append(jobid_list[k-1])
                 keep = 0
              else:
                 jobChoose = []
                 keep = 1
            except:
               jobChoose = []
               keep = 1
           else:
             jobChoose = []
             keep = 1
         else:
           for h in ansSplit:
             trat = h.split('-')
             tratlen = len(trat)
             if (tratlen == 1):
              try:
               ind =int(h)
               if ind in range(1,indJ+1):
                jobChoose.append(jobid_list[ind-1])
                keep = 0
               else:
                jobChoose = []
                keep = 1
              except:
               jobChoose = []
               keep = 1
             elif (tratlen == 2):
              try:
               s=int(trat[0])
               e=int(trat[1])
               ran = range(s,e+1)
               for k in ran:
                if k in range(1,indJ+1):
                 jobChoose.append(jobid_list[k-1])
                 keep = 0
                else:
                 jobChoose = []
                 keep = 1
              except:
               jobChoose = []
               keep = 1
             else:
              jobChoose = []
              keep = 1
     if jobChoose:
       print "\n"
       return jobChoose
     else:
        #Unable to find any jobid in the input file
        errMsg(info.logFile,'Error','UI_BAD_INPUT_FILE')
        wmsui_utils.exit(1)
"""
Read the specified input file and return a list containing the jobid (s)
"""
def getJobIdfromFile (json, inFile ):
     comment = ""
     sys_exit = 0
     if not os.path.isfile(inFile):
        errMsg('Error','UI_FILE_NOT_FOUND',inFile)
        exit(1)
     else:
       try:
           f=open(inFile)
       except:
           errMsg('Error','UI_OPEN_ERR',inFile)
           exit(1)
       right_jobs =[]
       str_jobs =[]
       parsed = 0
       for job in f.readlines():
           parsed +=1
           job = job.strip ()
           if ( job.find ('#*') == 0):
                 comment = job
                 if comment != "":
                    str_jobs.append(comment)
           elif ( job.find ('#') != 0) and ( job.find('//') != 0) and (job!=""):
              jobid = JobId()
              if jobid.fromString(job):
	         if json == False:
                    wmsui_utils.errMsg( 'Warning','UI_WRONG_JOBID_FORMAT',job)
		 else:
			print ", \"" + job + "\": \"JOBID HAS WRONG FORMAT\"" 
                 sys_exit = -1
              else:
                 if job in str_jobs:
		    if json == False:
                       wmsui_utils.errMsg( 'Warning','UI_JOBID_REPEAT',job)
                    sys_exit = -1
                 else:
                    right_jobs.append( jobid )
                    str_jobs.append (job)
       f.close
     if not parsed:
          wmsui_utils.errMsg('Error','UI_NO_JOBID')
          exit(1)
     return [sys_exit , right_jobs, str_jobs ]
"""
   Retrieve JobId from a list of strings:
"""
def getJobIdfromList (json, jobids):
     jobs= []

##      for job in jobids:
## 	     job= job.strip () 
## 	     jobid = JobId ( job )
## 	     jobs.append(jobid)

##      return jobs
     
     for job in jobids:
	     job= job.strip () 
	     jobid = JobId ( job )
	     if not jobid.set:
		     if json == False:
			     errMsg('Warning','UI_WRONG_JOBID_FORMAT',job)
		     else:
			     print ", \"" + job + "\": \"JOBID HAS WRONG FORMAT\"", 
			     
		     pass
	     else:
		     if job in jobs:
			     if json == False:
				     errMsg('Warning','UI_JOBID_REPEAT',job)
		     else:
			     jobs.append(jobid)
	    
     return jobs

"""
###############################
  Shadow Class: used to implement listener activities
###############################
"""
class Shadow:
  pipeRoot = ""
  jobid = JobId()
  threadNoGui=""
  #listener = ""
  host = ""
  port = 0
  pid = 0
  refresh  = 1
  accum = ""
  pipe = ""
  pipeOut = ""
  pipeErr = ""
  pipeIn = ""
  barbatruc=""
  writing = 0
  # gui = 1  #Deprecated
  """
  # INitialise parameters
  """
  def set(self , jid):
    self.jobid = jid
    #self.listener = ls
  """
  Write the string into the inputStream
  """
  def write (self, txt):
    self.writing = 1
    if not self.barbatruc:
         self.barbatruc=open ( self.pipeRoot +".in" , "w")
    self.pipeIn = open ( self.pipeRoot +".in" , "w")
    if txt[-1]!="\n":
       txt+="\n"
    self.pipeIn.write (txt)
    self.pipeIn.close()
    self.writing  = 0

  """
  Read and empty the Error buffer
  """
  def emptyErr(self ):
     return  self.pipeErr.read( 1 )
  """
  Read and empty the Output buffer
  """
  def emptyOut(self ):
     if not self.pipeOut:
         self.pipeOut =  open ( self.pipeRoot +".out" )
     return  self.pipeOut.read( 1 )
  """
  get the name of the error pipe
  """
  def getPipeErr(self):
     return self.pipeRoot +".err"
  """
  get the name of the input pipe
  """
  def getPipeIn(self):
     return self.pipeRoot +".in"
  """
  get the name of the  output pipe
  """
  def getPipeOut(self):
      return self.pipeRoot +".out"
  """
  get the name of the local host
  """
  def getHost(self):
     return  self.host
  """
  get the value of the shadow pid
  """
  def getPid(self):
     return self.pid
  """
  get the value of the port which is listening to
  """
  def getPort(self):
      return self.port
  """
  Kill the shadow listener process
  """
  def kill (self):
	if self.pid != 0 :
		command = "kill -9 " + repr( self.pid )
		os.system(command);
  """
  Destroy the instance
  """
  def detach(self):
     if self.barbatruc:
          self.barbatruc.close()
     self.kill()
     for fi in [  self.getPipeErr()  ,  self.getPipeOut()  ,  self.getPipeIn()   ]:
        try:
           os.remove( fi  )
        except:
           pass

  def __call__ (self, command):
	try:
		os.system( command  )
	except KeyboardInterrupt :
		print "bye bye!!!"
		exit(0)

  """
  Check the destination path
  """
  def  checkRootPath(self):
	# Pipe Stream required
	rootDir  = os.sep + "tmp"
	rd = wmsui_utils.solvePath(wmsui_utils.info.confAd.getStringValue ("ListenerStorage")[0] )
	if rd:
		rootDir = rd
	if  not os.path.isdir( rootDir ):
		errMsg('Error','UI_DIR_NOT_FOUND', rootDir)
		exit(1)
	self.pipeRoot =  rootDir + os.sep + "listener-" + self.jobid.unique
	foundPrevious =0
	for fi in [ self.pipeRoot ,  self.getPipeErr() ,   self.getPipeOut() ,  self.getPipeIn() ]:
		if  os.path.exists( fi ):
			errMsg( 'Warning' , 'UI_FILE_EXISTS', fi )
			foundPrevious=1
	if foundPrevious:
		question = "A previous interactive session might still be active.\nDo you whish to proceed anyway?"
		answ=wmsui_utils.questionYN(question)
		if not answ: #No answered
			print "bye"
			exit (0)


  """
  Launch the glite--wms-grid-console-shadow and retrieve host &port information
  """
  def console( self, port=0):
	shPath = wmsui_utils.info.prefix + os.sep +"bin" + os.sep
	command =shPath + "glite-wms-grid-console-shadow"
	if  not os.path.isfile( command):
		errMsg('Error','UI_FILE_NOT_FOUND', command)
		exit(1)
	else:
	   command = os.path.abspath( command )
	# Kill the glite-wms-grid-console-shadow, if still running:
	self.kill ()
	arguments = ""

	#Check for opened ports interval due to a possible firewall
	fromPort = 0
	toPort= 0
	env=0
	try:
		env = os.environ["GLOBUS_TCP_PORT_RANGE"]
		#environment variable found
		for sep in [ " " , ":" , "-" , "," ]:
			ind = env.find(sep)
			if ind!=-1:
				try:
					fromPort , toPort = env.split (sep)
					fromPort = int(fromPort)
					toPort = int(toPort) +1
					break
				except:
					pass
		if not fromPort:
			errMsg('Warning',"UI_ENVIRONM_ERROR","GLOBUS_TCP_PORT_RANGE" ,"Unable to parse the value" )
	except:
		pass
	#This variable stores the number of port retrieval.
	#If GLOBUS_TCP_PORT_RANGE is used then UI automatically retry on all available ports before surrending
	retryPort = 1
	#This is the grid_console_shadow error code for an used port
	ALREADY_USED_PORT = 98
	if port:
		# If the port is forced then check the value immediately
		if fromPort and toPort:
			# Port range found at the end of the cycle
			if port<fromPort or port>toPort:
				errMsg('Error','JOB_ATTACH_PORT_RANGE', self.jobid.jobid,  port , env)
				exit(1)
		arguments += " -port " + repr (port)
	elif fromPort and toPort:
		#Port not specified but range found: select randomly a port between the allowed range
		RandomPort = fromPort + int(( toPort - fromPort)* random.random() )
		port = RandomPort
		arguments += " -port " + repr (port)
		retryPort = toPort - fromPort
	# Launch the glite-wms-grid-console-shadow with --logfile (background process)
	# log std in/out/error into named pipe

	for i in range (retryPort):
		arguments += " -log-to-file " + self.pipeRoot   +" &";
		os.system(command + " " + arguments )
		time.sleep(10)
		# get the pid and the port:
		errConsoleInfo = self.getConsoleInfo()
		#Check For Error and if necessary retry
		if errConsoleInfo == ALREADY_USED_PORT:
			if fromPort and toPort and wmsui_utils.info.debug:
				errMsg('Warning', "UI_CAN_NOT_EXECUTE" , shPath +"glite-wms-grid-console-shadow", "Port " + repr(port) +" is already in use")
			if fromPort:
				port =  fromPort +   (port +1) % ( toPort - fromPort )
			arguments = " -port " + repr (port)
		elif errConsoleInfo:
			errMsg('Error', "UI_CAN_NOT_EXECUTE" , shPath +"glite-wms-grid-console-shadow","Command failed")
			exit(1)
		else:
			#No error found. Console successfully Started
			if fromPort and toPort and wmsui_utils.info.debug:
				wmsui_utils.print_message(False, wmsui_utils.info.logFile,"Firewall range port found. listening port forced to: " +repr (port )  )
			break

	if errConsoleInfo:
			errMsg('Error', "UI_CAN_NOT_EXECUTE" , shPath +"glite-wms-grid-console-shadow","Unable to find any available port")
			exit(1)

  def join(self):
	while 1:
		try:
			time.sleep(2)
		except KeyboardInterrupt :
			print "bye"
			self.detach ()
			exit(0)
	exit(0)

  """
  Once the consloe listener has started, port and pid information
  have to be retrieved from the info named pipe
  """
  def getConsoleInfo(self):
	adStr = "";
	timeout= 0
	MAXTIMEOUT=3
	while 1:
	   try:
	      self.pipe=open (self.pipeRoot)
	      break
	   except:
	      timeout=timeout+1
	      time.sleep(1)
	      if timeout >MAXTIMEOUT :
		errMsg('Error', "UI_CAN_NOT_EXECUTE" , "glite-wms-grid-console-shadow","Timeout limit exceeded while reading info named pipe")
		exit(1)
	while 1:
	    adStr += self.pipe.read( 1 )
	    if  adStr.find("]") !=-1:
	        break ;

	os.remove(   self.pipeRoot )
	ad = AdWrapper ( 1 )
	adStr = adStr.strip()
	if ad.fromString(adStr.replace ("\n" , " " ) ):
	   errMsg('Error', "UI_CAN_NOT_EXECUTE" , "glite-wms-grid-console-shadow","Wrong info pipe stream found")
	   exit(1)
	if ad.hasKey("SHADOW_ERROR"):
		return ad.getIntValue("SHADOW_ERROR")[0];
	port = ad.getIntValue("PORT");
	# Set the Shadow private members
	## PORT
	if len(port) ==1:
	   self.port = port[0]
	else:
	   errMsg('Error', "UI_CAN_NOT_EXECUTE" , "glite-wms-grid-console-shadow", "Wrong retrieved port value")
	   exit(1)
	## PID
	pid  = ad.getIntValue("PID");
	if len(pid) ==1:
	   self.pid = pid[0]
	else:
	   errMsg('Error', "UI_CAN_NOT_EXECUTE" ,  "glite-wms-grid-console-shadow","Wrong retrieved pid value")
	   exit(1)
	## HOST
	self.host = socket.getfqdn()
	return 0

"""
Event
"""
class Event:

	def __init__(self, eventName, eventAttributes, level):
		"""Event Constructor: Create an Event object with all its attributes."""

		# Set the Event Name
		self.__eventName = eventName
		
		# Set the Event Attributes array
		self.__eventAttributes = eventAttributes
		
		# Set the Logging level
		self.__level  = level
				
	def getAttribute(self, eventAttribute):
		"""Event getAttribute: retrieve a specific attribute."""
	
		# Check if the event attribute is valid
		if eventAttribute < 0 or eventAttribute >= EVENT_ATTR_MAX:
			# Print the error and terminate
			wmsui_utils.errMsg("Error:", "Event number " + str(eventAttribute) + " out of range 0:" + str(EVENT_ATTR_MAX-1))
			exit(1)
	
		# Return the requested attribute
		return self.__eventAttributes[eventAttribute]
		
	def printEvent(self, json, pprint):
		"""Event printEvent: print all the Events retrieved."""

		# Initialise the output
		message = ""

		quote = "\""
		Return = ""
		
		if json is True:
			if pprint is True:
				quote = ""
				Return = "\n";
		
		if self.__level:
			if json is False:
				message += "\t---\n"
			if json is False:
				message += "Event: "
			else:
				if pprint is True:
					message += "  {" + "\n\t\t    "
				else:
					message += " { " 
		else:
			if json is False:
				message +=" - "

		# Set the Event Name
		if json is False:
			message += self.__eventName + "\n"
		else:
			message += quote + "EventName" + quote + ": " + quote + self.__eventName + quote 
		if self.__level:
		
			# Initialie Standard and Ad messages
			stStr = ""  
			adStr = ""
				
			# Cycling event attributes inside a single event (only when level>0):
			for attributeIndex in range(EVENT_ATTR_MAX):
				
				# Reset tje Timestamp
				timeStamp = 0
				
				# Retrieve the Attribute name
				attributeName = events_names[attributeIndex]
				
				# Retrieve the Attribute value
				attributeValue = self.getAttribute(attributeIndex).strip()
				
				
				# Check for valid attribute name and value
				if attributeName and attributeValue and attributeValue!="(nil)":
				
					# Skip in case of Job Id or Event attributes
					if not attributeName in ["Jobid", "Event"]:
						
						# Manage properly the attribute				
						if attributeName in [ "Timestamp"]:
							# Set the timestamp
							timeStamp = int(attributeValue)

							# Convert to time the attribute vale
							attributeValue = time.asctime(time.localtime(  int (attributeValue ) ) )
							
							TS = ""
							if json is False:
								TS = attributeValue + DSTime
							else:
								TS = str(timeStamp)
								
							#print "TS=" + str(timeStamp);
								
							# Append the Timestamp to the Standard messages string
							if json is False:
								stStr += "- "+ attributeName.ljust(27) + "=    " + TS + "\n"
							else:
								if pprint is True:
									stStr += "\n\t\t    " + quote + attributeName + quote + ": " + quote + TS + quote 
								else:
									stStr += ", " + quote + attributeName + quote + ": " + quote + TS + quote 
							
							#print "ALVI-DEBUG: attributeName=[" + attributeName +"]\n"
							
							#print "ALVI-DEBUG: >>" + stStr +"<<"	
							
						elif attributeName in [ "Arrived"]:
							if self.__level > 1:
								# Convert to time the attribute vale
								
								timestamp = attributeValue;
								
								attributeValue = time.asctime(time.localtime(  int (attributeValue ) ) )
							
								# Append the Timestamp to the Standard messages string
								TS = ""
								if json is True:
									TS = timestamp
								else:
									TS = attributeValue + DSTime
								
								if json is False:
									stStr += "- "+ attributeName.ljust(27) + "=    " + TS +"\n"
								else:
									if pprint is True:
										stStr += "\n\t\t    " + quote + attributeName + quote + ": " + quote + TS + quote
									else:
										stStr += ", " + quote + attributeName + quote + ": " + quote + TS + quote
						elif attributeName in ["Job" , "Jdl", "Classad"]:
							if self.__level > 2:
								if attributeValue[0]=="[":
									jobad = AdWrapper (1)
									if jobad.fromString(attributeValue):
										err , apiMsg = jobad.get_error()
										errMsg('Warning','UI_JDL_WRONG_SYNTAX' , apiMsg ) ;
									attributeValue = jobad.toLines()
									attributeValue = colsep + attributeValue.replace( "\n" , colsep )
								if json is False:
									adStr += "- " +attributeName.ljust(15) + "=   " + attributeValue +"\n"
								else:
									if pprint is True:
										adStr += "\n\t\t    " + quote + attributeName + quote + ": " + quote + attributeValue + quote
									else:
										adStr += ", " + quote + attributeName + quote + ": " + quote + attributeValue + quote
						elif attributeName in ["Level", "Seqcode"]:
							if self.__level > 2:
								if json is False:
									stStr += "- "+ attributeName.ljust(27) + "=    " + attributeValue +"\n"
								else:
									if pprint is True:
										stStr += "\n\t\t    " + quote + attributeName + quote + ": " + quote + attributeValue + quote
									else:
										stStr += ", " + quote + attributeName + quote + ": " + quote + attributeValue + quote
						elif attributeName in ["Priority"]:
							if self.__level >2:
								try:
									if int(attributeValue):
										attributeValue="synchronous"
									else:
										attributeValue="asynchronous"
								except:
									pass
								if json is False:
									stStr += "- "+ attributeName.ljust(27) + "=    " + attributeValue +"\n"
								else:
									stStr += ", " + quote + attributeName + quote + ": " + quote + attributeValue + quote
						elif attributeName in ["Source", "Destination", "Result", "Dest id"]:
							if json is False:
								stStr += "- "+ attributeName.ljust(27) + "=    " + attributeValue +"\n"
							else:
								if pprint is True:
									stStr += "\n\t\t    "+ quote + attributeName + quote + ": " + quote + attributeValue + quote
								else:
									stStr += ", "+ quote + attributeName + quote + ": " + quote + attributeValue + quote
						elif self.__level > 1:
							if json is False:
								stStr += "- "+ attributeName.ljust(27) + "=    " + attributeValue +"\n"
							else:
								if pprint is True:
									stStr += "\n\t\t    "+ quote + attributeName + quote + ": " + quote + attributeValue + quote
								else:
									stStr += ", "+ quote + attributeName + quote + ": " + quote + attributeValue + quote
				#print "ALVI-DEBUG >>" + stStr + "<<\n";
				#print "ALVI-DEBUG >>" + adStr + "<<\n";

			# join the two pools
			if json is True:
				if pprint is True:
					message += stStr + adStr + "\n\t\t  }"
				else:
					message += stStr + adStr + "}"
			else:
				message += stStr + adStr
		else:
			if json is True:
				message = "{" + message + "}"
			else:
				pass
					
		return message
		
	def getJobId(self):
		"""Event getJobId: Return the JOB ID of the current object."""
		# Return the Job ID
		return self.__eventAttributes[events_names.index("Jobid")]

"""
JobStatus
"""
class JobStatus:

	def __init__(self, statusName, statusAttributes, level):
		"""JobStatus Constructor: Create a Job Status object with all its attributes."""

		# Set the Status Name
		self.__statusName = statusName
		
		# Set the Status Attributes array
		self.__statusAttributes = statusAttributes
		
		# Set the Logging level
		self.__level  = level

	def getAttribute(self, statusAttribute):
		"""JobStatus getAttribute: retrieve a specific attribute."""
	
		# Check if the status attribute is valid
		if statusAttribute < 0 or statusAttribute >= STATE_ATTR_MAX:
			# Print the error and terminate
			wmsui_utils.errMsg("Error:", "Status number " + str(statusAttribute) + " out of range 0:" + str(STATE_ATTR_MAX-1))
			exit(1)
	
		# Return the requested attribute
		return self.__statusAttributes[statusAttribute]
		
	def printStatus(self, json, noNodes=0):
		"""JobStatus printStatus: print the Status."""
		# Map has to be initialised for each PrintStatus
		self.jobidMap={}
		#self.level = level
		self.hierarchy = 0
		self.jobid =""
		self.nodeName=""

		intervals = int ( len(self.__statusAttributes) / STATE_ATTR_MAX  )

		result =""
		for off in range ( intervals ):
			offset = off*STATE_ATTR_MAX
			
			result +=self.__printSt__(json, self.__statusAttributes[ offset : offset + STATE_ATTR_MAX ] )

			if noNodes:
				break
		return result

	def __printSt__( self, json, info ):
	
		hierarchy = int(info[states_names.index("Hierarchy") ])
		indent = "    "*hierarchy
		stStr = ""
		adStr= ""
		if json == False:
			sepStart = "======================= glite-wms-job-status Success =====================\n"
			sep = "==========================================================================\n" + indent
		else:
			sepStart = ""
			sep=""
		jobid =info[ states_names.index("Jobid")]
		if not indent:
			# Indentation is NOT present: main Job
			self.jobid =jobid
			
			if json == False:
				Title= "\n" + indent+"" + sepStart +  "BOOKKEEPING INFORMATION:\n"
			else:
				Title=""
				
			# initialize jdl: this operation is not always present
			self.jdl=info [ states_names.index("Jdl") ]
			
		elif hierarchy > self.hierarchy:
			# Hierarchy increased: previous job was father
			Title = "\n"+ "    "*(hierarchy-1)+"- Nodes information for: "
			if self.__level>1:
				Title+= self.jobid+":\n"
			# Managing jobid-node mapping (when possible)
			if not self.jobidMap  and self.jdl:
				# jdl initialised, map still empty
				self.__createJobIdMap__()
		else:
			# Some Indentation Found: it's (not the first) children
			Title= ""
			self.jobid =jobid
		

		
		if json == False:
			Title = Title + "\n" +indent+"Status info for the Job : " + jobid
		else:
			Title = Title + "\"" + jobid + "\": {"
#			Title = Title + "\"ID\": \"" + jobid + "\""
		if self.jobidMap:
			if self.jobidMap.has_key(jobid):
				self.nodeName=self.jobidMap[jobid]
			else:
				errMsg('Warning','UI_SUBJOB_ERROR' , "Unable to find node id:"+jobid+"\ninside jdl of dag:\n"+self.jobid)

		s = info [ states_names.index("Status") ]

		s.rstrip()
		s.lstrip()

		
		
		if json == False:
			Title = Title + "\n" + indent+"Current Status:".ljust(20) + s
		else:
			Title = Title + " " + "\"Current Status\": \"" + s 
		#DONE_CODE check (only on Done )
		done_code_str = ""
		if s in ["Aborted","Cancelled"]:
			# minLog is 1, i.e. even one event is taken into consideration
			s.lstrip()

			#print "\n\n~~~~~~~~~~~ TITLE=[" + Title +"]\n\n"
			
			if json == True:
				Title += "\""
				#				done_code_str = done_code_str +"\""
				
			#print "\n\n~~~~~~~~~~~ TITLE=[" + Title +"]\n\n"

			#print "\n\n~~~~~~~~~~~ DONE_CODE_STR=[" + done_code_str +"]\n\n"
				
			done_code_str += self.__printDoneEvents__(json,jobid,indent,1)

			done_code_str.rstrip()
			done_code_str.lstrip()
			
		elif s=="Done":
			done_code  = info[ states_names.index("Done code") ]
			exit_code    = info[ states_names.index("Exit code") ]
			#PARSING Done code and Exit Code
			try:
				done_code  = int ( done_code )
			except ValueError:
				done_code_str = "(Unable to retrieve  a valid done_code)"
				
			try:
				exit_code  = int ( exit_code )
			except ValueError:
				exit_code = 1
			if done_code== 2:
				done_code_str = "(Cancelled)"
					
				done_code_str += self.__printDoneEvents__(json, jobid,indent)
			elif done_code== 1:
				done_code_str = "(Failed)"
				done_code_str += self.__printDoneEvents__(json,jobid,indent)
			elif done_code== 0 :
				if exit_code==0:
					done_code_str = "(Success)"
					if json == True:
						done_code_str += "\""
					done_code_str += self.__printDoneEvents__(json,jobid,indent)
				else:
					done_code_str = "(Exit Code !=0)"
					if json == True:
						done_code_str += "\""
			else:
				done_code_str = "(Unexpected done code: " + repr(done_code) +" )"
				
				if json == True:
					done_code_str += "\""
				done_code_str += self.__printDoneEvents__(json,jobid,indent)
			
				
			if self.__level:
				#Add exit code information
				if json == False:
					done_code_str = done_code_str +"\n" + indent+"Exit code:".ljust(20) +  repr (exit_code)
				else:
					done_code_str = done_code_str +", \"Exit code\": \"" + repr (exit_code) + "\""

		#print "\n\n~~~~~~~~~~~ DONE_CODE_STR=[" + done_code_str +"]\n\n"
		if done_code_str == "":
			if self.__level == 1:
				if json == True:
					Title = Title.strip()
					if not Title.endswith("\""):
						done_code_str += "\""
	
					
		Title = Title + done_code_str	

#		if json == True:
			#Title.rstrip(" " )
#			Title += "\""
		
		if self.__level:
			reason =  info[ states_names.index("Reason") ]
			if reason.strip():
				if json == False:
					Title = Title +"\n" + indent+"Status Reason:".ljust(20) + reason
				else:
					Title = Title +", \"Status Reason\": \"" + reason + "\""
			dest =  info[ states_names.index("Destination")]
			if dest.strip():
				if json == False:
					Title = Title +"\n" + indent+"Destination:".ljust(20) +  dest
				else:
					Title = Title +", \"Destination\": \"" + dest + "\""
					
			submissionTime =  int   (info[ states_names.index("Stateentertimes")].split(" ")[1].split("=")[1]   )

			if json == False:
				Title = Title + "\n" + indent+"Submitted:".ljust(20) + time.asctime(time.localtime( submissionTime )  ) + DSTime
			else:
				jobIds = {}
				Title = Title + ", \"Submitted\": \"" + time.asctime(time.localtime( submissionTime )  ) + DSTime + "\""
				_wrapStatus = Status(jobid, 0)
				_statusAttribute = _wrapStatus.getStatusAttributes(0)
				_jobInfo = _statusAttribute
				#_jobSchedId = str(_jobInfo[jobid])
				#_job = jobid#jobIds[jobSchedId]
				#_states = states_names
				_stateEnterTimes = states_names.index('Stateentertimes')
				timestamp = str(_jobInfo[_stateEnterTimes])
				#print "\n ************ timestamp=" + str(timestamp) + "\n\n"
#				print "\n####### STATUS=["+s+"]\n\n"
				statiL = timestamp.split()
#				print "\n~~~~~~~~ STATO(1)=["+statiL[1]+"]\n\n"
				for stati in statiL:
					if stati.startswith( s ):
						#print "~~~~~~~~~~~~~ STATO CORRENTE=[" + stati + "]\n\n"
						pieces = stati.split("=")[1]
						if len(pieces) > 1:
							statoTimestamp = stati.split("=")[1]
							Title += ", \"" + s + "\": \""+statoTimestamp+"\""
						#print "~~~~~~~~~~ STATO T=[" + statoTimestamp + "]\n\n"
			if self.__level>2 or hierarchy==0:
				parent = info[ states_names.index("Parent job")]
				if parent.strip():
					if json == False:
						Title+= "\n" +indent + "Parent Job:".ljust(20) + parent
					else:
						Title+= ", \"Parent Job\": \"" + parent + "\""
						
			#Separator between jdl attributes:
			colsep = ""
			if json == False:
				colsep = "\n"+indent+"      "
			
		if self.__level>1:
		    if json == False:
		    	stStr = stStr + "\n" + indent+"---\n\n" + indent+""
			
		    for val in range (len (states_names)-3 ): #cutting last tree parameters: status, status code, hierarchy
			name = states_names[val]
			value = info[val].strip()
			if name and value and value!="(nil)":
				if name in ["Jobid", "Exit code", "Reason", "Parent job"]:
					pass
				elif name == "Condor jdl":
					if self.__level>2:
						value = colsep + value.replace ("\n" , colsep )
						adStr += "- "+ name.ljust(24) + "=    " + value +"\n" + indent+""
				elif name in ["Children hist"]:
					if self.__level>2:
						value = value.replace( " " , "\n".ljust(6) + indent )
						adStr += "- "+ name.ljust(16) + "=   " + value[0] + "\n".ljust(6) + indent + value[1:] +"\n" + indent+""
				elif name in ["Stateentertime"]:
					stateEnterTime = int (info[ states_names.index("Stateentertime")])
					stStr += "- "+ name.ljust(27) + "=    " + time.asctime(time.localtime( stateEnterTime )  ) + DSTime +"\n" + indent+""
				elif name == "Stateentertimes":
					if self.__level>2:
						values = value.split(" ")
						value = ""
						for val in values[1:-1]:
							nam, val = val.split("=")
							nam = nam.ljust(16)
							try:
								val = int (val)
							except:
								val = 0
							if val==0:
								val = "\t\t---"
							else:
								val = time.asctime(time.localtime( val  ) )+ DSTime
							value = value + colsep + nam + " : " + val
						adStr += "- " +name.ljust(16) + "=   " + value +"\n" + indent+""
				elif name in ["Jdl" ]:
					if value[0]=="[":
						#it's a classad
						jobad = AdWrapper (1)
						if jobad.fromString(value):
							err , apiMsg = jobad.get_error ()
							errMsg('Warning','UI_JDL_WRONG_SYNTAX' ,  apiMsg) ;
						else:
							value = jobad.toLines()
							value = colsep + value.replace ("\n" , colsep )
							# Add string only if classad found (and proper level)
							if self.__level>2:
								adStr += "- " +name.ljust(16) + "=   " + value +"\n" + indent+""
							if jobad.hasKey("NodeName"):
								self.nodeName=jobad.getStringValue("NodeName")[0]
				elif name in ["Matched jdl"]:
					if self.__level>2:
						if value[0]=="[":
							#it's a classad
							jobad = AdWrapper (1)
							if jobad.fromString(value):
								err , apiMsg = jobad.get_error ()
								errMsg('Warning','UI_JDL_WRONG_SYNTAX' ,  apiMsg) ;
							value = jobad.toLines()
							value = colsep + value.replace ("\n" , colsep )
							# Add string only if classad found
							adStr += "- " +name.ljust(16) + "=   " + value +"\n" + indent+""
				elif name in [ "Lastupdatetime"]:
					value = time.asctime(time.localtime(  int (value ) ) )+ DSTime
					stStr += "- "+ name.ljust(27) + "=    " + value +"\n" + indent+""
				elif name in ["Rsl"]:
					if self.__level>2:
						value = colsep + value.replace ("\n" , colsep )
						adStr += "- " +name.ljust(16) + "=   " + value +"\n" + indent+""
				else:
					stStr += "- "+ name.ljust(27) + "=    " + value +"\n" + indent+""
					
		if self.nodeName:
			Title = Title + "\n" +indent+"Node Name: ".ljust(20) + self.nodeName
		if adStr:
			adStr = "---\n" + indent+"" +adStr
		
		stStr= Title + stStr +adStr
		
#		if json == True:
#			stStr = "{ " + stStr + " }"
		
		self.hierarchy = hierarchy
		if self.__level:
			if json == False:
				stStr += "\n" +sep
			
		if json == True:
			stStr.rstrip("\n");
			stStr.lstrip("\n");
			stStr.rstrip( );
			stStr.lstrip( );
			stStr += "} "
			
		return stStr

	def __createJobIdMap__(self):
		if self.jdl and self.jdl.strip().startswith("["):
				dagad = DagWrapper()
				if dagad.fromString(self.jdl):
					err , apiMsg = dagad.get_error ()
					errMsg('Warning','UI_JDL_WRONG_SYNTAX' ,  apiMsg)
				else:
					vectMap=dagad.getMap()
					err , apiMsg = dagad.get_error ()
					if err:
						errMsg('Warning','UI_JDL_WRONG_SYNTAX' ,  apiMsg)
					else:
						for i in range(len(vectMap)/2):
							self.jobidMap[vectMap[i*2]] = vectMap[i*2+1]

	def __printDoneEvents__(self,json,jobid,indent,minLog=2):
		"""
		Collect all logged event reasons and create a reasonable string
		jobid is the id of the job to be queried
		indent is the level of indentation for the job
		minLog is the minimal number of logged events to be considered
		(i.e. for DONE jobs 1 reason is already displayed, at least 2 reasons are needed)
		"""
		result=""

		toJoin = list()

		if self.__level:
			# Retrieve DONE events
			reasons = queryCodeEvents(jobid,"Done", minLog, self.__level)
			if len(reasons)>=minLog:
				if json == False:
					result ="\n" +indent+"Logged Reason(s):"
				else:
					result = ", \"Logged Reason\": {"
				counter = 0
				for reason in reasons:
					if json == False:
						result+="\n".ljust(5)+ indent + "- "+ reason
					else:
						_reason = reason.replace("\"", "\\\"");
						toJoin.append( "\"" + str(counter) + "\": \"" + _reason +"\"")
						counter += 1
				if json == True:
					result += ",".join(toJoin)
					result += "}"
					
		return result

	def getJobId(self):
		"""JobStatus getJobId: return the JOB ID of the current Status."""
		# Return the Job ID
		return self.__statusAttributes[states_names.index("Jobid")]

## 	def checkJobs( self, jobid, errors ):
## 	       	"""
## 		check a list of provided job id
## 		"""
		
## 		jobId = jobid
		
## 		try:
		
## 			wrapStatus = Status(jobId, 0)
## 			# Check for Errors
## 			err , apiMsg = wrapStatus.get_error ()
## 			if err:
## 				# Print the error and terminate
## 				raise Exception(apiMsg)
				
## 			# Retrieve the number of status (in this case is always 1)
## 			# statusNumber = wrapStatus.getStatusNumber()
 
##                 # Retrieve the list of attributes for the current UNIQUE event
##                 statusAttribute = wrapStatus.getStatusAttributes(0)
                
##                 # Check for Errors
##                 err , apiMsg = wrapStatus.get_error ()
##                 if err:
##                     # Print the error and terminate
##                     raise Exception(apiMsg)

##                 jobInfo = statusAttribute

##                 # retrieve scheduler Id
##                 jobSchedId = str(jobInfo[self.jobId])

		
## 		# update runningJob
##                 self.getJobInfo(jobInfo, job )
                
##                 #jobIds[jobSchedId] = job
                
##             except Exception, err :
##                 errors.append(
##                     "skipping " + jobId + " : " +  str(err) )
