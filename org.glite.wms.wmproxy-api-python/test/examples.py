#! /usr/bin/env python
import unittest
import SOAPpy
import sys
from wmproxymethods import Wmproxy
from wmproxymethods import Config
import socket


""" DEBUG MODE """
SOAPpy.Config.debug = 0
Config.DEBUGMODE = 1


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


def title(msg, *args):
	if Config.DEBUGMODE:
		print "\n########### DBG Message #################"
		print "* ", msg
		for arg in args:
			print " - ", arg
		print "########### DBG END #################"

class JobId:
	DEFAULT_JOBID="https://gundam.cnaf.infn.it:9000/a3hAXhGJ66tF9hsAlliXzg"
	def __init__(self):
		self.jobid=""
	def setJobId(self, jobid):
		title("Successfully set JobId: " , jobid)
		self.jobid=jobid
		return jobid
	def getJobId(self):
		if self.jobid:
			return self.jobid
		else:
			title ("WARNING: using DEFAULT JOBID",self.DEFAULT_JOBID)
			return self.DEFAULT_JOBID
		sys.exit(1)

class Jdl:
	DEFAULT_JDL="[ requirements = other.GlueCEStateStatus == \"Production\"; RetryCount = 0; JobType = \"normal\"; Executable = \"/bin/ls\"; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\";]"
	def __init__(self):
		self.jdl=""
	def loadJdl(self, jdlFile):
		jdlF  = open(jdlFile)
		lines = jdlF.readlines()
		jdlF.close()
		jdl=""
		for line in lines:
			jdl+=line
		return self.setJdl(jdl)
	def setJdl(self, jdl):
		title("Successfully set Jdl: " , jdl)
		self.jdl=jdl
		return jdl
	def getJdl(self, defaultJdl=""):
		if self.jdl:
			return self.jdl
		elif defaultJdl:
			return defaultJdl
		else:
			tile ("WARNING: using DEFAULT JDL",self.DEFAULT_JDL )
			return self.DEFAULT_JDL


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
protocol="gsiftp"
protocol="all"
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

dagjdl2="[ nodes = [ nodeB = [ description = [ requirements = RegExp(\"lxde01*\",other.GlueCEUniqueID); JobType = \"normal\"; Executable = \"/bin/date\"; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] ]; dependencies = { { { nodeA },{ nodeB } } }; nodeA = [ description = [ requirements = RegExp(\"lxde01*\",other.GlueCEUniqueID); JobType = \"normal\"; Executable = \"/bin/ls\"; StdOutput = \"std.out\"; OutputSandbox = { \"std.err\",\"std.out\" }; VirtualOrganisation = \"EGEE\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; StdError = \"std.err\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] ] ]; AllowZippedISB = false; VirtualOrganisation = \"EGEE\"; Type = \"dag\"; node_type = \"edg_jdl\" ]"



collectionjdl = "[ requirements = true; RetryCount = 3; nodes = { [ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); NodeName = \"nodeMarask\"; JobType = \"normal\"; executable = \"/bin/ls\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; InputSandbox = { root.inputsandbox[1] } ],[ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); NodeName = \"nodeMaraskino\"; JobType = \"normal\"; executable = \"/bin/ls\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; InputSandbox = { root.inputsandbox[2] } ],[ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); arguments = \"12\"; NodeName = \"nodeMaraska\"; JobType = \"normal\"; executable = \"/bin/sleep\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] }; AllowZippedISB = false; VirtualOrganisation = \"EGEE\"; Type = \"Collection\"; InputSandbox = { \"file:///home/grid_dev/wmproxy/ls.jdl\",\"file:///home/grid_dev/wmproxy/parametric.jdl\",\"file:///home/grid_dev/wmproxy/ENV\" } ]"


collectionjdl = "[ requirements = other.GlueCEInfoTotalCPUs>0; RetryCount = 3; nodes = { [ NodeName = \"nodeMarask\"; JobType = \"normal\"; executable = \"/bin/ls\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; ],[NodeName = \"nodeMaraskino\"; JobType = \"normal\"; executable = \"/bin/ls\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; ],[arguments = \"12\"; NodeName = \"nodeMaraska\"; JobType = \"normal\"; executable = \"/bin/sleep\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] }; AllowZippedISB = false; VirtualOrganisation = \"EGEE\"; Type = \"Collection\"]"

