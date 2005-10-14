"""
Python API for Wmproxy Web Service
"""

from WMPClient import WMPSOAPProxy
import SOAPpy  #Error Type
import os  #getDefaultProxy method
import socket # socket error mapping

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
		Equal to toString
		"""
		return self.getJobId()

	def getChildren(self):
		"""
		Return, if the istance represents a dag, the list of all its sons (as a list of JobIdStruct)
		"""
		return self.children
	def toString(self):
		"""
		Return the JobId string representation
		"""
		return self.jobid
	def getJobId(self):
		"""
		Equal to toString
		"""
		return self.jobid
	def getNodeName(self):
		"""
		Return, if the istance represents a dag node, the name of the node
		"""
		return self.nodeName

class BaseException(Exception):
	"""
	Base Exception Class deinfe a structure for
	all exception thrown in this module
	"""
	origin  = ""
	errType = ""
	methodName =""
	description=""
	args=[]

	def __repr__(self):
		result ="\n\t "
		if self.errType:
			result+=self.errType
		if self.origin:
			result+=" raised by "+self.origin
		if self.methodName:
			result+="\n\t "+self.methodName
		if self.description:
			result+=" " + self.description
		if DEBUGMODE:
			print "Debug mode ON"
			for fc in self.args:
				try:
					result+="\n\t "+ fc
				except:
					result+="\n\t "+ repr(fc)
		return result


class HTTPException(BaseException):
	"""
	Specify a structure for HTTP protocol exceptions
	"""
	def __init__(self, err):
		self.origin  = "HTTP Server"
		self.errType = "Error"
		self.error   = err
		self.methodName =""
		self.description=""

class SocketException(BaseException):
	"""
	Socket-Connection Error
	input: a socket.err error
	"""
	def __init__(self, err):
		self.origin  = "Socket Connection"
		self.errType = "Error"
		self.errorCode   = 105
		for ar in err.args:
			self.args.append(ar)

class WMPException(BaseException):
	"""
	Specify a structure for Wmproxy Server exceptions
	"""
	def __init__(self, err):
		try:
			self.origin  = err[0]
			self.errType = err[1]
			error = err[2][0]
			self.errorCode   = error["ErrorCode"]
			self.timestamp   = error["Timestamp"]
			self.methodName  = error["methodName"]
			self.description = error["Description"]
			for ar in error["FaultCause"]:
				self.args.append(ar)
		except:
			raise err


class ApiException(BaseException):
	"""
	Exception raised directly from server
	"""
	def __init__(self, method, message):
		self.origin  = "Wmproxy Api Python"
		self.errType = "Method not supported"
		self.errorCode   = 105
		self.methodName  = method
		self.description = message


def getDefaultProxy():
	""" retrieve PROXY Default Certificate File name"""
	try:
			return os.environ['X509_USER_PROXY']
	except:
			return '/tmp/x509up_u'+ repr(os.getuid())
def getDefaultNs():
	"""
	retrieve Wmproxy default namespace string representation
	"""
	return "http://glite.org/wms/wmproxy"
def getGrstNs():
	"""
	GridSite Delegation namespace string representation
	PutProxy and getProxyReq services requires gridsite specific namespace
	WARNING: for backward compatibility with WMPROXY server (version <= 1.x.x)
	deprecated PutProxy and getProxyReq sevices are still provided with
	wmproxy default namespace
	"""
	return "http://www.gridsite.org/namespaces/delegation-1"
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

	def methods(self):
		self.soapInit()
		return self.remote.methods

	"""
	ACTUAL WEB SERVICE METHODS
	"""
	def jobListMatch(self, jdl, delegationId):
		"""
		Method:  jobListMatch
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
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getCollectionTemplate(self, jobNumber, requirements, rank):
		"""
		Method:  getCollectionTemplate
		IN =  jobNumber (int)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		try:
			self.soapInit()
			return self.remote.getCollectionTemplate(jobNumber, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)


	def jobPurge(self, jobId):
		"""
		Method:  jobPurge
		IN =  jobId (string)
		"""
		try:
			self.soapInit()
			self.remote.jobPurge(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def jobStart(self, jobId):
		"""
		Method:  jobStart
		IN =  jobId (string)
		"""
		try:
			self.soapInit()
			self.remote.jobStart(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)


	def addACLItems(self, jobId, items):
		"""
		Method:  addACLItems
		IN =  jobId (string)
		IN =  items (StringList)
		"""
		try:
			self.soapInit()
			self.remote.addACLItems(jobId, items)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)


	def getACLItems(self, jobId):
		"""
		Method:  getACLItems
		IN =  jobId (string)
		OUT = a list of strings containing the ACL Items to add.
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getACLItems(jobId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getSandboxBulkDestURI(self, jobId):
		"""
		Method:  getSandboxBulkDestURI
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
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getSandboxDestURI(self, jobId):
		"""
		Method:  getSandboxDestURI
		TBD test better
		IN =  jobId (string)
		OUT = dstUri in all available protocols( list of strings)
		"""
		try:
			self.soapInit()
			return self.remote.getSandboxDestURI(jobId)[0]
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def jobCancel(self, jobId):
		"""
		Method:  jobCancel
		IN =  jobId (string)
		"""
		try:
			self.soapInit()
			self.remote.jobCancel(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getStringParametricJobTemplate(self, attributes, param, requirements, rank):
		"""
		Method:  getStringParametricJobTemplate
		IN =  attributes (StringList)
		IN =  param (StringList)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		try:
			self.soapInit()
			return self.remote.getStringParametricJobTemplate(attributes, param, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

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
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getFreeQuota(self):
		"""
		Method:  getFreeQuota
		OUT = softLimit (long)
		OUT = hardLimit (long)
		return a dictionary with soft&hard Limits
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getFreeQuota(),'softLimit''hardLimit')
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def putProxy(self, delegationID, proxy):
		"""
		Method:  putProxy
		ProxyOperationException: Proxy exception: Provided delegation id not valid
		WARNING: for backward compatibility putProxy is provided with both namespaces:
		defaultWmproxy namespace for WMPROXY servers (version <= 1.x.x)
		gridsite namespace for WMPROXY servers (version > 1.x.x)
		see getDefaultProxy() and  getDefaultNs() methods
		IN =  delegationID (string)
		IN =  proxy (string)
		"""
		try:
			self.soapInit()
			self.remote.putProxy(delegationID, proxy)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getProxyReq(self, delegationID):
		"""
		Method:  getProxyReq
		WARNING: for backward compatibility getProxyReq is provided with both namespaces:
		defaultWmproxy namespace for WMPROXY servers (version <= 1.x.x)
		gridsite namespace for WMPROXY servers (version > 1.x.x)
		see getDefaultProxy() and  getDefaultNs() methods
		IN =  delegationID (string)
		OUT = request (string)
		"""
		try:
			self.soapInit()
			return self.remote.getProxyReq(delegationID)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getVersion(self):
		"""
		Method:  getVersion
		OUT = version (string)
		"""
		try:
			self.soapInit()
			return self.remote.getVersion()
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getIntParametricJobTemplate(self, attributes, param, parameterStart, parameterStep, requirements, rank):
		"""
		Method:  getIntParametricJobTemplate
		IN =  attributes (StringList)
		IN =  param (int)
		IN =  parameterStart (int)
		IN =  parameterStep (int)
		IN =  requirements (string)
		IN =  rank (string)
		OUT = jdl (string)
		"""
		try:
			self.soapInit()
			return self.remote.getIntParametricJobTemplate(attributes, param, parameterStart, parameterStep, requirements, rank)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)


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
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def jobSubmit(self, jdl, delegationId):
		"""
		Method:  jobSubmit
		IN =  jdl (string)
		IN =  delegationId (string)
		OUT = jobIdStruct (JobIdStruct)
		"""
		try:
			self.soapInit()
			return JobIdStruct(self.remote.jobSubmit(jdl, delegationId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)


	def jobRegister(self, jdl, delegationId):
		"""
		Method:  jobRegister
		IN =  jdl (string)
		IN =  delegationId (string)
		OUT = jobIdStruct (JobIdStruct)
		"""
		try:
			self.soapInit()
			return JobIdStruct(self.remote.jobRegister(jdl, delegationId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def removeACLItem(self, jobId, item):
		"""
		Method (not tested):  removeACLItem
		IN =  jobId (string)
		IN =  item (string)
		"""
		raise ApiException("removeACLItem","not yet supported")
		try:
			self.soapInit()
			self.remote.removeACLItem(jobId, item)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getMaxInputSandboxSize(self):
		"""
		Method:  getMaxInputSandboxSize
		OUT = size (long)
		"""
		try:
			self.soapInit()
			return int(self.remote.getMaxInputSandboxSize())
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)


	def getTotalQuota(self):
		"""
		Method:  getTotalQuota
		AuthorizationException: LCMAPS failed to map user credential
		OUT = softLimit (long)
		OUT = hardLimit (long)
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getTotalQuota(),'softLimit''hardLimit')
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getOutputFileList(self, jobId):
		"""
		Method:  getOutputFileList
		IN =  jobId (string)
		OUT = OutputFileAndSizeList (StringAndLongList)
		"""
		try:
			self.soapInit()
			return self.remote.getOutputFileList(jobId)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

	def getDelegatedProxyInfo(self, jobId):
		"""
		Method:  getDelegatedProxyInfo
		IN =  jobId (string)
		OUT = list of strings containing Delegated Proxy information
		"""
		try:
			self.soapInit()
			return parseStructType(self.remote.getDelegatedProxyInfo(jobId))
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

        def getPerusalFiles(self, jobId, file, allChunks):
		"""
		Method: getPerusalFiles
		IN =  jobId (string)
		IN =  file (string)
		IN =  allChunks (boolean)
		OUT = fileList (StringList)
		"""
		try:
			self.soapInit()
			files = parseStructType(self.remote.getPerusalFiles(jobId, file, allChunks))
			if not files:
				return []
			else:
				files=files[0]
				if type(files) == type("str"):
					return [files]
				else:
					return files
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

        def enableFilePerusal(self, jobId, fileList):
		"""
		Method: enableFilePerusal
		IN =  jobId (string)
		IN =  fileList (StringList)
		"""
		try:
			self.soapInit()
			self.remote.enableFilePerusal(jobId, fileList)
		except SOAPpy.Types.faultType, err:
			raise WMPException(err)
		except SOAPpy.Errors.HTTPError, err:
			raise HTTPException(err)
		except socket.error, err:
			raise SocketException(err)

