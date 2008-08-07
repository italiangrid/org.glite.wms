
import sys, os
import re, string
import tempfile
import popen2
import log4py
import getpass

from threading import Condition, Thread
from testsuite_utils import getHostname, getCACertDir, getUserKeyAndCert
from testsuite_utils import getProxyFile

jobUtilsLogger = log4py.Logger().get_instance(classid="job_utils")
subscriptionRE = re.compile("SubscriptionID=\[([^\]]+)")

def eraseJobs(jobList, purgeCmd, logger=jobUtilsLogger):
    
    if len(jobList)>0:
        tempFD, tempFilename = tempfile.mkstemp(dir='/tmp')
        try:
            tFile = open(tempFilename, 'w+b')
            tFile.write("##CREAMJOBS##\n")
            for job in jobList:
                tFile.write(job + "\n")
            tFile.close()
      
            purgeCmdLine = purgeCmd + " -N -i " + tempFilename
            logger.debug("Command line: " + purgeCmdLine)
            purgeProc = popen2.Popen4(purgeCmdLine)

            for line in purgeProc.fromchild:
                if 'ERROR' in line or 'FATAL' in line:
                    logger.error(line[24:])
            purgeProc.fromchild.close()
                    
            os.remove(tempFilename)
                
        except:
            if 'tFile' in dir() and not tFile.closed:
                logger.error('Cannot write job file list')
            else:
               logger.error("Cannot purge jobs in " + tempFilename + ": " + str(sys.exc_info()[0]))
               
