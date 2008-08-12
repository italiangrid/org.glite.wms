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
    
    def __init__(self, parameters):
        threading.Thread.__init__(self)
        self.table = {}
        self.lock = threading.Lock()
        self.parameters = parameters
        
        self.finishedJobs = None
        self.jobProcessed = 0
        
        self.jobRE = re.compile("JobID=\[([^\]]+)")
        self.statusRE = re.compile('Status\s*=\s*\[([^\]]+)')
        self.failureRE = re.compile('FailureReason\s*=\s*\[([^\]]+)')
        
        self.pool = JobSubmitterPool(parameters, self)
        self.tableOfResults = {'DONE-OK': 0, 'DONE-FAILED': 0, \
                               'ABORTED': 0, 'CANCELLED': 0}

    def manageRunningState(self, currId):
        pass
    
    def processFinishedJobs(self):
        self.finishedJobs.clear()
    
    def processRunningJobs(self):
        pass
    
    def _handleStatusAndReason(self, currId, currStatus, currReason):
        if currStatus in JobPoller.runningStates:
            self.manageRunningState(currId)
        elif currStatus in JobPoller.finalStates:
            del(self.table[currId])
            self.jobProcessed += 1
            self.tableOfResults[currStatus] += 1
            if currReason<>None:
                self.finishedJobs.append(currId, currStatus, currReason)
                JobPoller.logger.info("Execution terminated for job: %s (%s, %s)"  
                                  % (currId, currStatus, currReason))
            else:
                self.finishedJobs.append(currId, currStatus)
                JobPoller.logger.info("Execution terminated for job: %s (%s)"  
                                  % (currId, currStatus))

    def run(self):

        minTS = time.time()

        serviceHost = self.parameters.resourceURI[:string.find(self.parameters.resourceURI,'/')]
                
        jobLeft = self.parameters.numberOfJob
        self.jobProcessed = 0
        
        while self.jobProcessed<self.parameters.numberOfJob:
            
            ts = time.time()

            statusCmd = testsuite_utils.cmdTable['status'] \
                        + " -f \"" + time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(minTS)) \
                        + "\" -e " + serviceHost + " --all"
            JobPoller.logger.debug('Command line: ' + statusCmd)
            statusProc = popen2.Popen4(statusCmd, True)
            
            self.lock.acquire()
            try:
                
                currId = None
                currStatus = None
                currReason = None
                
                for line in statusProc.fromchild:
                    if 'ERROR' in line or 'FATAL' in line:
                        JobPoller.logger.error(line[24:])
                        continue
                    
                    tmpm = self.jobRE.search(line)
                    if tmpm<>None:
                        #handle previous message
                        if currId<>None and currStatus<>None:
                            self._handleStatusAndReason(currId, currStatus, currReason)
                            currStatus = None
                            currReason = None
                        
                        currId = tmpm.group(1)
                        if not currId in self.table:
                            currId = None
                        continue
                    
                    tmpm = self.statusRE.search(line)
                    if tmpm<>None and currId<>None:
                        currStatus = tmpm.group(1)                        
                        continue
                    
                    tmpm = self.failureRE.search(line)
                    if tmpm<>None and currId<>None:
                        currReason = tmpm.group(1)

                statusProc.fromchild.close()
                #handle last message
                if currId<>None and currStatus<>None:
                    self._handleStatusAndReason(currId, currStatus, currReason)


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
            JobPoller.logger.debug("Job left: " + str(jobLeft) + " job processed: " + str(self.jobProcessed))
            
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
