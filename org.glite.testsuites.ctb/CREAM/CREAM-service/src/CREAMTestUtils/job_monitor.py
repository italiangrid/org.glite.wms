
import threading
import time
import popen2
import log4py

import job_utils, testsuite_utils
from submit_pool import JobSubmitterPool

class JobMonitor(threading.Thread):
    logger = log4py.Logger().get_instance(classid="JobMonitor")
    
    runningStates = ['IDLE', 'RUNNING', 'REALLY-RUNNING']
    finalStates = ['DONE-OK', 'DONE-FAILED', 'ABORTED', 'CANCELLED']
    
    def __init__(self, parameters, pManager=None):
        threading.Thread.__init__(self)
        self.table = {}
        self.notified = []
        self.lock = threading.Lock()
        self.parameters = parameters
        self.pool = JobSubmitterPool(parameters, self, pManager)
        self.tableOfResults = {'DONE-OK': 0, 'DONE-FAILED': 0, 'ABORTED': 0, 'CANCELLED': 0}
        
        self.finishedJobs = []
        
        self.lastNotifyTS = time.time()
        
    def manageNotifications(self):
        pass
    
    def processNotifiedJobs(self):
        pass

    def run(self):
        minTS = time.time()
         
        jobLeft = self.parameters.numberOfJob
        jobProcessed = 0
        
        while jobProcessed<self.parameters.numberOfJob:
            
            ts = time.time()
            
            self.lock.acquire()
            try:
                if (ts - self.lastNotifyTS) > self.parameters.rate*2:
                    JobMonitor.logger.warn("Missed last notify")
                    
                self.manageNotifications()
                    
                for (job, status) in self.notified:
                    if job in self.table:
                        del(self.table[job])
                        self.finishedJobs.append(job)
                        jobProcessed += 1
                        JobMonitor.logger.info("Terminated job %s with status %s" % (job, status))
                        self.tableOfResults[status] += 1
                            
                self.notified = []
                        
            except:
                JobMonitor.logger.error(sys.exc_info()[2])

            jobToSend = min(jobLeft, self.parameters.maxRunningJobs - len(self.table))                
            self.lock.release()
            
            self.processNotifiedJobs()
                
            job_utils.eraseJobs(self.finishedJobs)
            self.finishedJobs = []
                
            #TODO: imcremental pool feeding
            self.pool.submit(jobToSend)
#            snapshot = self.valueSnapshot()
#            if len(snapshot)>0:
#                minTS = float(min(snapshot)) -1
            jobLeft = self.parameters.numberOfJob - self.pool.getSuccesses()
            JobMonitor.logger.debug("Job left: " + str(jobLeft) + " job processed: " + str(jobProcessed))
            
            timeToSleep = self.parameters.rate - int(time.time() - ts)
            if timeToSleep>0:
                time.sleep(timeToSleep)
                
    def notify(self, jobHistory):
        self.lock.acquire()
        self.lastNotifyTS = time.time()
        (jobId, status) = jobHistory[-1]
        JobMonitor.logger.debug("Notify %s (%s)" % (jobId, status))
        if status in JobMonitor.finalStates:
            self.notified.append((jobId, status))
        self.lock.release()
    
    def put(self, uri, timestamp):
        self.lock.acquire()
        JobMonitor.logger.info("Submitted job: " + uri)
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
            JobMonitor.logger.info("Job with status %s: %d" % (key, self.tableOfResults[key]))
        return 0
