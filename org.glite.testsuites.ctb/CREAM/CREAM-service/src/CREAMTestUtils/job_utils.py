
import sys, os
import re, string
import tempfile
import popen2
import getpass

from threading import Condition, Thread
from testsuite_utils import hostname, getCACertDir, getUserKeyAndCert
from testsuite_utils import getProxyFile, cmdTable
from testsuite_utils import applicationTS, applicationID, mainLogger

subscriptionRE = re.compile("SubscriptionID=\[([^\]]+)")

def eraseJobs(jobList, cmd='purge',  proxyMan=None):
    
    if len(jobList)>0:
        tempFD, tempFilename = tempfile.mkstemp(dir='/tmp')
        try:
            tFile = os.fdopen(tempFD, 'w+b')
            tFile.write("##CREAMJOBS##\n")
            for job in jobList:
                tFile.write(job + "\n")
            tFile.close()
      
            eraseCmdLine = cmdTable[cmd] + " -N -i " + tempFilename
            mainLogger.debug("Command line: " + eraseCmdLine)
            
            if proxyMan<>None:
                proxyMan.beginLock()
            try:
                eraseProc = popen2.Popen4(eraseCmdLine)
    
                for line in eraseProc.fromchild:
                    if 'ERROR' in line or 'FATAL' in line:
                        mainLogger.error(line[24:])
                eraseProc.fromchild.close()
            finally:
                if proxyMan<>None:
                    proxyMan.endLock()
                    
            os.remove(tempFilename)
                
        except:
            if 'tFile' in dir() and not tFile.closed:
                mainLogger.error('Cannot write job file list')
            else:
               mainLogger.error("Cannot purge jobs in " + tempFilename + ": " + str(sys.exc_info()[0]))
               
def subscribeToCREAMJobs(cemonURL, parameters, proxyFile, useSSL=False):
    
    if useSSL:
        consumerURL = 'https://%s:%d' % (hostname, parameters.consumerPort)
    else:
        consumerURL = 'http://%s:%d' % (hostname, parameters.consumerPort)
    subscrCmd = "%s %s %s %s %s %s %s %d %d" % \
            (cmdTable['subscribe'], proxyFile, getCACertDir(), \
             cemonURL, consumerURL, \
             'CREAM_JOBS',  'CLASSAD', parameters.rate,  31536000)
    mainLogger.debug("Subscription command: " + subscrCmd)
    
    subscriptionId = None
    errBuffer = ''
    subscrProc = popen2.Popen4(subscrCmd)
    for line in subscrProc.fromchild:

        tmpm = subscriptionRE.search(line)
        if tmpm<>None:
            subscriptionId = tmpm.group(1)
            mainLogger.debug("Registered subscription " + subscriptionId)
        else:
            errBuffer += line
    subscrProc.fromchild.close()

    if subscriptionId==None:
        mainLogger.error(errBuffer)
        raise Exception('Cannot subscribe to ' + cemonURL)
    
    return subscriptionId

def unSubscribeToCREAMJobs(cemonURL, subscrID, parameters, proxyFile):
    unSubscrCmd = "%s %s %s %s %s" % \
            (cmdTable['unsubscribe'], proxyFile, getCACertDir(), \
             cemonURL, subscrID)
    mainLogger.debug("UnSubscription command: " + unSubscrCmd)
    unSubscrProc = popen2.Popen4(unSubscrCmd)
    for line in unSubscrProc.fromchild:
        mainLogger.debug(line)
    unSubscrProc.fromchild.close()
    
    
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
    
    logger = None

    def __init__(self, container):
        Thread.__init__(self)
        self.container = container
        self.running = True
        self.cond = Condition()
        if AbstractRenewer.logger==None:
            AbstractRenewer.logger = mainLogger.get_instance(classid='AbstractRenewer')
        
    def preRun(self):
        pass
        
    def run(self):
        self.cond.acquire()
        try:
            while self.running:
                self.cond.wait(int(self.sleepTime) * 3 / 4)
                
                if self.running:
                    
                    self.preRun()
                    
                    if self.single:
                        tsList = [applicationTS]
                    else:
                        tsList = self.container.valueSnapshot()
                        
                    for item in tsList:
                        rID = self.fString % (os.getpid(), float(item))
                        rcmd = self.cmdPrefix % rID
                        AbstractRenewer.logger.debug("Renew command: " + rcmd)
                        
                        if hasattr(self.container, 'proxyMan') and self.container.proxyMan<>None:
                            self.container.proxyMan.beginLock()
                            
                        renewProc = popen2.Popen4(rcmd)
                        for line in renewProc.fromchild:
                            if 'ERROR' in line or 'FATAL' in line:
                                AbstractRenewer.logger.error("Cannot renew " + rID)
                            else:
                                AbstractRenewer.logger.debug(line)
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
    
    logger = None
    
    def __init__(self, parameters, container):
        AbstractRenewer.__init__(self, container)

        endPoint = parameters.resourceURI[:string.find(parameters.resourceURI,'/')] 
        self.cmdPrefix = '%s -e %s -T %d %s ' % (cmdTable['lease'], endPoint, \
                                                                        parameters.leaseTime, '%s')
        self.sleepTime = parameters.leaseTime
        self.fString = 'LEASEID%d.%f'
        if parameters.leaseType=='none':
            raise Exception, 'Cannot start lease renewer: lease is not enabled'
        self.single = ( parameters.leaseType=='single' )
        
        if LeaseRenewer.logger==None:
            LeaseRenewer.logger = mainLogger.get_instance(classid='LeaseRenewer')
        
        if parameters.leaseType=='single':
            cannotLease = False
            leaseCmd = '%s -e %s  -T %d LEASEID%s' \
                    % (cmdTable['lease'], endPoint, parameters.leaseTime, applicationID) 
            LeaseRenewer.logger.debug("Lease command: " + leaseCmd)
            leaseProc = popen2.Popen4(leaseCmd)
            for line in leaseProc.fromchild:
                if 'ERROR' in line or 'FATAL' in line:
                    cannotLease = True
            leaseProc.fromchild.close()
            
            if cannotLease:
                raise Exception, "Cannot register LEASEID" + applicationID
        
