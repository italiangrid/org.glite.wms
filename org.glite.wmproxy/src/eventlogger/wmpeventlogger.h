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
#include "glite/jdl/JobAd.h"

// JobId
#include "glite/wmsutils/jobid/JobId.h"

// LB logger API
#include "glite/lb/consumer.h"
#include "glite/lb/JobStatus.h"


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
			LOG_REFUSE,
			LOG_CANCEL,
			LOG_CLEAR,
			LOG_ABORT,
			LOG_ENQUEUE_START,
			LOG_ENQUEUE_OK, LOG_ENQUEUE = LOG_ENQUEUE_OK,
			LOG_ENQUEUE_FAIL,
			LOG_USER_TAG
		};
		
		WMPEventLogger(const std::string &endpoint);
		virtual ~WMPEventLogger() throw();

		void init(const std::string &nsHost, int nsPort,
			glite::wmsutils::jobid::JobId *id,
			const std::string &desturiprotocol = "gridftp",
			int desturiport = 0);

		// LB Proxy
		void setLBProxy(bool value, char * userdn = NULL);
		bool getLBProxy();
		
		// Sequence code
		char * getSequence();
		void setSequenceCode(const std::string &seqcode);
		void incrementSequenceCode();
		
		// Proxy renewal
		char * registerProxyRenewal(const std::string &proxy_path,
			const std::string &my_proxy_server);
		void unregisterProxyRenewal();
			
		// Register methods
		void registerJob(glite::jdl::JobAd *ad, const std::string &path);
		std::vector<std::string> registerDag(WMPExpDagAd *ad,
			const std::string &path);
		
		std::vector<std::string> WMPEventLogger::generateSubjobsIds(int res_num);

		void logListener(const char* host, int port);
		void logCheckpointable(const char* current_step, const char* state);
		
		// Event logging
		void logEvent(event_name event, const char *reason, bool retry, bool test,
			const char *file_queue = NULL, const char *jdl = NULL);
		int logAbortEventSync(char *reason);
		int logAcceptEventSync();

		void logUserTag(std::string name, const std::string &value);
		void logUserTags(classad::ClassAd *userTags);
		void logUserTags(std::vector<std::pair<std::string,
			classad::ExprTree*> > userTags);

		void setLoggingJob(const std::string &jid, const char *seq_code = NULL);

		void setUserProxy(const std::string &proxy);
		
		regJobEvent retrieveRegJobEvent(const std::string &jobid_str);
		
		std::pair<std::string, regJobEvent> isStartAllowed();
		
		bool isAborted(std::string &reason);
		
		std::string getLastEventSeqCode();
		
		/**
		 * Gets the status of the context job
		 * @param childreninfo if set to true, subjobs info are also returned (if any)
		 * @return the status of the context job
		 */
		glite::lb::JobStatus getStatus(bool childreninfo);

		void registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs);
		edg_wlc_JobId *subjobs;
		
		
	private:
		
		edg_wll_Context ctx;
		glite::wmsutils::jobid::JobId *id;
		std::string dest_uri;
		std::string lb_host;
		int lb_port;
		std::string server;
		bool lbProxy_b;
		std::string desturiprotocol;
		std::string delegatedproxy;
		int desturiport;
		
		std::string error_message(const std::string &api, int exitcode = 0);
		
		void testAndLog(int &code, bool &with_hp, int &lap);
		int logEvent(event_name event, const char *reason,
			const char *file_queue = NULL, const char *jdl = NULL);
		void logEvent(event_name event, const char *reason, bool retry,
			const char *file_queue = NULL, const char *jdl = NULL);
			
		void randomsleep();
};

} // eventlogger
} // wmproxy
} // wms
} // glite

#endif // GLITE_WMS_WMPROXY_WMPEVENTLOGGER_H
