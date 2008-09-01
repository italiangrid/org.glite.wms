
import sys, os
import re, string
import tempfile
import popen2
import log4py
import getpass

from threading import Condition, Thread
from testsuite_utils import getHostname, getCACertDir, getUserKeyAndCert
from testsuite_utils import getProxyFile, cmdTable

jobUtilsLogger = log4py.Logger().get_instance(classid="job_utils")
subscriptionRE = re.compile("SubscriptionID=\[([^\]]+)")

def eraseJobs(jobList, cmd='purge',  proxyMan=None, logger=jobUtilsLogger):
    
    if len(jobList)>0:
        tempFD, tempFilename = tempfile.mkstemp(dir='/tmp')
        try:
            tFile = open(tempFilename, 'w+b')
            tFile.write("##CREAMJOBS##\n")
            for job in jobList:
                tFile.write(job + "\n")
            tFile.close()
      
            eraseCmdLine = cmdTable[cmd] + " -N -i " + tempFilename
            logger.debug("Command line: " + eraseCmdLine)
            
            if proxyMan<>None:
                proxyMan.beginLock()
            try:
                eraseProc = popen2.Popen4(eraseCmdLine)
    
                for line in eraseProc.fromchild:
                    if 'ERROR' in line or 'FATAL' in line:
                        logger.error(line[24:])
                eraseProc.fromchild.close()
            finally:
                if proxyMan<>None:
                    proxyMan.endLock()
                    
            os.remove(tempFilename)
                
        except:
            if 'tFile' in dir() and not tFile.closed:
                logger.error('Cannot write job file list')
            else:
               logger.error("Cannot purge jobs in " + tempFilename + ": " + str(sys.exc_info()[0]))
               
def subscribeToCREAMJobs(cemonURL, parameters, proxyFile, logger=jobUtilsLogger):
    
    consumerURL = 'http://%s:%d' % (getHostname(), parameters.consumerPort)
    subscrCmd = "%s %s %s %s %s %s %s %d %d" % \
            (cmdTable['subscribe'], proxyFile, getCACertDir(), \
             cemonURL, consumerURL, \
             'CREAM_JOBS',  'CLASSAD', parameters.rate,  31536000)
    logger.debug("Subscription command: " + subscrCmd)
    
    subscriptionId = None
    errBuffer = ''
    subscrProc = popen2.Popen4(subscrCmd)
    for line in subscrProc.fromchild:

        tmpm = subscriptionRE.search(line)
        if tmpm<>None:
            subscriptionId = tmpm.group(1)
            logger.debug("Registered subscription " + subscriptionId)
        else:
            errBuffer += line
    subscrProc.fromchild.close()

    if subscriptionId==None:
        logger.error(errBuffer)
        raise Exception('Cannot subscribe to' + cemonURL)
    
    return subscriptionId

def unSubscribeToCREAMJobs(cemonURL, subscrID, parameters, \
                         proxyFile, logger=jobUtilsLogger):
    unSubscrCmd = "%s %s %s %s %s" % \
            (cmdTable['unsubscribe'], proxyFile, getCACertDir(), \
             cemonURL, subscrID)
    logger.debug("UnSubscription command: " + unSubscrCmd)
    unSubscrProc = popen2.Popen4(unSubscrCmd)
    
    
class BooleanTimestamp:
    
    def __init__(self, timestamp, scheduled=False):
        self.timestamp = timestamp
        self.scheduled = scheduled
    
    def __float__(self):
        return self.timestamp
    
    def __lt__(self, nObj):
        return self.timestamp < float(nObj)
    
    def __le__(self, nObj):
        return self.timestamp <= float(nObj)
    
    def __eq__(self, nObj):
        return self.timestamp == float(nObj)
    
    def __ne__(self, nObj):
        return self.timestamp <> float(nObj)
    
    def __gt__(self, nObj):
        return self.timestamp > float(nObj)
    
    def __ge__(self, nObj):
        return self.timestamp >= float(nObj)


class AbstractRenewer(Thread):

    def __init__(self, parameters, container, logger=jobUtilsLogger):
        Thread.__init__(self)
        self.parameters = parameters
        self.container = container
        self.running = True
        self.cond = Condition()
        self.logger = logger
        
    def preRun(self):
        pass
        
    def doCycle(self, cmdPrefix, xTime, xID, fString):
        self.cond.acquire()
        try:
            while self.running:
                self.cond.wait(int(xTime) * 3 / 4)
                
                if self.running:
                    
                    self.preRun()
                    
                    if xID=='':
                        tsList = self.container.valueSnapshot()
                    else:
                        tsList = [ None ]
                        
                    for item in tsList:
                        if item==None:
                            rcmd = cmdPrefix % xID
                        else:
                            rcmd = cmdPrefix % (fString % (os.getpid(), float(item)))
                        self.logger.debug("Renew command: " + rcmd)
                        
                        if hasattr(self.container, 'proxyMan') and self.container.proxyMan<>None:
                            self.container.proxyMan.beginLock()
                            
                        renewProc = popen2.Popen4(rcmd)
                        for line in renewProc.fromchild:
                            if 'ERROR' in line or 'FATAL' in line:
                                self.logger.error("Cannot renew " + xID)
                        renewProc.fromchild.close()
                        
                        if hasattr(self.container, 'proxyMan') and self.container.proxyMan<>None:
                            self.container.proxyMan.endLock()

        finally:
            self.cond.release()
                    
    def halt(self):
        self.cond.acquire()
        self.running=False
        self.cond.notify()
        self.cond.release()


