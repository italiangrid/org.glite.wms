
import sys, os
import re, string
import tempfile
import popen2
import log4py

from testsuite_utils import getHostname, getCACertDir

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
    
    def __int__(self):
        return self.timestamp
    
    def __lt__(self, nObj):
        return self.timestamp < int(nObj)
    
    def __le__(self, nObj):
        return self.timestamp <= int(nObj)
    
    def __eq__(self, nObj):
        return self.timestamp == int(nObj)
    
    def __ne__(self, nObj):
        return self.timestamp <> int(nObj)
    
    def __gt__(self, nObj):
        return self.timestamp > int(nObj)
    
    def __ge__(self, nObj):
        return self.timestamp >= int(nObj)


