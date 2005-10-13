
/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$


#include "jobstatus.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// streams
#include<sstream>

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;

namespace glite {
namespace wms{
namespace client {
namespace services {

unsigned int DEFAULT_VERBOSITY = 1;
unsigned int MAX_VERBOSITY = 3;

/*
* Default constructor
*/
JobStatus::JobStatus( ){
	// init of the string attributes
	inOpt = NULL ;
	fromOpt = NULL ;
	toOpt = NULL ;
	statusOpt = NULL ;
	exdOpt = NULL ;
	// init of the boolean attributes
	allOpt  = false;
	// init of the integer attributes
	vbOpt = NULL;
};
/*
* Default destructor
*/
JobStatus::~JobStatus( ){
	if (inOpt){ delete(inOpt); }
	if (fromOpt ){ delete(fromOpt  ); }
	if (toOpt){ delete( toOpt ); }
	if (statusOpt){ delete(statusOpt); }
	if (exdOpt ){ delete(exdOpt ); }
	if (vbOpt ){ delete( vbOpt); }
};
/**
*	Reads the command-line user arguments and sets all the class attributes
*/
void JobStatus::readOptions (int argc,char **argv){
	Job::readOptions  (argc, argv, Options::JOBSTATUS);
        // input file
        inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	// JobId's
        if (inOpt){
        	jobIds = wmcUtils->getItemsFromFile(*inOpt);
        } else {
        	jobIds = wmcOpts->getJobIds();
        }
        jobIds = wmcUtils->checkJobIds (jobIds);
	if ( jobIds.size( ) > 1 && ! wmcOpts->getBoolAttribute(Options::NOINT) ){
        	jobIds = wmcUtils->askMenu(jobIds, Utils::MENU_JOBID);
         }
         // checks if the proxy file pathname is set
	if (proxyFile) {
        	logInfo->print (WMS_DEBUG, "Proxy File:", proxyFile);
 	} else {
                throw WmsClientException(__FILE__,__LINE__,
                                "readOptions",DEFAULT_ERR_CODE,
                                "Invalid Credential",
                                "No valid proxy file pathname" );
        }
	vbOpt =(unsigned int*) wmcOpts->getIntAttribute(Options::VERBOSE);
	if (!vbOpt){
		vbOpt = (unsigned int*)malloc(sizeof(int));
		*vbOpt = Options::DEFAULT_VERBOSITY ;
	} else if (*vbOpt < 0 ||  *vbOpt > Options::MAX_VERBOSITY ){
		ostringstream err;
		for (unsigned int i = 0; i != (Options::MAX_VERBOSITY+1); i++){
			err << i ;
			if (i < Options::MAX_VERBOSITY){ err << " | "; }
		}
                throw WmsClientException(__FILE__,__LINE__,
                                "readOptions",DEFAULT_ERR_CODE,
                                "Invalid Argument Value",
                                "Invalid Verbosity Level: possible values " + err.str() );
	}

};
/**
* perfroms the main operations
*/
void JobStatus::getStatus ( ){
	postOptionchecks();
	// checks that the jobids vector is not empty
	if (jobIds.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"getStatus", DEFAULT_ERR_CODE,
			"JobId Error",
			"No valid JobId for which the status can be retrieved" );
	}
};

void JobStatus::printInfo( ){
	ostringstream out;
	out << "\n" << wmcUtils->getStripe(88, "=") << "\n\n";
	out << "BOOKKEEPING INFORMATION:\n\n";
	out << "Current Status:\t" << "\n";

	if (*vbOpt > (Options::DEFAULT_VERBOSITY)){
		out << "Status Reason:\t" << "\n";
		out << "Submitted:\t" << "\n";
	}
	if (*vbOpt > (Options::DEFAULT_VERBOSITY+1)){
		out << "---\n\n";
		out << " - cancelling\t\t="   << "\n";
		out << " - children_num\t\t="   << "\n";
		out << " - cpuTime\t\t="   << "\n";
		out << " - done_code\t\t="   << "\n";
		out << " - expectUpdate\t\t="   << "\n";
		out << " - jobtype\t\t="   << "\n";
		out << " - lastUpdateTime\t\t="   << "\n";
		out << " - location\t\t="   << "\n";
		out << " - network_server\t\t="   << "\n";
		out << " - owner\t\t="   << "\n";
		out << " - resubmitted\t\t="   << "\n";
		out << " - seed\t\t="   << "\n";
		out << " - subjob_failed\t\t="   << "\n";
		out << " - user tags\t\t="   << "\n";
	};
	if (*vbOpt > (Options::DEFAULT_VERBOSITY+2)){
		out << "---\n\n";
		out << " - children_hist\t\t="   << "\n";
		out << " - jdl\t\t="   << "\n";
		out << " - stateEnterTimes =\t\t="   << "\n";
	};
	out << "\n" << wmcUtils->getStripe(88, "=") << "\n\n";

}

/*
v=0
**************************************************************
BOOKKEEPING INFORMATION:

Status info for the Job : https://ghemon.cnaf.infn.it:9000/86dVrSqpm8IKElERDrEbXA
Current Status:     Waiting
**************************************************************/

/*
V=1
************************************************************
BOOKKEEPING INFORMATION:

Status info for the Job : https://ghemon.cnaf.infn.it:9000/86dVrSqpm8IKElERDrEbXA
Current Status:     Waiting
Status Reason:      BrokerHelper: no compatible resources
Submitted:          Wed Jun  1 17:22:24 2005 CEST

*************************************************************/

/*
V=2
************************************************************
BOOKKEEPING INFORMATION:

Status info for the Job : https://ghemon.cnaf.infn.it:9000/86dVrSqpm8IKElERDrEbXA
Current Status:     Waiting
Status Reason:      BrokerHelper: no compatible resources
Submitted:          Wed Jun  1 17:22:24 2005 CEST
---

- cancelling              =    0
- children_num            =    0
- cpuTime                 =    0
- done_code               =    0
- expectUpdate            =    0
- jobtype                 =    0
- lastUpdateTime          =    Wed Jun  1 17:44:32 2005 CEST
- location                =    WorkloadManager/ghemon.cnaf.infn.it/WM
- network_server          =    https://131.154.100.122:7443/glite_wms_wmproxy_server
- owner                   =    /C=IT/O=INFN/OU=Personal Certificate/L=DATAMAT DSAGRD/CN=Fabrizio Pacini
- resubmitted             =    0
- seed                    =    ywJA29Cf8sw2hgFSVbMOyg
- subjob_failed           =    0
- user tags               =    delegation_id=fp;jdl_original=[ requirements = ( other.GlueCEStateStatus == "Production" ) && ( other.defaultExsitmented ); RetryCount = 3; Arguments = "> ls.out"; JobType = "normal"; Executable = "/bin/ls"; StdOutput = "sim.out"; vo = "EGEE"; OutputSandbox = { "ls.out" }; VirtualOrganisation = "EGEE"; rank = 3; Type = "job"; StdError = "sim.err"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; InputSandbox = { "/tmp/nodeA.txt","/tmp/nodeB.txt" } ];lb_sequence_code=UI=000000:NS=0000000004:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000;

*************************************************************/
/*
v=3
************************************************************
BOOKKEEPING INFORMATION:

Status info for the Job : https://ghemon.cnaf.infn.it:9000/86dVrSqpm8IKElERDrEbXA
Current Status:     Waiting
Status Reason:      BrokerHelper: no compatible resources
Submitted:          Wed Jun  1 17:22:24 2005 CEST
---

- cancelling              =    0
- children_num            =    0
- cpuTime                 =    0
- done_code               =    0
- expectUpdate            =    0
- jobtype                 =    0
- lastUpdateTime          =    Wed Jun  1 17:44:32 2005 CEST
- location                =    WorkloadManager/ghemon.cnaf.infn.it/WM
- network_server          =    https://131.154.100.122:7443/glite_wms_wmproxy_server
- owner                   =    /C=IT/O=INFN/OU=Personal Certificate/L=DATAMAT DSAGRD/CN=Fabrizio Pacini
- resubmitted             =    0
- seed                    =    ywJA29Cf8sw2hgFSVbMOyg
- subjob_failed           =    0
- user tags               =    delegation_id=fp;jdl_original=[ requirements = ( other.GlueCEStateStatus == "Production" ) && ( other.defaultExsitmented ); RetryCount = 3; Arguments = "> ls.out"; JobType = "normal"; Executable = "/bin/ls"; StdOutput = "sim.out"; vo = "EGEE"; OutputSandbox = { "ls.out" }; VirtualOrganisation = "EGEE"; rank = 3; Type = "job"; StdError = "sim.err"; DefaultRank =  -other.GlueCEStateEstimatedResponseTime; InputSandbox = { "/tmp/nodeA.txt","/tmp/nodeB.txt" } ];lb_sequence_code=UI=000000:NS=0000000004:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000;
---
- children_hist   =   0

- jdl             =

       [
        requirements = ( other.GlueCEStateStatus == "Production" ) && ( other.defaultExsitmented );
        RetryCount = 3;
        edg_jobid = "https://ghemon.cnaf.infn.it:9000/86dVrSqpm8IKElERDrEbXA";
        Arguments = "> ls.out";
        OutputSandboxPath = "/var/glite/SandboxDir/86/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f86dVrSqpm8IKElERDrEbXA/output";
        JobType = "normal";
        Executable = "/bin/ls";
        OutputSandboxDestURI = "gsiftp://131.154.100.122/var/glite/SandboxDir/86/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f86dVrSqpm8IKElERDrEbXA/output/ls.out";
        CertificateSubject = "/C=IT/O=INFN/OU=Personal Certificate/L=DATAMAT DSAGRD/CN=Fabrizio Pacini";
        StdOutput = "sim.out";
        X509UserProxy = "/var/glite/SandboxDir/86/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f86dVrSqpm8IKElERDrEbXA/user.proxy"; 
        vo = "EGEE";
        OutputSandbox = { "ls.out" };
        InputSandboxPath = "/var/glite/SandboxDir/86/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f86dVrSqpm8IKElERDrEbXA/input";
        LB_sequence_code = "UI=000000:NS=0000000006:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000"; 
        VirtualOrganisation = "EGEE";
        rank = 3;
        Type = "job"; 
        StdError = "sim.err"; 
        WMPInputSandboxBaseURI = "gsiftp://131.154.100.122/var/glite/SandboxDir/86/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f86dVrSqpm8IKElERDrEbXA";
        DefaultRank =  -other.GlueCEStateEstimatedResponseTime;
        InputSandbox = { "gsiftp://131.154.100.122/var/glite/SandboxDir/86/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f86dVrSqpm8IKElERDrEbXA/nodeA.txt","gsiftp://131.154.100.122/var/glite/SandboxDir/86/https_3a_2f_2fghemon.cnaf.infn.it_3a9000_2f86dVrSqpm8IKElERDrEbXA/nodeB.txt" }
       ]
- stateEnterTimes =
      Submitted        : Wed Jun  1 17:22:24 2005 CEST
      Waiting          : Wed Jun  1 17:24:25 2005 CEST
      Ready            :                ---
      Scheduled        :                ---
      Running          :                ---
      Done             :                ---
      Cleared          :                ---
      Aborted          :                ---
      Cancelled        :                ---
      Unknown          :                ---

*************************************************************/


}}}} // ending namespaces