collectionjdlUNO = "[ requirements = true; RetryCount = 3; nodes = { [ requirements = ( true ) && ( other.GlueCEStateStatus == \"Production\" ); arguments = \"12\"; NodeName = \"nodeMaraska\"; JobType = \"normal\"; executable = \"/bin/sleep\"; rank =  -other.GlueCEStateEstimatedResponseTime; Type = \"job\"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime ] }; AllowZippedISB = false; VirtualOrganisation = \"EGEE\"; Type = \"Collection\"; InputSandbox = { \"file:///home/grid_dev/wmproxy/ls.jdl\",\"file:///home/grid_dev/wmproxy/parametric.jdl\",\"file:///home/grid_dev/wmproxy/ENV\" } ]"


"""
JDL Instance
"""
jdl = Jdl()

# LEVEL/SUBLEVEL MENU
LEV_DEFAULT=-2
LEV_HELP=-1
LEV_MAX=5


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
	SANDBOX / OUTPUT
	"""
	def testgetSandboxDestURI(self):
		jobURI = self.wmproxy.getSandboxDestURI(jobid.getJobId(),protocol)
		dagURI = self.wmproxy.getSandboxDestURI(dagad.getJobId(),protocol)
		title("testgetSandboxDestURI: ", jobURI,dagURI)
		assert jobURI, "Wrong DEST URI!! (jobid)"
		assert dagURI, "Wrong DEST URI!! (dagad)"

	def testgetSandboxBulkDestURI(self):
		protocol=""
		jobURI=self.wmproxy.getSandboxBulkDestURI(jobid.getJobId(),protocol)
		#dagURI=self.wmproxy.getSandboxBulkDestURI(dagad.getJobId(),protocol)
		dagURI=""
		title("testgetSandboxBulkDestURI: ", jobURI,dagURI)
		if jobURI:
			for jid in jobURI.keys():
				uris = jobURI[jid]
				for uri in uris:
					print "URI RECEIVED: " , uri

		assert jobURI, "Wrong DEST URI!! (jobid)"
		# assert dagURI, "Wrong DEST URI!! (dagad)"
		if jobURI:
			for jid in jobURI.keys():
				uris = jobURI[jid]
				"URIS DAG", uris


	def testgetTransferProtocols(self):
		protocols = self.wmproxy.getTransferProtocols()
		title ("transferProtocols are:", protocols)

	def testgetOutputFileList(self):
		jobFL = self.wmproxy.getOutputFileList(jobid.getJobId(),protocol)
		dagFL = self.wmproxy.getOutputFileList(dagad.getJobId(),protocol)
		title("getOutputFiles WITH protocol are (both of them might be empty):","JOB", jobFL, "DAG",dagFL)
		jobFL = self.wmproxy.getOutputFileList(jobid.getJobId())
		dagFL = self.wmproxy.getOutputFileList(dagad.getJobId())
		title("getOutputFiles WITHOUT protocol are (both of them might be empty):","JOB", jobFL, "DAG",dagFL)

	"""
	SUBMISSION
	"""
	def testcollectionSubmit(self):
		title("testcollectionSubmit")
		dagadInstance=self.wmproxy.jobSubmit(jdl.getJdl(collectionjdl), delegationId)
		assert dagadInstance, "Empty DAGAD!!!"
		dagad.setJobId(dagadInstance.getJobId())

	def testcollectionSubmitOne(self):
		title("testcollectionSubmitOne")
		dagadInstance=self.wmproxy.jobSubmit(jdl.getJdl(collectionjdlUNO), delegationId)
		assert dagadInstance, "Empty DAGAD!!!"
		dagad.setJobId(dagadInstance.getJobId())

	def testdagSubmit(self):
		title("testdagSubmit")
		dagadInstance=self.wmproxy.jobSubmit(jdl.getJdl(dagjdl2), delegationId)
		assert dagadInstance, "Empty DAGAD!!!"
		dagad.setJobId(dagadInstance.getJobId())
	def testjobSubmit(self):
		title("testjobSubmit")
		jobidInstance =self.wmproxy.jobSubmit(jdl.getJdl(jobjdl), delegationId)
		assert  jobidInstance , "Empty JobId!!"
		jobid.setJobId(jobidInstance.getJobId())

	def testjobListMatch(self):
		title("testcollectionSubmitOne")
		matchingCEs=self.wmproxy.jobListMatch(jdl.getJdl(jobjdl), delegationId)
		assert  matchingCEs , "Empty JobId!!"


	def testcycleJob(self):
		for jdl in [jdl.getJdl(jobjdl)]:
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
		for jdl in [jdl.getJdl(dagjdl)]:
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
		title("testgetStringParametricJobTemplate")
		attributes  = ["Executable" , "Arguments"]
		param = ["un","dos","tres"]
		assert self.wmproxy.getStringParametricJobTemplate(attributes, param, requirements, rank), "Empty Template!!"
	def testgetIntParametricJobTemplate(self):
		title("testgetIntParametricJobTemplate")
		attributes  = ["Executable" , "Arguments"]
		param = 4
		parameterStart=1
		parameterStep=1
		assert self.wmproxy.getIntParametricJobTemplate(attributes, param, parameterStart, parameterStep, requirements, rank), "Empty Template!!"
	def testgetCollectionTemplate(self):
		title("testgetCollectionTemplate")
		jobNumber=5
		assert  self.wmproxy.getCollectionTemplate(jobNumber, requirements, rank), "Empty Template!!"
	def testgetJobTemplate(self):
		title("testgetJobTemplate")
		jobType =[]
		executable ="/bin/ls"
		arguments = "/tmp/*"
		title(self.wmproxy.getJobTemplate(jobType, executable, arguments, requirements, rank))
		assert self.wmproxy.getJobTemplate(jobType, executable, arguments, requirements, rank), "Empty Template!!"
	def testgetDAGTemplate(self):
		title("testgetDAGTemplate")
		dependencies={}
		assert  self.wmproxy.getDAGTemplate(dependencies,requirements, rank), "Empty Template!!"

	"""
	Perusal
	"""
	def testgetPerusalFiles(self):
		title("testgetPerusalFiles")
		file="std.err"
		allChunks = True
		assert self.wmproxy.getPerusalFiles(jobid.getJobId(), file, allChunks), "No Perusal file retrieved (perhaps not yet generated)"
	def testenableFilePerusal(self):
		title("testenableFilePerusal")
		fileList=["std.out", "std.err"]
		self.wmproxy.enableFilePerusal(jobid.getJobId(), fileList)
	"""
	Proxy
	"""
	def testgetProxyReq(self):
		gpr = self.wmproxy.getProxyReq(delegationId)
		title("getProxyReq (wmp namespace)", gpr)
		assert gpr
		gpr = self.wmproxy.getProxyReq(delegationId, self.wmproxy.getGrstNs())
		title("getProxyReq (grst namespace)", gpr)
		assert gpr
	def testputProxy(self):
		title("testputProxy")
		assert self.wmproxy.putProxy(delegationId,jobid.getJobId())
	def testgetProxyReqGrst(self):
		title("testgetProxyReqGrst")
		assert self.wmproxy.getProxyReq(delegationId,self.wmproxy.getGrstNs())
	def testputProxyGrst(self):
		title("testputProxyGrst")
		assert self.wmproxy.putProxy(delegationId,jobid.getJobId(),self.wmproxy.getGrstNs())
	def testDelegatedProxyInfo(self):
		pi= self.wmproxy.getDelegatedProxyInfo(delegationId)
		title("testDelegatedProxyInfo:", pi)
		return pi
	def testJobProxyInfo(self):
		pi=self.wmproxy.getJobProxyInfo(jobid.getJobId())
		title("testJobProxyInfo:", pi)
		return pi
	def testGetJDL(self):
		for  jdlType in [0,1]:
			pi=self.wmproxy.getJDL(jobid.getJobId(),jdlType)
			title("getJDL:", pi)
		return pi
	"""
	Other
	"""
	def testaddACLItems(self):
		title("testaddACLItems")
		items=["un", "due", "tre", "prova"]
		return self.wmproxy.addACLItems(jobid.getJobId(), items)


def addSuites(suiteTitle,suites, level, sublevel):
	mainSuite = unittest.TestSuite()
	help=""
	for i in range (len(suites)):
		if sublevel==LEV_DEFAULT:
			# Add test
			mainSuite.addTest(WmpTest(suites[i]))
		elif sublevel==i:
			# Add Required test
			mainSuite.addTest(WmpTest(suites[i]))
		elif sublevel==LEV_HELP:
			# generate help
			if i%2==0 and i!=0:
				help+="\n"
			tmpH= "." +str(i) +"  "+ suites[i]
			help+=tmpH.ljust(45)
	if help:
		print "\n**** " + str(level	)+ " "+ suiteTitle +" subtests: ***\n" +help
	return mainSuite


def runTextRunner(level, sublevel):
	"""
	Authomatically generate suites
	if level are set to LEV_HELP it only generates HELP
	"""
	allSuites={ \
	# SUBMIT SUITES
	"submitSuite":["testdagSubmit","testcollectionSubmit","testcollectionSubmitOne",\
	"testjobSubmit","testjobListMatch","testcycleJob"],\
	# PERUSAL SUITES
	"PerusalSuite":["testgetPerusalFiles","testenableFilePerusal"],\
	# TEMPLATES SUITES
	"templateSuite":["testgetStringParametricJobTemplate","testgetIntParametricJobTemplate",\
	"testgetCollectionTemplate","testgetDAGTemplate","testgetJobTemplate"],\
	# GETURI SUITES
	"getURISuite":["testgetSandboxDestURI","testgetSandboxBulkDestURI","testgetTransferProtocols","testgetOutputFileList"],\
	# PROXY SUITES
	"proxySuite":["testgetProxyReq","testputProxy","testgetProxyReqGrst","testputProxyGrst",\
	"testDelegatedProxyInfo","testGetJDL"],\
	}  #END SUITES

	LEV_MAX=len(allSuites.keys())
	allParsedSuites=[]
	sIndex=0
	# Generate Suites
	for suiteKey in allSuites.keys():
		allParsedSuites.append(addSuites(suiteKey,allSuites[suiteKey], sIndex, sublevel))
		sIndex+=1
	runner = unittest.TextTestRunner()
	# Execute Tests
	if level==LEV_MAX:
		# EXECUTE ALL SUITES
		for suite in allParsedSuites:
			runner.run (suite)
	elif level<0:
		# DO NOTHING: not allowed value
		pass
	elif level < LEV_MAX:
		# EXECUTE Selected SUITES
		runner.run (allParsedSuites[level])
	else:
		# DO NOTHING: not allowed value
		title("Warning!! Test number " + str(level) +"DOES NOT EXIST!")
		pass



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
			CUSTOM (alternative) MAIN
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


def printHelp(command, helpLevel=0):
	print "\nUsage:  ".ljust(25) + command + " 0-"+str(LEV_MAX-1)+"[.<subtest number>]  [<jobid>] [<jdl>]"
	print "ALL tests:".ljust(25) + command + " -a [<jobid>] [<jdl>]"
	print "help:     ".ljust(25) + command + " -h"

if __name__=="__main__":
	try:
		if len(sys.argv)<2:
			printHelp(sys.argv[0])
			sys.argv.append(raw_input("Please Select one test(or type '-h', or press ^C):\n"))
		if sys.argv[1]=="-h":
			runTextRunner(LEV_HELP,LEV_HELP)
			printHelp(sys.argv[0])
			print " - - - "
			print "EXAMPLES:"
			print "\t"+ sys.argv[0] + " 1".ljust(25) +"Will perform all templateSuite TESTS"
			print "\t"+ sys.argv[0] + " 2.4".ljust(25) +"Will perform testenableFilePerusal from perusalSuite SUITE"
			sys.argv[1]=(raw_input("Please Select one test(or press ^C):\n"))
	except KeyboardInterrupt:
		print "\nbye!"
		sys.exit(0)
	level = sys.argv[1]
	if level=="-a":
		if questionYN("Are you sure you wish to perform ALL tests?"):
			level=LEV_MAX
		else:
			printHelp(sys.argv[0])
			sys.exit(0)
	sublevel= LEV_DEFAULT
	if len(sys.argv)>2:
		jobid.setJobId(sys.argv[2])
		dagad.setJobId(sys.argv[2])
	if len(sys.argv)>3:
		jdl.loadJdl(sys.argv[3])
	try:
		level= int(level)
		if level>LEV_MAX:
			print "No such Example (not yet!)"
			raise 5
	except:
		try:
			level, sublevel = level.split(".")
			level = int(level)
			sublevel= int (sublevel)
		except:
			printHelp(sys.argv[0])
			sys.exit(0)
	print "#############################################"
	print "Using WMPROXY Service: " , url
	print "Using DELEGATION Id:   " , delegationId
	print "Example selected:" , level
	print "#############################################"
	runTextRunner(level, sublevel)
	print " END TEST \n"

