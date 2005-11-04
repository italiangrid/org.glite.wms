
#ifndef __JOBCACHE_H__
#define __JOBCACHE_H__

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "jnlFile_ex.h"
#include "jobCacheOperation.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "jnlFileManager.h"
#include "ClassadSyntax_ex.h"
#include "elementNotFound_ex.h"
#include "creamJob.h" 

//#include "classad_distribution.h"

#define OPERATION_SEPARATOR ":"
#define MAX_OPERATION_COUNTER 10

#define DEFAULT_JNLFILE "/tmp/jobCachePersistFile"
#define DEFAULT_SNAPFILE "/tmp/jobCachePersistFile.snapshot"

namespace api = glite::ce::cream_client_api;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	//______________________________________________________
	class jobCache {

	private:
	  static jobCache *_instance;
	  static std::string jnlFile;
	  static std::string snapFile;

	  std::map<std::string, CreamJob> hash;
	  std::map<std::string, std::string> cream_grid_hash;
	  pthread_mutex_t mutexHash;
	  pthread_mutex_t mutexSnapFile;
	  int operation_counter;

	  void loadJournal(void) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&, jnlFileReadOnly_ex&);

	  void loadSnapshot(void) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&);

	  std::ifstream is;
	  std::ofstream os;
	  glite::wms::ice::util::jnlFileManager* jnlMgr;

	  CreamJob unparse(const std::string&) throw(ClassadSyntax_ex&);
	  void toString(const CreamJob&, std::string&);

	  void getOperation(const std::string&,
			    operation&,
			    std::string&);

	protected:
	  jobCache(const std::string&, const std::string&) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&);
	  
	public:

	  static jobCache* getInstance() throw(jnlFile_ex&, ClassadSyntax_ex&);
	  static void setJournalFile(const std::string& jnl) { jnlFile = jnl; }
	  static void setSnapshotFile(const std::string& snap) { snapFile = snap; }

	  virtual ~jobCache();

	  void put(const CreamJob&) throw(jnlFile_ex&, jnlFileReadOnly_ex&);

	  void updateStatusByCreamJobID(const std::string& cid, const api::job_statuses::job_status&) throw(elementNotFound_ex&);

	  void updateStatusByGridJobID(const std::string& gid, const api::job_statuses::job_status&) throw(elementNotFound_ex&);

	  void remove_by_grid_jobid(const std::string&) 
	    throw (jnlFile_ex&, jnlFileReadOnly_ex&);

	  void remove_by_cream_jobid(const std::string&) 
	    throw (jnlFile_ex&, jnlFileReadOnly_ex&, elementNotFound_ex&);

	  bool isFinished_by_grid_jobid(const std::string&) throw(elementNotFound_ex&);
	  bool isFinished_by_cream_jobid(const std::string&) throw(elementNotFound_ex&);

	  std::string get_grid_jobid_by_cream_jobid(const std::string&) throw(elementNotFound_ex&);
	  std::string get_cream_jobid_by_grid_jobid(const std::string&) throw(elementNotFound_ex&);
	  CreamJob getJobByCreamJobID(const std::string&) throw(elementNotFound_ex&);
	  CreamJob getJobByGridJobID(const std::string&) throw(elementNotFound_ex&);
	  api::job_statuses::job_status 
	  getStatus_by_cream_jobid(const std::string&) throw(elementNotFound_ex&);
	  api::job_statuses::job_status
	  getStatus_by_grid_jobid(const std::string&) throw(elementNotFound_ex&);
	  
	  void dump(void) throw(jnlFile_ex&);
	  void print(FILE*);
	  void getActiveCreamJobIDs(std::vector<std::string>& target) ;  
	};
      }
    }
  }
}

#endif
