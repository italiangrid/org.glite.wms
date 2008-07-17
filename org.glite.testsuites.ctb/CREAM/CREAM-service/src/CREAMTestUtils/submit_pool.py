import sys, os
import threading
import time
import popen2
import string
import log4py

class SubmitterThread(threading.Thread):
    
    logger = log4py.Logger().get_instance(classid="SubmitterThread")
    
    def __init__(self, pool, cmd, tName):
        threading.Thread.__init__(self)
        self.setName(tName)
        self.pool = pool
        self.command = cmd

    def run(self):
        running = True
        while running:
            running = self.pool.wait()
            if running:
                notified = False
                try:
                    SubmitterThread.logger.debug("Thread submitting: " + self.getName())
                    tmpTS = time.time()
                    submitProc = popen2.Popen4(self.command, True)
                    tmpTs = time.time() - tmpTS
                    for line in submitProc.fromchild:
                        if line[0:8]=='https://':
                            self.pool.notifySubmitResult(jobId=string.strip(line))
                            notified = True
                            break
                        if 'ERROR' in line or 'FATAL' in line:
                            self.pool.notifySubmitResult(failure=line[24:] + " execution time: " + str(tmpTs))
                            notified = True
                            break
                    if not notified:
                        self.pool.notifySubmitResult(failure='Missing jobId or failure reason')
                    submitProc.fromchild.close()
                except:
                    SubmitterThread.logger.error(str(sys.exc_info()[1]))
                    self.pool.notifySubmitResult(failure=str(sys.exc_info()[1]))

class JobSubmitterPool:
    
    logger = log4py.Logger().get_instance(classid="JobSubmitterPool")
    
    def __init__(self, parameters, cmdTable, jobTable=None):
        self.jobTable = jobTable
        self.successes = 0
        self.failures = 0
        self.globalSem = threading.Semaphore(1)
        self.masterCond = threading.Condition()
        self.slaveCond = threading.Condition()
        self.running = True
        self.left = 0
        self.processed = 0
        
        cmd = cmdTable['submit']
        if parameters.delegationID=='':
            cmd += ' -a'
        else:
            cmd += ' -D ' + parameters.delegationID
        cmd += ' -r  ' + parameters.resourceURI
        cmd += ' ' + parameters.jdl
        JobSubmitterPool.logger.debug("Command line: " + cmd)
        
        for k in range(0,parameters.maxConcurrentSubmit):
            subThr = SubmitterThread(self, cmd, "Submitter"  + str(k))
            subThr.start()
        

    def submit(self, numberOfSubmit, resetCounter=False):
        
        if numberOfSubmit<1:
            return 0
        
        self.globalSem.acquire()
        
        self.slaveCond.acquire()
        self.left = numberOfSubmit
        self.processed = 0
        self.slaveCond.release()
        
        go = True
        self.masterCond.acquire()
        if resetCounter:
            self.successes = 0
            self.failures = 0
        
        while go:
            self.slaveCond.acquire()
            if self.left>0:
                self.slaveCond.notifyAll()
            self.slaveCond.release()
            
            go = self.processed<numberOfSubmit
            if go:
                self.masterCond.wait()
            
        self.masterCond.release()
        self.globalSem.release()
        return 0

    def __call__(self, numberOfSubmit):
        self.submit(numberOfSubmit)
            
    def notifySubmitResult(self, jobId='', failure=None):
        self.slaveCond.acquire()
        self.processed += 1
        self.slaveCond.release()
        
        self.masterCond.acquire()
        if jobId<>'':
            if self.jobTable<>None:
                self.jobTable.put(jobId, time.time())
            self.successes += 1
        if failure<>None:
            JobSubmitterPool.logger.error(failure)
            self.failures += 1
        self.masterCond.notify()
        self.masterCond.release()

    def wait(self):
        self.slaveCond.acquire()
        while self.left<1 and self.running:
            self.slaveCond.wait()
        self.left -= 1
        result = self.running
        self.slaveCond.release()
        return result
        
    def getSuccesses(self):
        self.masterCond.acquire()
        result =  self.successes
        self.masterCond.release()
        return result

    def getFailures(self):
        self.masterCond.acquire()
        result =  self.failures
        self.masterCond.release()
        return result
    
    def count(self):
        self.masterCond.acquire()
        result = self.successes + self.failures
        self.masterCond.release()
        return result
    
    def shutdown(self):
        self.slaveCond.acquire()
        self.running = False
        self.slaveCond.notifyAll()
        self.slaveCond.release()

class MokeObject:
    def __init__(self, resource, jdl, delegationID=''):
        self.maxConcurrentSubmit = 5
        self.delegationID = delegationID
        self.resourceURI = resource
        self.jdl = jdl
        
    def put(self, uri, timestamp):
        print " -- " + uri

        
def main():
    if os.environ.has_key("GLITE_LOCATION"):
        gliteLocation = os.environ["GLITE_LOCATION"]
    else:
        gliteLocation = "/opt/glite"
        
    if len(sys.argv)>4:
        delegationID = sys.argv[4]
    else:
        delegationID = ''
        
    mokeObj = MokeObject(sys.argv[1], sys.argv[2], delegationID)
    cmdTable = { 'submit' : gliteLocation + '/bin/glite-ce-job-submit'}
    pool = JobSubmitterPool(mokeObj, cmdTable, mokeObj)
    pool.submit(int(sys.argv[3]))
    pool.shutdown()

if __name__ == "__main__":
    main()