def subscribeToCREAMJobs(subscribeCmd, cemonURL, parameters, \
                         proxyFile, logger=jobUtilsLogger):
    
    consumerURL = 'http://%s:%d' % (getHostname(), parameters.consumerPort)
    subscrCmd = "%s %s %s %s %s %s %s %d %d" % \
            (subscribeCmd, proxyFile, getCACertDir(), \
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

def unSubscribeToCREAMJobs(unSubscribeCmd, cemonURL, subscrID, parameters, \
                         proxyFile, logger=jobUtilsLogger):
    unSubscrCmd = "%s %s %s %s %s" % \
            (unSubscribeCmd, proxyFile, getCACertDir(), \
             cemonURL, subscrID)
    logger.debug("UnSubscription command: " + unSubscrCmd)
    unSubscrProc = os.system(unSubscrCmd)
    
    
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



class ProxyValidity:
    
    def __init__(self, cmds, proxyFile):
        self.cmdTable = cmds
        self.proxyFile = proxyFile
        
    def __int__(self):
        infoCmd = "%s -timeleft -file %s" % \
            (self.cmdTable['proxy-info'], self.proxyFile)
        infoProc = popen2.Popen4(infoCmd)
        #TODO missing error handling
        for line in infoProc.fromchild:
            result = int(string.strip(line))
        infoProc.fromchild.close()
        
        return result

class AbstractRenewer(Thread):

    def __init__(self, parameters, cmds, container, logger=jobUtilsLogger):
        Thread.__init__(self)
        self.parameters = parameters
        self.cmdTable = cmds
        self.container = container
        self.running = True
        self.cond = Condition()
        self.logger = logger
        
    def setup(self):
        pass
    
    def doCycle(self, cmdPrefix, xTime, xID, fString):
        self.cond.acquire()
        try:
            while self.running:
                self.cond.wait(int(xTime) * 3 / 4)
                
                self.setup()
                
                if self.running:
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
                        
                        renewProc = popen2.Popen4(rcmd)
                        for line in renewProc.fromchild:
                            if 'ERROR' in line or 'FATAL' in line:
                                self.logger.error("Cannot renew " + xID)
                        renewProc.fromchild.close()
        finally:
            self.cond.release()
                    
    def halt(self):
        self.cond.acquire()
        self.running=False
        self.cond.notify()
        self.cond.release()


class LeaseRenewer(AbstractRenewer):
    
    def __init__(self, parameters, cmds, container, logger=jobUtilsLogger):
        AbstractRenewer.__init__(self, parameters, cmds, container, logger)
        
    def run(self):
        
        cmdPrefix = '%s -e %s -T %d %s ' % (self.cmdTable['lease'], \
                        self.parameters.resourceURI[:string.find(self.parameters.resourceURI,'/')], \
                        self.parameters.leaseTime, '%s')
        
        self.doCycle(cmdPrefix, self.parameters.leaseTime, \
                     self.parameters.leaseID, 'LEASEID%d.%f')
    

class ProxyRenewer(AbstractRenewer):

    def __init__(self,parameters, cmds, container, logger=jobUtilsLogger):
        AbstractRenewer.__init__(self, parameters, cmds, container, logger)

        if hasattr(parameters, 'vo') and parameters.vo<>'':
            self.cert, self.key = getUserKeyAndCert()
            #TODO manage key without passphrase
            self.password = getpass.getpass()
            
            self.proxyFile = '/tmp/x509up_u%d_%d' % (os.getuid(), os.getpid())
            os.putenv('X509_USER_PROXY', self.proxyFile)
        else:
            self.proxyFile = getProxyFile()
        
    def setup(self):
        
        if hasattr(self.parameters, 'vo') and self.parameters.vo<>'':
            proxyCmd = '%s -voms %s -valid %s -out %s -pwstdin' \
                    % (self.cmdTable['proxy-init'], self.parameters.vo, \
                       self.parameters.valid, self.proxyFile)
            
            self.logger.debug('Create proxy command: ' + proxyCmd)
            proxyProc =popen2.Popen4(proxyCmd)
            proxyProc.tochild.write(self.password)
            proxyProc.tochild.close()
            for line in proxyProc.fromchild:
                if 'proxy is valid' in line:
                    self.logger.debug(line)
            proxyProc.fromchild.close()
            
            return proxyProc.wait()
        else:
            return 0
            
    def run(self):
        
        cmdPrefix = '%s -e %s %s ' % (self.cmdTable['proxy-renew'], \
                        self.parameters.resourceURI[:string.find(self.parameters.resourceURI,'/')], \
                        '%s')
        
        if hasattr(self.parameters, 'valid') and self.parameters.valid<>'':
            tokens = string.split(self.parameters.valid, ':')
            elaps = int(tokens[0])*3600 + int(tokens[1])*60
        else:
            elaps = ProxyValidity(self.cmdTable, self.proxyFile)
        
        self.doCycle(cmdPrefix, elaps, self.parameters.delegationID, 'DELEGID%d.%f')


class JobProcessed:
    
    def __init__(self, cmdTable):
        self.jobTable = {}
        self.purgeCmd = cmdTable['purge']
        
    def append(self, jobId, jobStatus=None):
        if jobStatus==None:
            self.jobTable[jobId] = True
        else:
            self.jobTable[jobId] = self.canPurge(jobStatus)
            
    def canPurge(self, jobStatus):
        return True
    
    def dontPurge(self, jobId):
        self.jobTable[jobId] = False
    
    def __len__(self):
        return len(self.jobTable)
    
    def clear(self):
        if len(self.jobTable)>0:
            jobList = [ x for x in self.jobTable if self.jobTable[x] ]
            eraseJobs(jobList, self.purgeCmd)
            self.jobTable.clear()
            


class DummyClass:
    def __init__(self):
        self.vo = 'dteam'
        self.valid = '00:10'
        self.delegationID = 'orso'
        self.resourceURI = 'cream-02.pd.infn.it:8443/cream-lsf-creamtest1'
        
    def __getitem__(self, key):
        if key=='proxy-init':
            return '/data/glite/precert/stage/bin/voms-proxy-init'
        if key=='proxy-info':
            return '/data/glite/precert/stage/bin/voms-proxy-info'
        if key=='proxy-renew':
            return '/data/glite/precert/stage/bin/glite-ce-proxy-renew'
        
if __name__ == "__main__":
    dummyObj = DummyClass()
    proxyRenewer = ProxyRenewer(dummyObj, dummyObj, None)
    proxyRenewer.run()