class LeaseRenewer(AbstractRenewer):
    
    def __init__(self, parameters, container, logger=jobUtilsLogger):
        AbstractRenewer.__init__(self, parameters, container, logger)
        
    def run(self):
        
        cmdPrefix = '%s -e %s -T %d %s ' % (cmdTable['lease'], \
                        self.parameters.resourceURI[:string.find(self.parameters.resourceURI,'/')], \
                        self.parameters.leaseTime, '%s')
        
        self.doCycle(cmdPrefix, self.parameters.leaseTime, \
                     self.parameters.leaseID, 'LEASEID%d.%f')

class ProxyRenewer(AbstractRenewer):

    def __init__(self,parameters, container, logger=jobUtilsLogger):
        AbstractRenewer.__init__(self, parameters, container, logger)
        
    def preRun(self):
        if hasattr(self.container, 'proxyMan') and self.container.proxyMan<>None:
            self.container.proxyMan.renewProxy()
        
    def run(self):
        
        cmdPrefix = '%s -e %s %s ' % (cmdTable['proxy-renew'], \
                        self.parameters.resourceURI[:string.find(self.parameters.resourceURI,'/')], \
                        '%s')
        
        self.doCycle(cmdPrefix, self.elaps, self.parameters.delegationID, 'DELEGID%d.%f')


class VOMSProxyManager(Thread):
    
    def __init__(self,parameters, logger=jobUtilsLogger):
        Thread.__init__(self)
        
        if not hasattr(parameters, 'vo') or parameters.vo=='':
            raise Exception, "Missing vo parameter"

        self.cert, self.key = getUserKeyAndCert()
        #TODO manage key without passphrase
        self.password = getpass.getpass()
        
        self.proxyFile = '/tmp/x509up_u%d_%d' % (os.getuid(), os.getpid())
            
        if hasattr(parameters, 'valid') and parameters.valid<>'':
            tokens = string.split(parameters.valid, ':')
            self.elaps = int(tokens[0])*3600 + int(tokens[1])*60
        else:
            self.elaps = 600

        self.parameters = parameters
        self.running = True
        self.cond = Condition()
        self.logger = logger
        self.pCond = Condition()
        self.wCheck = False
        self.rCheck = 0
        
        self.renewProxy()
        os.putenv('X509_USER_PROXY', self.proxyFile)
        
    def renewProxy(self):
        
        result = 0
        if hasattr(self.parameters, 'vo') and self.parameters.vo<>'':
            proxyCmd = '%s -voms %s -valid %s -out %s -pwstdin' \
                    % (cmdTable['proxy-init'], self.parameters.vo, \
                       self.parameters.valid, self.proxyFile)
            
            result += 1
            self.logger.debug('Create proxy command: ' + proxyCmd)
            
            try:
                self.pCond.acquire()
                self.wCheck = True
                if self.rCheck>0:
                    self.pCond.wait()
                self.pCond.release()
                
                proxyProc =popen2.Popen4(proxyCmd)
                proxyProc.tochild.write(self.password)
                proxyProc.tochild.close()
                for line in proxyProc.fromchild:
                    if 'proxy is valid' in line:
                        self.logger.debug(line)
                proxyProc.fromchild.close()
                
                result = proxyProc.wait()
            finally:
                self.pCond.acquire()
                self.wCheck = False
                self.pCond.notifyAll()
                self.pCond.release()

        return result
    
    def beginLock(self):
        self.pCond.acquire()
        self.rCheck += 1
        while self.wCheck:
            self.pCond.wait()
        self.pCond.release()
        
    def endLock(self):
        self.pCond.acquire()
        self.rCheck -= 1
        self.pCond.notifyAll()
        self.pCond.release()                
        
    def run(self):

        self.cond.acquire()
        try:
            while self.running:
                self.cond.wait(int(self.elaps) * 3 / 4)
                if self.running:
                    self.renewProxy()
        finally:
            self.cond.release()

    def halt(self):
        self.cond.acquire()
        self.running=False
        self.cond.notify()
        self.cond.release()
    
    
class JobProcessed:
    
    def __init__(self):
        self.jobTable = {}
        
    def append(self, jobId, jobStatus=None, failureReason=''):
        if jobStatus==None:
            self.jobTable[jobId] = True
        else:
            self.jobTable[jobId] = self.canPurge(jobStatus, failureReason)
            
    def canPurge(self, jobStatus, failureReason=''):
        return True
    
    def dontPurge(self, jobId):
        self.jobTable[jobId] = False
    
    def __len__(self):
        return len(self.jobTable)
    
    def getPurgeableJobs(self):
        return [ x for x in self.jobTable if self.jobTable[x] ]
    
    def clear(self):
        self.jobTable.clear()
            
