
#ifndef __JNLFILEMANAGER_H__
#define __JNLFILEMANAGER_H__

#include <string>
#include <exception>
#include <fstream>
#include <pthread.h>
#include <streambuf>
#include <sstream>
#include "jobCacheOperation.h"
#include "jnlFile_ex.h"
#include "jnlFileReadOnly_ex.h"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class jnlFileManager {
	  
	  std::string filename;
	  std::ofstream os;
	  std::ifstream is;
	  bool readonly, isempty;
	  pthread_mutex_t mutexJnlFile;
	  unsigned long long savedoff;

	public:
	  jnlFileManager(const std::string&) throw (glite::wms::ice::util::jnlFile_ex&);
	  virtual ~jnlFileManager() throw () {
	    if(readonly) is.close();
	    else os.close();
	  }
	  
	  void truncate() throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);
	  void log(const glite::wms::ice::util::operation&, 
		   const std::string& params) throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);
	  
	  bool getNextOperation(std::string&) throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);
	  void readonly_mode(const bool&) throw (glite::wms::ice::util::jnlFile_ex&);
	  void rewind() throw (glite::wms::ice::util::jnlFile_ex&);
	  void lock(void);
	  void unlock(void);
	};

	/**
	 * A Class that safely handle journal Manager locking/unlokcing
	 */
	class lockJournalManager {
	  jnlFileManager *jnl;
	public:
	  lockJournalManager(jnlFileManager* jnlM) : jnl(jnlM) { 
	    jnl->lock();
	  }
	  
	  ~lockJournalManager() { 
	    jnl->unlock();
	  }
	};

      }
    }
  }
}
#endif
	
