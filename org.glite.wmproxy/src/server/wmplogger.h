#ifndef  GLITE_WMS_WMPROXY_WMPLOGGER_H
#define GLITE_WMS_WMPROXY_WMPLOGGER_H

// DagAd
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/ExpDagAd.h"

// JobId
#include "glite/wmsutils/jobid/JobId.h"

// LB logger API
#include "glite/lb/consumer.h"

//namespace glite {
//namespace wms {
//namespace wmproxy {

class WMPLogger  {

	public:
		enum txType {
			START,
			OK,
			FAIL
		};

		WMPLogger();
		virtual ~WMPLogger() throw();

		void init(const std::string &nsHost, int nsPort, glite::wmsutils::jobid::JobId *id);

		glite::wms::jdl::ExpDagAd *registerJob(glite::wms::jdl::JobAd *ad, int resource);
		void registerJob(glite::wms::jdl::JobAd *ad);
		void registerDag(glite::wms::jdl::ExpDagAd *ad);

		void transfer(txType tx, const std::string &jdl, const char *error = "");

		std::string getSequence();

		void logUserTags(classad::ClassAd *userTags);
		void logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> > userTags);

	private:
		void registerSubJobs(glite::wms::jdl::ExpDagAd *ad, edg_wlc_JobId *subjobs);
		const char * error_message(const char *api);

		glite::wmsutils::jobid::JobId *id;
		static pthread_mutex_t dgtransfer_mutex;
		edg_wll_Context ctx;
		std::string nsHost;
		int nsPort;
};

//} // wmproxy
//} // wms
//} // glite

#endif // GLITE_WMS_WMPROXY_WMPLOGGER_H
