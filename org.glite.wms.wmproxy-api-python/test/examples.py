#! /usr/bin/env python
import unittest
import SOAPpy
import sys
from wmproxymethods import Wmproxy
from wmproxymethods import Config
import socket

Config.DEBUGMODE = 1
def title(msg, *args):
	if Config.DEBUGMODE:
		print "\n########### DBG Message #################"
		print "* ", msg
		for arg in args:
			print " - ", arg
		print "########### DBG END #################"

class JobId:
	def __init__(self):
		self.jobid=""
	def setJobId(self, jobid):
		title("Successfully set JobId: " , jobid)
		self.jobid=jobid
	def getJobId(self):
		if self.jobid:
			return self.jobid
		print "\n#############################################"
		print "WARNING! JOBID NOT YET SET!!! (Please perform a submission or force it)"
		print "#############################################\n"
		sys.exit(1)




"""
CUSTOM VALUES:
"""
gundam   =  "https://gundam.cnaf.infn.it:7443/glite_wms_wmproxy_server"
ghemon   =  "https://ghemon.cnaf.infn.it:7443/glite_wms_wmproxy_server"
tigerman =  "https://tigerman.cnaf.infn.it:7443/glite_wms_wmproxy_server"
trinity  =  "https://10.100.4.52:7443/glite_wms_wmproxy_server"
url = ghemon






ns ="http://glite.org/wms/wmproxy"

delegationId = "rask"
requirements ="ther.GlueCEStateStatus == \"Production\""
rank ="-other.GlueCEStateEstimatedResponseTime"
"""
		JOBID
"""
jobid = JobId()
dagad = JobId()

#jobid.setJobId("https://gundam.cnaf.infn.it:9000/a3hAXhGJ66tF9hsAlliXzg")
#jobid.setJobId("https://gundam.cnaf.infn.it:9000/FfQ3bgCap3bb8z6K7XF4Wg")
#jobid.setJobId("https://ghemon.cnaf.infn.it:9000/yFcVefR0QEizUXdOHJ-vvg")
#jobid.setJobId("https://tigerman.cnaf.infn.it:9000/JKHQx4dF8OyfH8J1jCyWhw")
#jobid.setJobId("https://ghemon.cnaf.infn.it:9000/Fc1GQj4EFCzXwLdZZr5BQA")
#dagid.setJobId("https://ghemon.cnaf.infn.it:9000/nbHaY_L91fhZ_vcJoWnIdA")

"""
		JDLS
"""
jobjdl ="[ requirements = other.GlueCEStateStatus == \"Production\"; RetryCount = 0; JobType = \"normal\"; Executable = \"/bin/ls\"; Stdoutput = \"std.out\"; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; StdError = \"std.err\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; perusalFileEnable= true; ]"
dagjdl="[ nodes = [ nodeB = [ description = [ requirements = other.GlueCEStateStatus == \"Production\"; JobType = \"normal\"; Executable = \"/bin/date\"; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] ]; dependencies = { { { nodeA },{ nodeB } } }; nodeA = [ description = [ requirements = other.GlueCEStateStatus == \"Production\"; JobType = \"normal\"; Executable = \"/bin/ls\"; StdOutput = \"std.out\"; OutputSandbox = { \"std.err\",\"std.out\" }; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; StdError = \"std.err\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] ] ]; VirtualOrganisation = \"EGEE\"; Type = \"dag\"; node_type = \"edg_jdl\"; enableFilePerusal= true;]"


collectionjdl = "[ requirements = true; RetryCount = 3; nodes = { [ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); NodeName = \"nodeMarask\"; JobType = \"normal\"; executable = \"/bin/ls\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; InputSandbox = { root.inputsandbox[1] } ],[ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); NodeName = \"nodeMaraskino\"; JobType = \"normal\"; executable = \"/bin/ls\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; InputSandbox = { root.inputsandbox[2] } ],[ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); arguments = \"12\"; NodeName = \"nodeMaraska\"; JobType = \"normal\"; executable = \"/bin/sleep\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] }; AllowZippedISB = false; VirtualOrganisation = \"EGEE\"; Type = \"Collection\"; InputSandbox = { \"file:///home/grid_dev/wmproxy/ls.jdl\",\"file:///home/grid_dev/wmproxy/parametric.jdl\",\"file:///home/grid_dev/wmproxy/ENV\" } ]"


