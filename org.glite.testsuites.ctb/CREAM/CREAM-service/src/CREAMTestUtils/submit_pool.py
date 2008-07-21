import sys, os
import threading
import time
import popen2
import string
import log4py
import testsuite_utils

class SubmitterThread(threading.Thread):
    
    logger = log4py.Logger().get_instance(classid="SubmitterThread")
    
    def __init__(self, pool, scmd, dcmd, tName):
        threading.Thread.__init__(self)
        self.setName(tName)
        self.pool = pool
        self.submitFStr = scmd
        self.delegatFStr = dcmd

    def run(self):
        running = True
        while running:
            running = self.pool.wait()
            if running:
                notified = False
                try:
                    SubmitterThread.logger.debug("Thread submitting: " + self.getName())
                    tmpTS = time.time()
                    
                    if self.delegatFStr<>None:
                        delegCmd = self.delegatFStr % (os.getpid(), tmpTS)
                        submCmd = self.submitFStr % (os.getpid(), tmpTS)
                        
                        SubmitterThread.logger.debug("Delegate cmd: " +delegCmd)
                        delegProc = popen2.Popen4(delegCmd)
                        for line in delegProc.fromchild:
                            if 'ERROR' in line or 'FATAL' in line:
                                self.pool.notifySubmitResult(failure=line[24:])
                                notified = True
                        delegProc.fromchild.close()
                    else:
                        delegCmd = self.delegatFStr
                        submCmd = self.submitFStr
                    
                    if not notified:
                        SubmitterThread.logger.debug("Submit  cmd: " + submCmd)
                        submitProc = popen2.Popen4(submCmd)
                        for line in submitProc.fromchild:
                            if line[0:8]=='https://':
                                self.pool.notifySubmitResult(jobId=string.strip(line), timestamp=tmpTS)
                                notified = True
                                break
                            if 'ERROR' in line or 'FATAL' in line:
                                self.pool.notifySubmitResult(failure=line[24:])
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
        
        if parameters.delegationID=='':
            dString = 'DELEGID%d.%f'
            dcmd = '%s -e %s %s' % (cmdTable['delegate'], \
                                              parameters.resourceURI[:string.find(parameters.resourceURI,'/')],
                                              dString)
        else:
            dString = parameters.delegationID
            dcmd = None
            
        scmd = '%s -D %s -r %s %s' % (cmdTable['submit'], \
                                      dString, parameters.resourceURI, parameters.jdl)
        
        for k in range(0,parameters.maxConcurrentSubmit):
            subThr = SubmitterThread(self, scmd, dcmd, "Submitter"  + str(k))
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
            
    def notifySubmitResult(self, jobId='', failure=None, timestamp=0):
        self.slaveCond.acquire()
        self.processed += 1
        self.slaveCond.release()
        
        self.masterCond.acquire()
        if jobId<>'':
            if self.jobTable<>None:
                self.jobTable.put(jobId, timestamp)
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
    cmdTable = testsuite_utils.getCECommandTable()
        
    if len(sys.argv)>4:
        delegationID = sys.argv[4]
    else:
        delegationID = ''
        
    mokeObj = MokeObject(sys.argv[1], sys.argv[2], delegationID)
    pool = JobSubmitterPool(mokeObj, cmdTable, mokeObj)
    pool.submit(int(sys.argv[3]))
    pool.shutdown()

if __name__ == "__main__":
    main()
