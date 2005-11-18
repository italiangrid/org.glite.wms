/*
 	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpeventlogger.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//


#ifndef GLITE_WMS_WMPROXY_WMPEVENTLOGGER_H
#define GLITE_WMS_WMPROXY_WMPEVENTLOGGER_H

#include <vector>

// DagAd
#include "wmpexpdagad.h"
#include "glite/wms/jdl/JobAd.h"

// JobId
#include "glite/wmsutils/jobid/JobId.h"

// LB logger API
#include "glite/lb/consumer.h"
#include "glite/lb/JobStatus.h"

#ifndef HAVE_LBPROXY
#define EDG_WLL_SOURCE_WM_PROXY EDG_WLL_SOURCE_NETWORK_SERVER
#endif

namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {

struct regJobEvent {
	std::string instance;
	std::string jdl;
	std::string parent;
};	
		
class WMPEventLogger  {

	public:
		enum event_name {
			LOG_ACCEPT,
			LOG_CANCEL,
			LOG_CLEAR,
			LOG_ABORT,
			LOG_ENQUEUE_OK, LOG_ENQUEUE = LOG_ENQUEUE_OK,
			LOG_ENQUEUE_FAIL
		};
		
		WMPEventLogger(const std::string &endpoint);
		virtual ~WMPEventLogger() throw();

		void init(const std::string &nsHost, int nsPort,
			glite::wmsutils::jobid::JobId *id,
			const std::string &desturiprotocol = "gridftp",
			int desturiport = 0);

		// LB Proxy
		void setLBProxy(bool value) {lbProxy_b=value;}
		bool getLBProxy() {return lbProxy_b;}
		
		// Sequence code
		char * getSequence();
		void setSequenceCode(const std::string &seqcode);
		void incrementSequenceCode();
		
		// Proxy renewal
		char * registerProxyRenewal(const std::string &proxy_path,
			const std::string &my_proxy_server);
		void unregisterProxyRenewal();
			
		// Register methods
		void registerJob(glite::wms::jdl::JobAd *ad);
		std::vector<std::string> registerDag(WMPExpDagAd *ad);
		
		std::vector<std::string> WMPEventLogger::generateSubjobsIds(int res_num);

		bool logListener(const char* host, int port);
		bool logCheckpointable(const char* current_step, const char* state);
		
		// Event logging
		bool logEvent(event_name event, const char* reason,
			const char* file_queue=NULL, const char* jdl=NULL);
		void logEvent(event_name event, const char* reason, bool retry,
			const char* file_queue=NULL, const char* jdl=NULL);
		void logEvent(event_name event, const char* reason, bool retry, bool test,
			const char* file_queue=NULL, const char* jdl=NULL);
		bool logAbortEventSync(char *reason);

		void logUserTag(std::string name, const std::string &value);
		void logUserTags(classad::ClassAd *userTags);
		void logUserTags(std::vector<std::pair<std::string,
			classad::ExprTree*> > userTags);

		void setLoggingJob(const std::string &jid, const char* seq_code = NULL);

		int setUserProxy(const std::string &proxy);
		
		//bool retrieveEvent(const std::string &jobid_str, event_name eventname);
		regJobEvent retrieveRegJobEvent(const std::string &jobid_str);
		bool isRegisterEventOnly();
		
		std::string getUserTag(const std::string &tagname);
		
		
		/**
		 * Gets the full status of the job represented by the input job id
		 * @param jid the job id to get the status for
		 * @return the status of the corresponding job
		 */
		static glite::lb::JobStatus getStatus(glite::wmsutils::jobid::JobId *jid,
			const std::string &delegatedproxy, bool childreninfo = false);
			
		static const std::string QUERY_SEQUENCE_CODE;
		static const std::string QUERY_JDL_ORIGINAL;

		void registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs);
		edg_wlc_JobId * subjobs;
	private:
		
		void testAndLog(int &code, bool &with_hp, int &lap);
		
		const char * error_message(const char *api);
		std::string dest_uri;
		glite::wmsutils::jobid::JobId *id;
		edg_wll_Context ctx;
		//edg_wlc_JobId * subjobs;
		std::string lb_host;
		int lb_port;
		std::string server;
		bool lbProxy_b;
		std::string desturiprotocol;
		std::string delegatedproxy;
		int desturiport;
		
		static const char * GLITE_WMS_LOG_DESTINATION;
		static const int LB_RENEWAL_PORT = 7512;
		static const int LOG_RETRY_COUNT = 3;
};

} // eventlogger
} // wmproxy
} // wms
} // glite

#endif // GLITE_WMS_WMPROXY_WMPEVENTLOGGER_H
