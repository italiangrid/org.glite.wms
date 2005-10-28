
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
#include "classad_distribution.h"

#define OPERATION_SEPARATOR ":"
#define MAX_OPERATION_COUNTER 10

#define DEFAULT_JNLFILE "/tmp/jobCachePersistFile"
#define DEFAULT_SNAPFILE "/tmp/jobCachePersistFile.snapshot"

namespace api = glite::ce::cream_client_api;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	//______________________________________________________________________
	class CreamJob {
	public:
	  std::string jobid;
	  api::job_statuses::job_status status;
	  
	  CreamJob() : jobid( "" ), 
	    status( api::job_statuses::UNKNOWN ) { };

	  CreamJob(const CreamJob& j) {
	    this->jobid = j.jobid;
	    this->status = j.status;
	  }

	  CreamJob( const std::string& jid,
		    const api::job_statuses::job_status s) 
	    : jobid( jid ), status( s ) {}

	};

	//______________________________________________________________________
	struct Job {
	  std::string grid_jobid;
	  CreamJob cream_job;
	};

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
	  classad::ClassAdParser parser;
	  classad::ClassAdUnParser unp;

	  void loadJournal(void) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&, jnlFileReadOnly_ex&);

	  void loadSnapshot(void) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&);

	  std::ifstream is;
	  std::ofstream os;
	  glite::wms::ice::util::jnlFileManager* jnlMgr;

	  Job unparse(const std::string&) throw(ClassadSyntax_ex&);
	  void getOperation(const std::string&,
			    operation&,
			    std::string&) ;

	  std::string makeClassad(const std::string&, 
				  const std::string&, 
				  const api::job_statuses::job_status&);

	protected:
	  jobCache(const std::string&, const std::string&) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&);
	  
	public:

	  static jobCache* getInstance() throw(jnlFile_ex&, ClassadSyntax_ex&);
	  static void setJournalFile(const std::string& jnl) { jnlFile = jnl; }
	  static void setSnapshotFile(const std::string& snap) { snapFile = snap; }

	  virtual ~jobCache();

	  void put(const std::string& grid_jobid, 
		   const std::string& cream_jobid, 
		   const api::job_statuses::job_status& status = api::job_statuses::UNKNOWN)
	    throw(jnlFile_ex&, jnlFileReadOnly_ex&);

	  void remove_by_grid_jobid(const std::string&) 
	    throw (jnlFile_ex&, jnlFileReadOnly_ex&);

	  void remove_by_cream_jobid(const std::string&) 
	    throw (jnlFile_ex&, jnlFileReadOnly_ex&);

	  bool isFinished_by_grid_jobid(const std::string&);
	  bool isFinished_by_cream_jobid(const std::string&);

	  std::string get_grid_jobid_by_cream_jobid(const std::string&);
	  std::string get_cream_jobid_by_grid_jobid(const std::string&);

	  api::job_statuses::job_status getStatus_by_grid_jobid(const std::string&);
	  api::job_statuses::job_status getStatus_by_cream_jobid(const std::string&);
	  void dump(void) throw(jnlFile_ex&);
	  void print(FILE*);
	};
      }
    }
  }
}

#endif
