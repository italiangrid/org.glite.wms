

import sys, os, os.path, tempfile
import re, string
import popen2


def checkIsOk(value):
    return True

def checkRate(value):
    if value<5:
        raise Exception('Bad rate ' + repr(value))

def atLeastOne(value):
    if value<1:
        raise Exception('Bad number for job ' + repr(value))
    
def checkPort(value):
    if value<1025 or value>65535:
        raise Exception("Bad port number: " + repr(value))
    
class Parameters:
    def __init__(self):
        self.pTable = {}

    def register(self, name, type, default=None, check=None, optChar=None):
        
        if check==None:
            check = checkIsOk

        if optChar==None:
            optChar = name[0]
            
        self.pTable[name] = (type, check, optChar)
        if default==None and type=='s':
            default=''
        elif default==None and type=='d':
            default=0
        elif default==None and type=='b':
            default=False

        #check(default)

        setattr(self, name, default)

    def testAndSet(self, k, v):
        type, check, optChar = self.pTable[k]
        if type=='d':
            iValue = int(v)
            check(iValue)
            setattr(self, k, iValue)
        elif type=='b':
            if v=='':
                setattr(self, k, True)
            else:
                setattr(self, k, v.lower()=='true')
        else:
            check(v)
            setattr(self, k, v)        

    def parseOptList(self, optList):
        for k,v in optList:
            if k[0:2]=='--':
                optName = k[2:]
            elif k[0]=='-':
                for tmpo in self.pTable.keys():
                    type, check, optChar = self.pTable[tmpo]
                    if optChar==k[1]:
                        optName = tmpo
            self.testAndSet(optName, v)

    def parseConfigFile(self, confFileName):
        propRegExp = re.compile("^\s*(\w+)\s*=\s*(\w+)")
        try:
            confFile = open(confFileName)
            for line in confFile:
                prop = propRegExp.search(line)
                if prop <> None:
                    self.testAndSet(prop.group(1), prop.group(2))
            confFile.close()
        finally:
            if 'confFile' in dir() and not confFile.closed:
                confFile.close()
    
    def getLongOptList(self):
        result = []
        for k in self.pTable.keys():
            type, check, optChar = self.pTable[k]
            if type=='b':
                result.append(k)
            else:
                result.append(k + "=")
        return result

    def getShortOptString(self):
        result = ''
        for k in self.pTable.keys():
            type, check, optChar = self.pTable[k]
            if type=='b':
                result += optChar
            else:
                result += (optChar + ":")
        return result


def createTempJDL(sleepTime, logger=None):
    tempFD, tempFilename = tempfile.mkstemp(dir='/tmp')
    try:
        tFile = open(tempFilename, 'w+b')
        tFile.write('[executable="/bin/sleep";\n')
        tFile.write('arguments="' + str(sleepTime) +'";]\n')
        tFile.close()
        return tempFilename
    except :
        if logger<>None:
            logger.error("Cannot create temp jdl file:" + str(sys.exc_info()[0]))
    return None

def getCECommandTable():
    if os.environ.has_key("GLITE_LOCATION"):
        gliteLocation = os.environ["GLITE_LOCATION"]
    else:
        gliteLocation = "/opt/glite"
        
    gliteCeCommand = { "submit": gliteLocation + "/bin/glite-ce-job-submit",
                       "status": gliteLocation + "/bin/glite-ce-job-status",
                       "cancel": gliteLocation + "/bin/glite-ce-job-cancel",
                       "purge": gliteLocation + "/bin/glite-ce-job-purge",
                       "subscribe": gliteLocation + "/bin/CEMonitorSubscriber",
                       "unsubscribe": gliteLocation + "/bin/CEMonitorUnsubscriber",
                       "lease": gliteLocation + "/bin/glite-ce-job-lease",
                       "proxy-init": gliteLocation + "/bin/voms-proxy-init",
                       "proxy-info": gliteLocation + "/bin/voms-proxy-info",
                       "delegate": gliteLocation + "/bin/glite-ce-delegate-proxy",
                       "proxy-renew": gliteLocation + "/bin/glite-ce-proxy-renew"};

    for k in gliteCeCommand.keys():
        if not os.access(gliteCeCommand[k], os.X_OK):
            raise Exception, "Cannot find executable " + gliteCeCommand[k]
        
    return gliteCeCommand

def _getCredFile(envName, default):
    if os.environ.has_key(envName):
        credFile = os.environ[envName];
    else:
        credFile = default
        
    if not os.path.isfile(credFile):
        raise Exception, "Cannot find: " + credFile
    
    return credFile

def getProxyFile():
    return _getCredFile("X509_USER_PROXY", "/tmp/x509up_u" + str(os.getuid()))

def getUserKeyAndCert():
    if os.environ.has_key('HOME'):
        homeDir = os.environ['HOME'];
    else:
        homeDir = "."
    cert = _getCredFile("X509_USER_CERT", homeDir + "/.globus/usercert.pem")
    key = _getCredFile("X509_USER_KEY", homeDir + "/.globus/userkey.pem")
    return (cert, key)

def getHostname():
    proc = popen2.Popen4('/bin/hostname -f')
    hostname =  string.strip(proc.fromchild.readline())
    proc.fromchild.close()
    return hostname

def getCACertDir():
    if os.environ.has_key("CACERT_DIR"):
        caCertDir = os.environ["CACERT_DIR"];
    else:
        caCertDir = "/etc/grid-security/certificates"
        
    if not os.path.isdir(caCertDir):
        raise Exception, "Cannot find CA certificate directory " + caCertDir
    return caCertDir
        
class ManPage:
    
    def __init__(self):
        self.shortDescr = None
        self.description = None
        self.synopsis = None
        self.parameters = []
        self.env = []
        
    def display(self):
        
        if os.environ.has_key("HELP_FORMAT"):
            format = os.environ["HELP_FORMAT"]
        else:
            format = None
            
        executable = os.path.basename(sys.argv[0])
        
        if format=='GROFF':
            print ".TH " + executable + ' "1" ' + executable + ' "GLITE Testsuite"\n'
            print ".SH NAME:"
            print executable + ' \- ' + self.shortDescr + '\n'
            print ".SH SYNOPSIS"
            print '.B ' + executable
            print self.synopsis + '\n'
            print '.SH DESCRIPTION'
            print self.description + '\n'
            if len(self.parameters)>0:
                print '.SH OPTIONS'
                for item in self.parameters:
                    print '.HP\n\\fB%s\\fR, \\fB%s\\fR\n%s\n\n.IP\n%s\n.PP' % item
            if len(self.env)>0:
                print '.SH ENVIRONMENT'
                for item in self.env:
                    print '.TP\n.B %s\n%s\n.' % item

        else:
            print "NAME\n\t" + executable + " - " + self.shortDescr + "\n"
            print "SYNOPSIS\n\t" + executable + ' ' + self.synopsis + "\n"
            print "DESCRIPTION\n\t" + self.description + "\n"
            for item in self.parameters:
                print "\t%s %s %s\n\t\t%s\n" % item
            if len(self.env)>0:
                print "ENVIRONMENT\n"
                for item in self.env:
                    print "\t%s\n\t\t%s\n" % item