collectionjdlUNO = "[ requirements = true; RetryCount = 3; nodes = { [ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); arguments = \"12\"; NodeName = \"nodeMaraska\"; JobType = \"normal\"; executable = \"/bin/sleep\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] }; AllowZippedISB = false; VirtualOrganisation = \"EGEE\"; Type = \"Collection\"; InputSandbox = { \"file:///home/grid_dev/wmproxy/ls.jdl\",\"file:///home/grid_dev/wmproxy/parametric.jdl\",\"file:///home/grid_dev/wmproxy/ENV\" } ]"



jdl=jobjdl


""" DEBUG MODE """
SOAPpy.Config.debug = 0

"""
Setup Test Class for Maraska
"""
class WmpTest(unittest.TestCase):
	"""
	SetUp methods:
	"""
	def setUp(self):
		self.wmproxy = Wmproxy(url, ns)

	def tearDown(self):
		self.wmproxy = ""

	"""
	getSandboxURIs
	"""
	def testgetSandboxDestURI(self):
		assert self.wmproxy.getSandboxDestURI(jobid.getJobId()), "Empty DEST URI!!"
		assert self.wmproxy.getSandboxDestURI(dagad.getJobId()), "Empty DEST URI!!"

	def testgetSandboxBulkDestURI(self):
		assert self.wmproxy.getSandboxBulkDestURI(jobid.getJobId()), "Empty DEST URI!!"
		assert self.wmproxy.getSandboxBulkDestURI(dagad.getJobId()), "Empty DEST URI!!"


	"""
	SUBMISSION
	"""
	def testcollectionSubmit(self):
		dagadInstance=self.wmproxy.jobSubmit(collectionjdlUNO, delegationId)
		dagad.setJobId(dagadInstance.getJobId())
		dagadInstance=self.wmproxy.jobSubmit(collectionjdl, delegationId)
		assert dagadInstance, "Empty DAGAD!!!"
		dagad.setJobId(dagadInstance.getJobId())

	def testdagSubmit(self):
		dagadInstance=self.wmproxy.jobSubmit(dagjdl, delegationId)
		assert dagadInstance, "Empty DAGAD!!!"
		dagad.setJobId(dagadInstance.getJobId())
	def testjobSubmit(self):
		jobidInstance =self.wmproxy.jobSubmit(jobjdl, delegationId)
		assert  jobidInstance , "Empty JobId!!"
		jobid.setJobId(jobidInstance.getJobId())
	def testcycleJob(self):
		for jdl in [jobjdl]:
			title("Cycle Job: Registering..")
			jobidInstance = self.wmproxy.jobRegister(jdl,delegationId)
			jobid.setJobId(jobidInstance.getJobId())
			title("Cycle Job: jobid is:" , jobid.getJobId())
			title("Cycle Job:  getSandboxDestURI...",self.wmproxy.getSandboxDestURI(jobid.getJobId()))
			title("Cycle Job:  getSandboxBulkDestURI...", self.wmproxy.getSandboxBulkDestURI(jobid.getJobId()))
			title("Cycle Job:  getFreeQuota...", self.wmproxy.getFreeQuota())
			title("Cycle Job:  Starting the job....",self.wmproxy.jobStart(jobid.getJobId()))
			title("Cycle Job: FINISH!")

	def testcycleDag(self):
		for jdl in [dagjdl]:
			title("Cycle Job: Registering..")
			dagid= self.wmproxy.jobRegister(jdl,delegationId)
			dagid=dagid.getJobId()
			title("Cycle Job: jobid is:" , dagid)
			title("Cycle Job:  getSandboxDestURI...")
			title(self.wmproxy.getSandboxDestURI(dagid))
			title("Cycle Job:  getSandboxBulkDestURI...")
			title(self.wmproxy.getSandboxBulkDestURI(dagid))
			title("Cycle Job:  getFreeQuota...")
			title(self.wmproxy.getFreeQuota())
			title("Cycle Job:  Starting the job....")
			self.wmproxy.jobStart(dagid)
			title("Cycle Job: FINISH!")
	"""
	TEMPLATES
	"""
	def testgetStringParametricJobTemplate(self):
		attributes  = ["Executable" , "Arguments"]
		param = ["un","dos","tres"]
		assert self.wmproxy.getStringParametricJobTemplate(attributes, param, requirements, rank), "Empty Template!!"
	def testgetIntParametricJobTemplate(self):
		attributes  = ["Executable" , "Arguments"]
		param = 4
		parameterStart=1
		parameterStep=1
		assert self.wmproxy.getIntParametricJobTemplate(attributes, param, parameterStart, parameterStep, requirements, rank), "Empty Template!!"
	def testgetCollectionTemplate(self):
		jobNumber=5
		assert  self.wmproxy.getCollectionTemplate(jobNumber, requirements, rank), "Empty Template!!"
	def testgetJobTemplate(self):
		jobType =[]
		executable ="/bin/ls"
		arguments = "/tmp/*"
		title(self.wmproxy.getJobTemplate(jobType, executable, arguments, requirements, rank))
		assert self.wmproxy.getJobTemplate(jobType, executable, arguments, requirements, rank), "Empty Template!!"
	def testgetDAGTemplate(self):
		dependencies={}
		assert  self.wmproxy.getDAGTemplate(dependencies,requirements, rank), "Empty Template!!"

	"""
	Perusal
	"""
	def testgetPerusalFiles(self):
		file="std.err"
		allChunks = True
		assert self.wmproxy.getPerusalFiles(jobid.getJobId(), file, allChunks), "No Perusal file retrieved (perhaps not yet generated)"
	def testenableFilePerusal(self):
		fileList=["std.out", "std.err"]
		self.wmproxy.enableFilePerusal(jobid.getJobId(), fileList)
	"""
	Proxy
	"""
	def testgetProxyReq(self):
		assert self.wmproxy.getProxyReq(delegationId)
	def testputProxy(self):
		assert self.wmproxy.putProxy(delegationId,jobid.getJobId())
	def testgetProxyReqGrst(self):
		assert self.wmproxy.getProxyReq(delegationId,self.wmproxy.getGrstNs())
	def testputProxyGrst(self):
		assert self.wmproxy.putProxy(delegationId,jobid.getJobId(),self.wmproxy.getGrstNs())

	def testDelegatedProxyInfo(self):
		pi= self.wmproxy.getDelegatedProxyInfo(delegationId)
		title("DELEGATEDPROXY", pi)
		return pi
	def testJobProxyInfo(self):
		pi=self.wmproxy.getJobProxyInfo(jobid.getJobId())
		title("JobProxy:", pi)
		return pi
	def testGetJDL(self):
		#jdlType= wmproxymethods
		for  jdlType in [0,1]:
			pi=self.wmproxy.getJDL(jobid.getJobId(),jdlType)
			title("getJDL:", pi)
		return pi




	"""
	Other
	"""
	def testaddACLItems(self):
		items=["un", "due", "tre", "prova"]
		return self.wmproxy.addACLItems(jobid.getJobId(), items)
