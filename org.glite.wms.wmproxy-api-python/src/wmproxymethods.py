"""
Python API for Wmproxy Web Service
"""

from WMPClient import WMPSOAPProxy
import SOAPpy  #Error Type
import os  #getDefaultProxy method

DEBUGMODE=1
### Static methods/classes: ###

def parseStructType(struct,*fields):
	"""
	with fielsd: Parses a SOAPPy.Types.structType returning a dictionary of parsed attributes
	without fields: Parses a SOAPPy.Types.structType returning a list of parsed attributes
	"""
	if fields:
		result= {}
		for field in fields:
			try:
				result[field]=struct.__getitem__(field)
			except:
				if DEBUGMODE:
					print "parseStructType: Unable to find field: " , field
				result[field]=""
		return result
	else:
		result=[]
		for item in struct:
			result.append(item)
		return result


class JobIdStruct:
	"""
	Output Structure for Register and Submission operation
	each JobIdStructure must have:
	- an identifier, th jobid itself (string)
	moreover it can have:
	- a name, if it is a node of a dag (string)
	- one or more children if it is a dag (list of JobIdStructure(s))
	"""
	def __init__ (self,soapStruct):
		"""
		Default constructor
		"""
   		self.children = []
		self.nodeName = ""
		# Mandatory field
		self.jobid=soapStruct.__getitem__("id")
		# children fill (if present)
		try:
			for child in  soapStruct.__getitem__("childrenJob"):
				self.children.append( JobIdStruct(child) )
		except AttributeError:
			# no children found: it is not a dag
			pass
		try:
			self.nodeName = soapStruct.__getitem__("name")
		except AttributeError:
			# no node name found: it is not a node
			pass

	def __repr__ (self):
		"""
		Equal to getJobId
		"""
		return getJobId()

	def getChildren(self):
		"""
		Return, if the istance represents a dag, the list of all its sons (as a list of JobIdStruct)
		"""
		return self.children
	def getJobId(self):
		"""
		Return the JobId
		"""
		return self.jobid
	def getNodeName(self):
		"""
		Return, if the istance represents a dag node, the name of the node
		"""
		return self.nodeName

class BaseExeption:
	"""
	Base Exception Class deinfe a structure for
	all exception thrown in this module
	"""
	def __repr__(self):
		result ="\n\t "+self.errType+" raised by "+self.origin
		result+="\n\t "+self.error['methodName'] +" " +self.error['Description']
		return result
	def getOrigin(self):
		return self.origin
	def getErrType(self):
		return self.errType
	def getError():
		return self.error

class HTTPException(BaseExeption):
	"""
	Specify a structure for HTTP protocol exceptions
	"""
	def __init__(self, err):
		self.origin  = "HTTP Server"
		self.errType = "Error"
		self.error   = err


class ApiException(BaseExeption):
	"""
	Specify a structure for Server exceptions
	"""
	def __init__(self, method, message):
		self.origin  = "Wmproxy Api Python"
		self.errType = "Method not supported"
		self.error   = { \
			'ErrorCode':"INTERNAL", \
			'methodName':method, \
			'Description':"Method unavailable", \
			'FaultCause':message}


class WMPException(BaseExeption):
	"""
	Specify a structure for Wmproxy Server exceptions
	"""
	def __init__(self, err):
		self.origin  = err[0]
		self.errType = err[1]
		self.error   = parseStructType (err[2][0],'ErrorCode','Timestamp','methodName','Description','FaultCause')

def getDefaultProxy():
	""" retrieve PROXY Default Certificate File name"""
	try:
			return os.environ['X509_USER_PROXY']
	except:
			return '/tmp/x509up_u'+ repr(os.getuid())
def getDefaultNs():
	""" retrieve Wmproxy defaultt namespace name"""
	return "http://glite.org/wms/wmproxy"

