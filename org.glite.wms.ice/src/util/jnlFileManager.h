
#ifndef __JNLFILEMANAGER_H__
#define __JNLFILEMANAGER_H__

#include <string>
#include <exception>
#include <fstream>
//#include <pthread.h>
#include <streambuf>
#include <sstream>
#include "jobCacheOperation.h"
#include "jnlFile_ex.h"
#include "jnlFileReadOnly_ex.h"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/regex.hpp"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class jnlFileManager {
	  
	  std::string filename;
	  std::ofstream os;
	  std::ifstream is;
	  bool readonly, isempty;
          boost::recursive_mutex mutexJnlFile;
          boost::recursive_mutex::scoped_lock mutexJnlFileLock;

	  unsigned long long savedoff;

          static const boost::regex match;

	public:
	  jnlFileManager(const std::string&) throw (glite::wms::ice::util::jnlFile_ex&);
	  virtual ~jnlFileManager() throw () {
	    if(readonly) is.close();
	    else os.close();
	  }
	  
	  void truncate() throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);

          /**
           * Logs an operation with the given parameters on the journal
           * manager. The journal must <em>not</em> be in read-only
           * mode before calling this method. 
           *
           * @param op the operation to log 
           * @param params the string representing the parameters of that operation
           */
	  void logOperation(glite::wms::ice::util::operation op, 
                            const std::string& params) throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);

          /**
           * Gets the next operation from the journal file. If no
           * operations are logged on the journal (that is, we reached
           * the end of file) this method returns false.
           * 
           * @param op the code of the next operation
           * @param params the parameters of the next operation
           * @return true iff the line was succesfully fetched and parsed from the journal fila
           */
          bool getOperation( glite::wms::ice::util::operation& op,
                             std::string& params ) throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);

          /**
           * Set/unset the readonly mode flag on the journal.
           *
           * @param b the new value of the readonly flag.
           */
	  void readonly_mode(const bool& b) throw (glite::wms::ice::util::jnlFile_ex&);
	  void rewind() throw (glite::wms::ice::util::jnlFile_ex&);
	  void lock(void);
	  void unlock(void);
	};

      }
    }
  }
}
#endif
	
