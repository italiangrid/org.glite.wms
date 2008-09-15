from ZSI import ParsedSoap, SoapWriter, resolvers, TC, \
    FaultFromException, FaultFromZSIException
import os, sys
from SocketServer import ThreadingMixIn
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import log4py
import testsuite_utils, job_utils
import re, string

NSTable = {'cemon_types': 'http://glite.org/ce/monitorapij/types', \
                    'cemon_faults': 'http://glite.org/ce/monitorapij/faults', \
                    'cemon_consumer': 'http://glite.org/ce/monitorapij/ws'}

jobIDRE = re.compile('CREAM_JOB_ID\s*=\s*\"(CREAM\d+)\";')
jobStatusRE = re.compile('JOB_STATUS\s*=\s*\"([A-Z-]+)\";')

logger = log4py.Logger().get_instance(classid="CEMonitorConsumer")

class Notify:
    def __init__(self):
        pass
Notify.typecode = TC.Struct(Notify, [], 'NotifyResponse')

class SOAPRequestHandler(BaseHTTPRequestHandler):

    server_version = 'CEMonitorConsumer/1.8 ' + BaseHTTPRequestHandler.server_version
    
    def send_xml(self, text, code=200):

        self.send_response(code)
        self.send_header('Content-type', 'text/xml; charset="utf-8"')
        self.send_header('Content-Length', str(len(text)))
        self.end_headers()
        self.wfile.write(text)
        self.wfile.flush()

    def send_fault(self, f, code=500):

        self.send_xml(f.AsSOAP(), code)

    def handleNotify(self, soap):
    
        notification = soap.getElementsByTagNameNS(NSTable['cemon_types'], 'Notification')[0]
        events = notification.getElementsByTagNameNS(NSTable['cemon_types'], 'Event')
        for event in events:
            jobHistory = []
            
            messages = event.getElementsByTagNameNS(NSTable['cemon_types'], 'Message')
            for msg in messages:
                classad = msg.childNodes[0].nodeValue
                r1 = jobIDRE.search(classad)
                r2 = jobStatusRE.search(classad)
                if r1<>None and r2<> None:
                    jobHistory.append( (self.server.servicePrefix+r1.group(1), r2.group(1)) )
                    
            self.server.jobTable.notify(jobHistory)


    def do_POST(self):

        try:
            ct = self.headers['content-type']
            if ct.startswith('multipart/'):
                cid = resolvers.MIMEResolver(ct, self.rfile)
                xml = cid.GetSOAPPart()
                ps = ParsedSoap(xml, resolver=cid.Resolve)
            else:
                length = int(self.headers['content-length'])
                ps = ParsedSoap(self.rfile.read(length))
        except ParseException, e:
            self.send_fault(FaultFromZSIException(e))
            return
        except Exception, e:
            self.send_fault(FaultFromException(e, 1, sys.exc_info()[2]))
            return
        
        try:
            self.handleNotify(ps.body_root)
            #TODO: missing namespace cemon_consumer
            sw = SoapWriter(nsdict=NSTable)
            sw.serialize(Notify(), Notify.typecode)
            sw.close()
            self.send_xml(str(sw))
        except Exception, e:
            self.send_fault(FaultFromException(e, 0, sys.exc_info()[2]))
        
class ConsumerServer(ThreadingMixIn, HTTPServer):
    #See description in SocketServer.py about ThreadingMixIn
    def __init__(self, address, parameters, jobTable=None):
        HTTPServer.__init__(self, address, SOAPRequestHandler)
        self.jobTable = jobTable
        self.parameters = parameters
        self.running = False
        self.servicePrefix = 'https://' + parameters.resourceURI[:string.find(parameters.resourceURI,'/') + 1]
        self.cemonURL = self.servicePrefix + "ce-monitor/services/CEMonitor"
        
        self.proxyFile = testsuite_utils.getProxyFile()
        self.subscrId = job_utils.subscribeToCREAMJobs(self.cemonURL, \
                                                       self.parameters, self.proxyFile, logger)
        
    def __call__(self):
        self.running = True
        while self.running:
            self.handle_request()
        logger.debug("Consumer thread is over")
            
    def halt(self):
#        job_utils.unSubscribeToCREAMJobs(self.cemonURL, self.subscrId, \
#                                         self.parameters, self.proxyFile, logger)
        self.running = False

class DummyTable:
    def notify(self, jobHistory):
        print '---------------------------------------------'
        for item in jobHistory:
            print "JobID: %s status: %s" % item

def main():
    consumer = ConsumerServer(('',9000), DummyTable())
    consumer()

if __name__ == "__main__":
    main()