class Wmproxy:
	"""
	Provide all WMProxy web services
	"""
	def __init__(self, url, ns="", proxy=""):
		"""
		Default Constructor
		"""
		self.url=url
		self.ns=ns
		self.proxy=proxy
		self.remote=""
		self.init=0

	def soapInit(self):
		#Perform initialisation  (if necessary)
		if self.init==0:
			if not self.proxy:
				self.proxy=getDefaultProxy()
			if not self.ns:
				self.ns=getDefaultNs()
			print "Init calling service for:", self.proxy
			self.remote = WMPSOAPProxy(self.url,namespace=self.ns, key_file=self.proxy, cert_file=self.proxy)
			self.init=1

	def setUrl(self,url):
		"""
		Change/Set the url where to load the WSDL from
		IN =  url(string)
		"""
		self.init=0
		self.url=url

	def setNamespace(self,ns):
		"""
		Change/Set the Wmproxy web server namespace
		IN =  url (string)
		"""
		self.init=0
		self.ns=ns

	def setProxy(self,proxy):
		"""
		Change/Set the proxty certificate file location
		IN =  proxy (string)
		"""
		self.init=0
		self.proxy=proxy

	def jobListMatch(self, jdl, delegationId):
		"""
		Method (tested):  jobListMatch
		IN =  jdl (string)
		IN =  delegationId (string)
		OUT = CEIdAndRankList (StringAndLongList)
		"""
		try:
			self.soapInit()
			result = []
			jlm = self.remote.jobListMatch(jdl, delegationId)[0]
			for i in range (len(jlm)):
				result.append(parseStructType(jlm[i]))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)


	def getCollectionTemplate(self, jobNumber, requirements, rank):
		"""
		Method (not tested):  getCollectionTemplate
		IN =  jobNumber (int)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		raise ApiException("getCollectionTemplate","not yet supported")
		try:
			self.soapInit()
			return self.remote.getCollectionTemplate(jobNumber, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)


	def jobPurge(self, jobId):
		"""
		Method (tested):  jobPurge
		IN =  jobId (string)
		"""
		try:
			self.soapInit()
			self.remote.jobPurge(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def jobStart(self, jobId):
		"""
		Method (tested):  jobStart
		IN =  jobId (string)
		"""
		try:
			self.soapInit()
			self.remote.jobStart(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def putProxy(self, delegationID, proxy):
		"""
		Method (tested):  putProxy
		ProxyOperationException: Proxy exception: Provided delegation id not valid
		IN =  delegationID (string)
		IN =  proxy (string)
		"""
		try:
			self.soapInit()
			self.remote.putProxy(delegationID, proxy)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)


	def addACLItems(self, jobId, items):
		"""
		Method (not tested):  addACLItems
		IN =  jobId (string)
		IN =  items (StringList)
		"""
		try:
			self.soapInit()
			self.remote.addACLItems(jobId, items)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)


	def getACLItems(self, jobId):
		"""
		Method (tested):  getACLItems
		IN =  jobId (string)
		OUT = a list of strings containing the ACL Items to add.
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getACLItems(jobId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getSandboxBulkDestURI(self, jobId):
		"""
		Method (tested):  getSandboxBulkDestURI
		IN =  jobId (string)
		OUT = A dictonary containing, for each jobid (string), its destUris in all available protocols (list of strings)
		"""
		destUris={}
		try:
			self.soapInit()
			dests= self.remote.getSandboxBulkDestURI(jobId)[0]
			for dest in dests:
				destUris[ dest.__getitem__("id")]=dest.__getitem__("Item")
			return destUris
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getSandboxDestURI(self, jobId):
		"""
		Method (tested):  getSandboxDestURI
		TBD test better
		IN =  jobId (string)
		OUT = dstUri in all available protocols( list of strings)
		"""
		try:
			self.soapInit()
			return self.remote.getSandboxDestURI(jobId)[0]
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def jobCancel(self, jobId):
		"""
		Method (tested):  jobCancel
		IN =  jobId (string)
		"""
		try:
			self.soapInit()
			self.remote.jobCancel(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getStringParametricJobTemplate(self, attributes, param, requirements, rank):
		"""
		Method (not tested):  getStringParametricJobTemplate
		IN =  attributes (StringList)
		IN =  param (StringList)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		raise ApiException("getStringParametricJobTemplate","not yet supported")
		try:
			self.soapInit()
			return self.remote.getStringParametricJobTemplate(attributes, param, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getJobTemplate(self, jobType, executable, arguments, requirements, rank):
		"""
		Method (not tested):  getJobTemplate
		IN =  jobType (JobTypeList)
		IN =  executable (string)
		IN =  arguments (string)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		raise ApiException("getJobTemplate","not yet supported")
		try:
			self.soapInit()
			return self.remote.getJobTemplate(jobType, executable, arguments, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getFreeQuota(self):
		"""
		Method (tested):  getFreeQuota
		AuthorizationException: LCMAPS failed to map user credential
		OUT = softLimit (long)
		OUT = hardLimit (long)
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getFreeQuota(),'softLimit''hardLimit')
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)


	def getProxyReq(self, delegationID):
		"""
		Method (tested):  getProxyReq
		ProxyOperationException: Proxy exception: Provided delegation id not valid
		IN =  delegationID (string)
		OUT = request (string)
		"""
		try:
			self.soapInit()
			return self.remote.getProxyReq(delegationID)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getVersion(self):
		"""
		Method (tested):  getVersion
		OUT = version (string)
		"""
		try:
			self.soapInit()
			return self.remote.getVersion()
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getIntParametricJobTemplate(self, attributes, param, parameterStart, parameterStep, requirements, rank):
		"""
		Method (not tested):  getIntParametricJobTemplate
		IN =  attributes (StringList)
		IN =  param (int)
		IN =  parameterStart (int)
		IN =  parameterStep (int)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		raise ApiException("getIntParametricJobTemplate","not yet supported")
		try:
			self.soapInit()
			return self.remote.getIntParametricJobTemplate(attributes, param, parameterStart, parameterStep, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)


	def getDAGTemplate(self, dependencies, requirements, rank):
		"""
		Method (not tested):  getDAGTemplate
		IN =  dependencies (GraphStructType)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		raise ApiException("getDAGTemplate","not yet supported")
		try:
			self.soapInit()
			return self.remote.getDAGTemplate(dependencies, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def jobSubmit(self, jdl, delegationId):
		"""
		Method (tested):  jobSubmit
		IN =  jdl (string)
		IN =  delegationId (string)
		OUT = jobIdStruct (JobIdStruct)
		"""
		try:
			self.soapInit()
			return JobIdStruct(self.remote.jobSubmit(jdl, delegationId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)


	def jobRegister(self, jdl, delegationId):
		"""
		Method (tested):  jobRegister
		IN =  jdl (string)
		IN =  delegationId (string)
		OUT = jobIdStruct (JobIdStruct)
		"""
		try:
			self.soapInit()
			return JobIdStruct(self.remote.jobRegister(jdl, delegationId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def removeACLItem(self, jobId, item):
		"""
		Method (not tested ):  removeACLItem
		IN =  jobId (string)
		IN =  item (string)
		"""
		raise ApiException("removeACLItem","not yet supported")
		try:
			self.soapInit()
			self.remote.removeACLItem(jobId, item)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getMaxInputSandboxSize(self):
		"""
		Method (tested):  getMaxInputSandboxSize
		OUT = size (long)
		"""
		try:
			self.soapInit()
			return int(self.remote.getMaxInputSandboxSize())
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)


	def getTotalQuota(self):
		"""
		Method (tested):  getTotalQuota
		AuthorizationException: LCMAPS failed to map user credential
		OUT = softLimit (long)
		OUT = hardLimit (long)
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getTotalQuota(),'softLimit''hardLimit')
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getOutputFileList(self, jobId):
		"""
		Method (tested):  getOutputFileList
		IN =  jobId (string)
		OUT = OutputFileAndSizeList (StringAndLongList)
		"""
		try:
			self.soapInit()
			return self.remote.getOutputFileList(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

	def getDelegatedProxyInfo(self, jobId):
		"""
		Method (tested):  getDelegatedProxyInfo
		IN =  jobId (string)
		OUT = list of strings containing Delegated Proxy information
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getDelegatedProxyInfo(jobId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)