def runTextRunner(level=0):
	"""    TEMPLATES   """
	templateSuite = unittest.TestSuite()
	templateSuite.addTest( WmpTest("testgetStringParametricJobTemplate"))
	templateSuite.addTest( WmpTest("testgetIntParametricJobTemplate"))
	templateSuite.addTest( WmpTest("testgetCollectionTemplate"))
	#		templateSuite.addTest( WmpTest("testgetDAGTemplate"))    #"test   NOT YET SUPPORTED"
	#		templateSuite.addTest( WmpTest("testgetJobTemplate"))    # "test  NOT YET SUPPORTED"
	"""  SUBMISSION """
	submitSuite = unittest.TestSuite()
	submitSuite.addTest( WmpTest("testdagSubmit"))
	submitSuite.addTest( WmpTest("testcollectionSubmit"))
	submitSuite.addTest( WmpTest("testjobSubmit"))
	submitSuite.addTest( WmpTest("testcycleJob"))
	""" PERUSAL """
	perusalSuite = unittest.TestSuite()
	perusalSuite.addTest( WmpTest("testgetPerusalFiles"))
	perusalSuite.addTest( WmpTest("testenableFilePerusal"))
	""" get URI"""
	getURISuite = unittest.TestSuite()
	getURISuite.addTest( WmpTest("testgetSandboxDestURI"))
	getURISuite.addTest( WmpTest("testgetSandboxBulkDestURI"))
	""" get/put Proxy"""
	proxySuite = unittest.TestSuite()
	proxySuite.addTest( WmpTest("testgetProxyReq"))
	#		proxySuite.addTest( WmpTest("testputProxy"))	 	#"test NOT YET SUPPORTED"
	proxySuite.addTest( WmpTest("testgetProxyReqGrst"))
	#		proxySuite.addTest( WmpTest("testputProxyGrst"))  	#"test  NOT YET SUPPORTED"
	proxySuite.addTest(WmpTest("testDelegatedProxyInfo"))
	proxySuite.addTest(WmpTest("testJobProxyInfo"))
	proxySuite.addTest(WmpTest("testGetJDL"))


	"""
		RUNNER
	UNCOMMENTED Tests will be executed when "runner" perform is active (see below)
	"""

	runner = unittest.TextTestRunner()
	if level==0 or level ==1:
		runner.run (submitSuite)
	if level==0 or level ==2:
		runner.run (perusalSuite)
	if level==0 or level ==3:
		runner.run (templateSuite)
	if level==0 or level ==4:
		runner.run (getURISuite)
	if level==0 or level ==5:
		runner.run (proxySuite)


