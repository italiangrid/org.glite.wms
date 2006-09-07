import unittest
import sys



# LEVEL/SUBLEVEL MENU
LEV_DEFAULT=-2
LEV_HELP=-1
LEV_ALLTESTS=-3

class Config:
	EXTRA_PARAM =""
	DEBUGMODE = 1
	allSuites={ \
	"ExampleSuite": ["Please_Add_Suite"  ],\
	"ExampleSuiteTwo"  : ["Please_Add_Suite","Please_Add_Suite"] \
	}  #END SUITES
	LEV_MAX=len(allSuites.keys())


def printHelp(command, helpLevel=0):
	print "\nUsage:  ".ljust(25) + command + " 0-"+str(Config.LEV_MAX-1)+"[.<subtest number>] " + Config.EXTRA_PARAM
	print "ALL tests:".ljust(25) + command + " -a"
	print "help:     ".ljust(25) + command + " -h"


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

def addSuites(SuiteTest,suiteTitle,suites, level, sublevel):
	mainSuite = unittest.TestSuite()
	help=""
	for i in range (len(suites)):
		if sublevel==LEV_DEFAULT:
			# Add test
			mainSuite.addTest(SuiteTest(suites[i]))
		elif sublevel==i:
			# Add Required test
			mainSuite.addTest(SuiteTest(suites[i]))
		elif sublevel==LEV_HELP:
			# generate help
			if i%2==0 and i!=0:
				help+="\n"
			tmpH= "   -> "+str(level)+"." +str(i) +"  "+ suites[i]
			help+=tmpH.ljust(45)
	if help:
		print "\n****************************************************"
		print "SUITE "+str(level)+": " + " "+ suiteTitle +"\n   Subtests:\n" +help
		print "\n****************************************************"
	return mainSuite


def runTextRunner(SuiteTest,level, sublevel):
	"""
	Authomatically generate suites
	if level are set to LEV_HELP it only generates HELP
	"""
	allParsedSuites=[]
	sIndex=0
	# Generate Suites
	for suiteKey in Config.allSuites.keys():
		allParsedSuites.append(addSuites(SuiteTest,suiteKey,Config.allSuites[suiteKey], sIndex, sublevel))
		sIndex+=1
	runner = unittest.TextTestRunner()
	# Execute Tests
	if level==LEV_ALLTESTS:
		# EXECUTE ALL SUITES
		for suite in allParsedSuites:
			if Config.DEBUGMODE:
				print "  --> Running suite(s):\n",suite
			runner.run (suite)
	elif level<0:
		# DO NOTHING, private levels
		pass
	elif level < Config.LEV_MAX:
		# EXECUTE Selected SUITES
		if Config.DEBUGMODE:
			print "  --> Running suite(s):\n",allParsedSuites[level]
		runner.run (allParsedSuites[level])
	else:
		# DO NOTHING: not allowed value
		title("Warning!! Test number " + str(level) +"DOES NOT EXIST!")
		pass

def run_unit(SuiteTest):
	Config.LEV_MAX=len(Config.allSuites.keys())
	try:
		if len(sys.argv)<2:
			printHelp(sys.argv[0])
			sys.argv.append(raw_input("Please Select one test(or type '-h', or press ^C):\n"))
		if sys.argv[1]=="-h":
			runTextRunner(SuiteTest,LEV_HELP,LEV_HELP)
			printHelp(sys.argv[0])
			print " - - - "
			print "EXAMPLES:"
			print "\t"+ sys.argv[0] + " 1".ljust(25) +"Will perform all TESTS from all SUITES"
			print "\t"+ sys.argv[0] + " 3".ljust(25) +"Will perform all TESTS from SUITE number 3"
			print "\t"+ sys.argv[0] + " 2.4".ljust(25) +"Will perform TEST number 4 from SUITE number 2"
			sys.argv[1]=(raw_input("Please Select one test(or press ^C):\n"))
	except KeyboardInterrupt:
		print "\nbye!"
		sys.exit(0)
	level = sys.argv[1]
	if level=="-a":
		if questionYN("Are you sure you wish to perform ALL tests?"):
			level=LEV_ALLTESTS
		else:
			printHelp(sys.argv[0])
			sys.exit(0)
	sublevel= LEV_DEFAULT
	try:
		level= int(level)
		if level>=Config.LEV_MAX:
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
	runTextRunner(SuiteTest,level, sublevel)
	print " END TEST \n"