class ProxyRenewer(AbstractRenewer):
    
    logger = None

    def __init__(self,parameters, container, proxyMan):
        AbstractRenewer.__init__(self, container)
        self.proxyMan = proxyMan
        
        endPoint = parameters.resourceURI[:string.find(parameters.resourceURI,'/')] 
        self.cmdPrefix = '%s -e %s %s ' % (cmdTable['proxy-renew'], endPoint, '%s')
        #the timesleep is calculated dynamically
        self.sleepTime = self.proxyMan
        self.fString = 'DELEGID%d.%f'
        self.single = ( parameters.delegationType=='single' )
        
        if ProxyRenewer.logger == None:
            ProxyRenewer.logger = mainLogger.get_instance(classid='ProxyRenewer')
        
        if parameters.delegationType=='single':
            cannotDelegate = False
            delegCmd = '%s -e %s DELEGID%s' \
                                % (cmdTable['delegate'], endPoint, applicationID) 
            ProxyRenewer.logger.debug("Delegate command: " + delegCmd)
            delegProc = popen2.Popen4(delegCmd)
            for line in delegProc.fromchild:
                if 'ERROR' in line or 'FATAL' in line:
                    cannotDelegate = True
            delegProc.fromchild.close()
            
            if cannotDelegate:
                raise Exception, "Cannot delegate proxy DELEGID" + applicationID
        
    def preRun(self):
        self.proxyMan.renewProxy()
        
class VOMSProxyManager(Thread):
    
    logger = None
    
    def __init__(self,parameters):
        Thread.__init__(self)

        if VOMSProxyManager.logger==None:
            VOMSProxyManager.logger = mainLogger.get_instance(classid='VOMSProxyManager')
#        if not hasattr(parameters, 'vo') or parameters.vo=='':
#            raise Exception, "Missing vo parameter"

        self.cert, self.key = getUserKeyAndCert()
        if self.cert==None:
            VOMSProxyManager.logger.debug("Using external proxy certificate")
            self.usingProxy = True
            self.proxyFile = getProxyFile()
            return
        
        VOMSProxyManager.logger.debug("Enabled voms proxy management")
        self.usingProxy = False
        #TODO manage key without passphrase
        self.password = getpass.getpass('Password for user key: ')
        
        self.proxyFile = '/tmp/x509up_u%d_%d' % (os.getuid(), os.getpid())
            
        if hasattr(parameters, 'valid') and parameters.valid<>'':
            tokens = string.split(parameters.valid, ':')
            self.interval = int(tokens[0])*3600 + int(tokens[1])*60
        else:
            self.interval = 600

        self.parameters = parameters
        self.running = True
        self.cond = Condition()
        self.pCond = Condition()
        self.wCheck = False
        self.rCheck = 0
        
        self.renewProxy()
        os.environ['X509_USER_PROXY'] = self.proxyFile
        
    def renewProxy(self):
        
        if self.usingProxy:
            VOMSProxyManager.logger.debug("Cannot renew external proxy")
            return 0
        
        result = 0
        if hasattr(self.parameters, 'vo') and self.parameters.vo<>'':
            proxyCmd = '%s -voms %s -valid %s -out %s -pwstdin' \
                    % (cmdTable['proxy-init'], self.parameters.vo, \
                       self.parameters.valid, self.proxyFile)
            
            result += 1
            VOMSProxyManager.logger.debug('Create proxy command: ' + proxyCmd)
            
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
                        VOMSProxyManager.logger.debug(line)
                proxyProc.fromchild.close()
                
                result = proxyProc.wait()
            finally:
                self.pCond.acquire()
                self.wCheck = False
                self.pCond.notifyAll()
                self.pCond.release()

        return result
    
    def beginLock(self):
        
        if self.usingProxy:
            return
        
        self.pCond.acquire()
        self.rCheck += 1
        while self.wCheck:
            self.pCond.wait()
        self.pCond.release()
        
    def endLock(self):
        
        if self.usingProxy:
            return
        
        self.pCond.acquire()
        self.rCheck -= 1
        self.pCond.notifyAll()
        self.pCond.release()                
        
    def run(self):
        
        if self.usingProxy:
            VOMSProxyManager.logger.debug("Voms proxy management is disabled")
            return

        self.cond.acquire()
        try:
            while self.running:
                self.cond.wait(int(self.interval) * 3 / 4)
                if self.running:
                    self.renewProxy()
        finally:
            self.cond.release()

    def halt(self):
        
        if self.usingProxy:
            return
        
        self.cond.acquire()
        self.running=False
        self.cond.notify()
        self.cond.release()
        
    def __int__(self):
        
        if not self.usingProxy:
            self.cond.acquire()
            
        try:
            infoCmd = "%s -timeleft -file %s" % \
                (cmdTable['proxy-info'], self.proxyFile)
            infoProc = popen2.Popen4(infoCmd)
            for line in infoProc.fromchild:
                result = int(string.strip(line))
            infoProc.fromchild.close()
            
            if infoProc.wait()<>0:
                raise Exception, "Cannot calculate proxy timeleft"        
        finally:
            if not self.usingProxy:
                self.cond.release()
            
        return result
    
    
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
            