def getRemote():
	"""
	Creating Proxy and returning remote instance
	Used for debug test
	"""
	wmproxy = Wmproxy(url, ns)
	wmproxy.soapInit()
	return wmproxy.remote

def getWmproxy():
	"""
	Creating Proxy and returning wmproxy instance
	Used for debug test
	"""
	wmproxy = Wmproxy(url, ns)
	wmproxy.soapInit()
	return wmproxy



"""
				MAIN
"""
def custom():
	# PERFORM wmproxyMethod directly:
	wmproxy = getWmproxy()
	wmproxy = getRemote()
	# return wmproxy.getJobProxyInfo(jobid)
	return wmproxy.jobSubmit(collectionjdlUNO, delegationId)

def custom3(jdl):
	# PERFORM wmproxyMethod directly:
	wmproxy = getRemote()
	return wmproxy.jobSubmit(jdl, delegationId)


def custom2():
	# PERFORM Custom UNIT TEST:
	customSuite= unittest.TestSuite()
	customSuite.addTest( WmpTest("testcollectionSubmit"))
	runner = unittest.TextTestRunner()
	runner.run(customSuite)


if __name__=="__main__":
	print "#############################################"
	print "WMPROXY Service: " , url
	print "DELEGATION used: " , delegationId
	print "#############################################"
	if len(sys.argv)<2:
		print "Usage: "
		print sys.argv[0] , "0-5  [<jobid>]\n"
		print "0) perform all unit tests"
		print "1) perform all submitSuite"
		print "2) perform all perusalSuite"
		print "3) perform all templateSuite"
		print "4) perform getURISuite"
		print "5) perform all proxySuite"
		sys.exit(0)

	level = sys.argv[1]
	if len(sys.argv)>2:
		jobid.setJobId(sys.argv[2])
	try:
		level= int(level)
		if level>5:
			raise 5
	except:
		print "Usage: "
		print sys.argv[0] , "0-5  [<jobid>]\n"
		sys.exit(0)
	runTextRunner(level)
	print " END TEST \n"

