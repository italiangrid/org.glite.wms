
#ifndef __ICE_CORE_H__
#define __ICE_CORE_H__

#include <string>
#include <pthread.h>
#include "ClassadSyntax_ex.h"
#include "iceInit_ex.h"
#include "eventStatusListener.h"
#include "eventStatusPoller.h"
#include "thread.h"
#include "glite/wms/common/utilities/FLExtractor.h"

class jobRequest;

typedef glite::wms::common::utilities::FLExtractor<std::string>::iterator FLEit;

namespace glite {
  namespace wms {
    namespace ice {
      
      enum iceSupportedCommand {
	jobsubmit,
	jobcancel
      };
      
      struct iceCommand {
	std::string gid_jobid;
	std::string cream_jobid;
	iceSupportedCommand command;
	std::string jdl;
      };
      
      class ice {
	//glite::wms::ice::util::jobCache* job_cache;
	glite::wms::ice::util::eventStatusListener* listener;
	glite::wms::ice::util::eventStatusPoller* poller;
	bool status_listener_started;
	std::string ns_filelist;
	std::string wm_filelist;
	glite::wms::ice::util::thread* listenerThread, *pollerThread;

	std::vector<FLEit> requests;
	glite::wms::common::utilities::FLExtractor<std::string> fle;
	glite::wms::common::utilities::FileList<std::string> flns;

      public:
	ice(const std::string& NS_FL, 
	    const std::string& WM_FL,
	    const std::string& jobcache_persist_file,
	    const int& tcpport,
	    const bool& start_listener,
	    const bool& start_poller,
	    const int&  poller_delay,
	    const std::string& CreamUrl,
	    const std::string& hostCert)
	  throw(glite::wms::ice::iceInit_ex&);

	virtual ~ice();
	
	void startJobStatusListener() throw(glite::wms::ice::util::thread_start_ex&);
	void stopJobStatusListener();
	void startJobStatusPoller() throw(glite::wms::ice::util::thread_start_ex&);
	void stopJobStatusPoller();
	void clearRequests();
	void getNextRequests(std::vector<std::string>&); 
	void removeRequest(const unsigned int&);
	void ungetRequest(const unsigned int&);

	//	glite::wms::ice::util::jobCache* getJobCache(void) { return job_cache; };
      }; // class ice

    } // namespace ice
  } // namespace ce
} // namespace glite

#endif
