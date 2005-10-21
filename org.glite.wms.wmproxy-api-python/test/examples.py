#! /usr/bin/env python2.2
import unittest
import SOAPpy
import sys
from wmproxymethods import Wmproxy
from wmproxymethods import Config
import socket

Config.DEBUGMODE = 0

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
jobid="https://gundam.cnaf.infn.it:9000/WUoS9JuNa66lm8nB_LgdvA"
jobid="https://gundam.cnaf.infn.it:9000/a3hAXhGJ66tF9hsAlliXzg"
jobid="https://gundam.cnaf.infn.it:9000/FfQ3bgCap3bb8z6K7XF4Wg"
jobid="https://ghemon.cnaf.infn.it:9000/yFcVefR0QEizUXdOHJ-vvg"
#jobid="https://tigerman.cnaf.infn.it:9000/JKHQx4dF8OyfH8J1jCyWhw"
jobid="https://ghemon.cnaf.infn.it:9000/Fc1GQj4EFCzXwLdZZr5BQA"
dagid="https://ghemon.cnaf.infn.it:9000/nbHaY_L91fhZ_vcJoWnIdA"

"""
		JDLS
"""
jobjdl ="[ requirements = other.GlueCEStateStatus == \"Production\"; RetryCount = 0; JobType = \"normal\"; Executable = \"/bin/ls\"; Stdoutput = \"std.out\"; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; StdError = \"std.err\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; enableFilePerusal= true; ]"
dagjdl="[ nodes = [ nodeB = [ description = [ requirements = other.GlueCEStateStatus == \"Production\"; JobType = \"normal\"; Executable = \"/bin/date\"; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] ]; dependencies = { { { nodeA },{ nodeB } } }; nodeA = [ description = [ requirements = other.GlueCEStateStatus == \"Production\"; JobType = \"normal\"; Executable = \"/bin/ls\"; StdOutput = \"std.out\"; OutputSandbox = { \"std.err\",\"std.out\" }; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; StdError = \"std.err\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] ] ]; VirtualOrganisation = \"EGEE\"; Type = \"dag\"; node_type = \"edg_jdl\"; enableFilePerusal= true;]"


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
		assert self.wmproxy.getSandboxDestURI(jobid), "Empty DEST URI!!"
		assert self.wmproxy.getSandboxDestURI(dagad), "Empty DEST URI!!"
	def testgetSandboxBulkDestURI(self):
		assert self.wmproxy.getSandboxBulkDestURI(jobid), "Empty DEST URI!!"
		assert self.wmproxy.getSandboxBulkDestURI(dagad), "Empty DEST URI!!"


	"""
	SUBMISSION
	"""
	def testdagSubmit(self):
		assert  self.wmproxy.jobSubmit(dagjdl, delegationId)
	def testjobSubmit(self):
		assert  self.wmproxy.jobSubmit(jobjdl, delegationId), "Empty JobId!!"
	def cycleJob(self):
		for jdl in [jobjdl, dagjdl]:
			print "Cycle Job: Registering.."
			jobid = self.wmproxy.jobRegister(jdl,delegationId)
			print "Cycle Job: jobid is:" , jobid
			print"Cycle Job:  getSandboxDestURI..."
			print self.wmproxy.getSandboxDestURI(jobid.toString())
			print "Cycle Job:  getSandboxBulkDestURI..."
			print self.wmproxy.getSandboxBulkDestURI(jobid.toString())
			print "Cycle Job:  getFreeQuota..."
			print self.wmproxy.getFreeQuota()
			print "Cycle Job:  Starting the job...."
			self.wmproxy.jobStart(jobid.toString())
			print "Cycle Job: FINISH!"
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
		jobType ={}
		executable ="/bin/ls"
		arguments = "/tmp/*"
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
		assert self.wmproxy.getPerusalFiles(jobid, file, allChunks), "No Perusal file retrieved (perhaps not yet generated)"
	def testenableFilePerusal(self):
		fileList=["std.out", "std.err"]
		self.wmproxy.enableFilePerusal(jobid, fileList)
	"""
	Proxy
	"""
	def testgetProxyReq(self):
		assert self.wmproxy.getProxyReq(delegationId)
	def testputProxy(self):
		assert self.wmproxy.putProxy(delegationId,jobid)

	def testgetProxyReqGrst(self):
		assert self.wmproxy.getProxyReq(delegationId,self.wmproxy.getGrstNs())
	def testputProxyGrst(self):
		assert self.wmproxy.putProxy(delegationId,jobid,self.wmproxy.getGrstNs())

	"""
	Other
	"""
	def testaddACLItems(self):
		items=["un", "due", "tre", "prova"]
		return self.wmproxy.addACLItems(jobid, items)
def runTextRunner():
	"""    TEMPLATES   """
	templateSuite = unittest.TestSuite()
	templateSuite.addTest( WmpTest("testgetStringParametricJobTemplate"))
	templateSuite.addTest( WmpTest("testgetIntParametricJobTemplate"))
	templateSuite.addTest( WmpTest("testgetCollectionTemplate"))
	#templateSuite.addTest( WmpTest("testgetDAGTemplate") "testgetDAGTemplate NOT YET SUPPORTED"
	#templateSuite.addTest( WmpTest("testgetJobTemplate")) "testgetJobTemplate NOT YET SUPPORTED"
	"""  SUBMISSION """
	submitSuite = unittest.TestSuite()
	submitSuite.addTest( WmpTest("testdagSubmit"))
	submitSuite.addTest( WmpTest("testjobSubmit"))
	submitSuite.addTest( WmpTest("cycleJob"))
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
	#proxySuite.addTest( WmpTest("testputProxy"))
	proxySuite.addTest( WmpTest("testgetProxyReqGrst"))
	#proxySuite.addTest( WmpTest("testputProxyGrst"))
	""" RUNNER """
	runner = unittest.TextTestRunner()
	#runner.run (perusalSuite)
	#runner.run (submitSuite)
	#runner.run (templateSuite)
	#runner.run (getURISuite)
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
print "#############################################"
print "WMPROXY Service: " , url
print "DELEGATION used: " , delegationId
print "JobId:           " , jobid
print "                  (only for jobid methods..)"
print "#############################################"


perform = "all"
perform = "runner"
#perform = "main"

if __name__=="__main__":
	if perform=="all":
		unittest.main()
	elif perform == "runner":
		runTextRunner()
	else:
		main()
	print " END TEST \n"



