import re, string
import threading
import time
import popen2
import testsuite_utils, job_utils

from submit_pool import JobSubmitterPool

class JobPoller(threading.Thread):

    logger = None

    runningStates = ['IDLE', 'RUNNING', 'REALLY-RUNNING']
    finalStates = ['DONE-OK', 'DONE-FAILED', 'ABORTED', 'CANCELLED']
    
    def __init__(self, parameters, pManager=None):
        threading.Thread.__init__(self)
        self.table = {}
        self.lock = threading.Lock()
        self.parameters = parameters
        self.proxyMan = pManager
        
        self.finishedJobs = None
        
        self.jobRE = re.compile("JobID=\[([^\]]+)")
        self.statusRE = re.compile('Status\s*=\s*\[([^\]]+)')
        self.exitCodeRE = re.compile('ExitCode\s*=\s*\[([^\]]*)')
        self.failureRE = re.compile('FailureReason\s*=\s*\[([^\]]+)')
        
        self.pool = JobSubmitterPool(parameters, self, pManager)
        self.tableOfResults = {'DONE-OK': 0, 'DONE-FAILED': 0, \
                               'ABORTED': 0, 'CANCELLED': 0}
        
        if JobPoller.logger==None:
            JobPoller.logger = testsuite_utils.mainLogger.get_instance(classid="JobPoller")

    def manageRunningState(self, currId):
        pass
    
    def processFinishedJobs(self):
        if hasattr(self.finishedJobs, 'getPurgeableJobs'):
            jobList = self.finishedJobs.getPurgeableJobs()
        else:
            jobList = self.finishedJobs
        if len(jobList)>0:
            job_utils.eraseJobs(jobList)
        self.finishedJobs.clear()
    
    def processRunningJobs(self):
        pass
    
    def run(self):

        minTS = time.time()

        serviceHost = self.parameters.resourceURI[:string.find(self.parameters.resourceURI,'/')]
                
        jobLeft = self.parameters.numberOfJob
        jobProcessed = 0
        
        while jobProcessed<self.parameters.numberOfJob:
            
            ts = time.time()

            statusCmd = testsuite_utils.cmdTable['status'] \
                        + " -f \"" + time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(minTS)) \
                        + "\" -e " + serviceHost + " --all"
            JobPoller.logger.debug('Command line: ' + statusCmd)
            
            if self.proxyMan<>None:
                self.proxyMan.beginLock()
            statusProc = popen2.Popen4(statusCmd, True)
                            
            currId = None
            currStatus = None
            currReason = ''

            self.lock.acquire()

            try:
                
                try:
                    line = statusProc.fromchild.next()
                    while True:
                        if 'ERROR' in line or 'FATAL' in line:
                            JobPoller.logger.error(line[24:])
                            line = statusProc.fromchild.next()
                            continue
    
                        tmpm = self.jobRE.search(line)
                        if tmpm<>None:
                            currId = tmpm.group(1)
                            line = statusProc.fromchild.next()
    
                            tmpm = self.statusRE.search(line)
                            if tmpm==None:
                                JobPoller.logger.error('Missing status for ' + currId)
                                continue
                            
                            currStatus = tmpm.group(1)
                            try:
                                line = statusProc.fromchild.next()
                                tmpm = self.exitCodeRE.search(line)
                                if tmpm<>None:
                                    line = statusProc.fromchild.next()
                                
                                tmpm = self.failureRE.search(line)
                                if tmpm<>None:
                                    currReason = tmpm.group(1)
                                    line = statusProc.fromchild.next()
                            finally:
                                if currId in self.table:
                                    if currStatus in JobPoller.runningStates:
                                        self.manageRunningState(currId)
                                    elif currStatus in JobPoller.finalStates:
                                        del(self.table[currId])
                                        jobProcessed += 1
                                        self.tableOfResults[currStatus] += 1
                                        self.finishedJobs.append(currId, currStatus, currReason)
                                        JobPoller.logger.info(
                                              "Execution terminated for job: %s (%s, %s)"  
                                              % (currId, currStatus, currReason))
                                    else:
                                        JobPoller.logger.debug("Status %s for %s" % (currStatus, currId))
                                currId = None
                                currStatus = None
                                currReason = ''

                        else:
                            line = statusProc.fromchild.next()
                
                except StopIteration:
                    if currId<>None:
                        JobPoller.logger.debug("Unprocessed job: " + currId)
                    statusProc.fromchild.close()
                
            finally:
                jobToSend = min(jobLeft, self.parameters.maxRunningJobs - len(self.table))
                self.lock.release()
                if self.proxyMan<>None:
                    self.proxyMan.endLock()

            self.processRunningJobs()
            self.processFinishedJobs()

            if jobToSend>0:
                #TODO: imcremental pool feeding
                self.pool.submit(jobToSend)
                snapshot = self.valueSnapshot()
                if len(snapshot)>0:
                    minTS = float(min(snapshot)) -1
                jobLeft = self.parameters.numberOfJob - self.pool.getSuccesses()
            JobPoller.logger.debug("Job left: " + str(jobLeft) + " job processed: " + str(jobProcessed))
            
            timeToSleep = self.parameters.rate - int(time.time() - ts)
            if timeToSleep>0:
                time.sleep(timeToSleep)

    def put(self, uri, timestamp):
        self.lock.acquire()
        JobPoller.logger.info("Submitted job: " + uri)
        self.table[uri] = timestamp
        self.lock.release()

    def size(self):
        self.lock.acquire()
        result = len(self.table)
        self.lock.release()
        return result

    def valueSnapshot(self):
        self.lock.acquire()
        result = self.table.values()
        self.lock.release()
        return result
            
    def shutdown(self):
        self.pool.shutdown()
        for key in self.tableOfResults:
            JobPoller.logger.info("Job with status %s: %d" % (key, self.tableOfResults[key]))
        return 0
