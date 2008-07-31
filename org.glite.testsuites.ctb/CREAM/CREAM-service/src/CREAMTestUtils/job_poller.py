import re, string
import threading
import time
import popen2
import testsuite_utils, job_utils
import log4py

from submit_pool import JobSubmitterPool

class JobPoller(threading.Thread):

    logger = log4py.Logger().get_instance(classid="JobPoller")

    runningStates = ['IDLE', 'RUNNING', 'REALLY-RUNNING']
    finalStates = ['DONE-OK', 'DONE-FAILED', 'ABORTED', 'CANCELLED']
    
    def __init__(self, parameters, cmds):
        threading.Thread.__init__(self)
        self.table = {}
        self.lock = threading.Lock()
        self.parameters = parameters
        self.cmdTable = cmds
        
        self.finishedJobs = None
        
        self.jobRE = re.compile("JobID=\[([^\]]+)")
        self.statusRE = re.compile('Status\s*=\s*\[([^\]]+)')
        self.failureRE = re.compile('FailureReason\s*=\s*\[([^\]]+)')
        
        self.pool = JobSubmitterPool(parameters, cmds, self)
        self.tableOfResults = {'DONE-OK': 0, 'DONE-FAILED': 0, \
                               'ABORTED': 0, 'CANCELLED': 0}

    def manageRunningState(self, currId):
        pass
    
    def processFinishedJobs(self):
        self.finishedJobs.clear()
#        job_utils.eraseJobs(self.finishedJobs, self.cmdTable['purge'], JobPoller.logger)
#        self.finishedJobs = []
    
    def processRunningJobs(self):
        pass
    
    def run(self):

        minTS = time.time()

        serviceHost = self.parameters.resourceURI[:string.find(self.parameters.resourceURI,'/')]
                
        jobLeft = self.parameters.numberOfJob
        jobProcessed = 0
        
        while jobProcessed<self.parameters.numberOfJob:
            
            ts = time.time()

            statusCmd = self.cmdTable['status'] \
                        + " -f \"" + time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(minTS)) \
                        + "\" -e " + serviceHost + " --all"
            JobPoller.logger.debug('Command line: ' + statusCmd)
            statusProc = popen2.Popen4(statusCmd, True)
            
            self.lock.acquire()
            try:
                
                currId = None
                
                for line in statusProc.fromchild:
                    if 'ERROR' in line or 'FATAL' in line:
                        JobPoller.logger.error(line[24:])
                        continue
                    
                    tmpm = self.jobRE.search(line)
                    if tmpm<>None:
                        currId = tmpm.group(1)
                        if not currId in self.table:
                            currId = None
                        continue
                    
                    tmpm = self.statusRE.search(line)
                    if tmpm<>None and currId<>None:
                        jobStatus = tmpm.group(1)
                        
                        if jobStatus in JobPoller.runningStates:
                            self.manageRunningState(currId)
                        elif jobStatus in JobPoller.finalStates:
                            del(self.table[currId])
                            self.finishedJobs.append(currId, jobStatus)
                            jobProcessed += 1
                            self.tableOfResults[jobStatus] += 1
                            JobPoller.logger.info("Execution terminated for job: %s with status %s"  
                                                  % (currId, jobStatus))
                        continue
                    
                    tmpm = self.failureRE.search(line)
                    if tmpm<>None and currId<>None:
                        jobFailure = tmpm.group(1)
                        JobPoller.logger.debug("Failure reason for job %s: %s" %(currId, jobFailure)) 
                        continue

                statusProc.fromchild.close()

                if len(self.finishedJobs)>0 and len(self.table):
                    minTS = float(min(self.table.values())) -1
                    
            finally:
                self.lock.release()

            self.processRunningJobs()
            self.processFinishedJobs()

            #TODO: imcremental pool feeding
            jobToSend = min(jobLeft, self.parameters.maxRunningJobs - len(self.table))
            self.pool.submit(jobToSend)
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
    
    def shutdown(self):
        self.pool.shutdown()
        for key in self.tableOfResults:
            JobPoller.logger.info("Job with status %s: %d" % (key, self.tableOfResults[key]))
        return 0
