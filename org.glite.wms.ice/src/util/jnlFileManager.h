
#ifndef __GLITE_WMS_ICE_UTIL_JNLFILEMANAGER_H__
#define __GLITE_WMS_ICE_UTIL_JNLFILEMANAGER_H__

#include <string>
#include <exception>
#include <fstream>
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

	//!  The journal manager for job cache persistency
	/*!
	  jnlFileManager is used only by the jobCache singleton
	  and its purpose is simply to manage the persistency of
	  all jobs in memory. jnlFileManager manages a journal
	  file. The journal file contains all the
	  operations performed on the jobCache's internal cache (basically insert or remove). Through
	  jnlFileManager the jobCache singleton can recover exactly the internal status
	  of its std::map after a crash by reading the snapshot file 
	  (handled by the jobCache itself) and then applying 
	  all the operations described in the journal file.
	*/
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
	  //! jnlFileManager Constructor
	  /*! 
	    Creates a jnlFileManager object.
	    \param jnlFile a string the specify the path and name of the journal file.
	    \throw jnlFile_ex& if the journal file couldn't be open or some other access error occurred
	   */
	  jnlFileManager(const std::string& jnlFile) throw (glite::wms::ice::util::jnlFile_ex&);
	  virtual ~jnlFileManager() throw () {
	    if(readonly) is.close();
	    else os.close();
	  }
	  
	  //! Truncate the journal file
	  /*!
	    \throw jnlFile_ex& if the truncate operation failed
	    \throw jnlFileReadOnly_ex& if the journal is in read only mode
	    \sa readonly_mode()
	  */
	  void truncate() throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);

	  //! Logs a jobCache operation
          /*!
            Logs an operation with the given parameters on the journal
            file. The journal must <em>not</em> be in read-only
            mode before calling this method. 
	    
            \param op the operation to log (defined in jobCacheOperation.h); the only two possible operation can be "put a job in cache" and "remove a job from cache"
            \param params the string representing the parameters of that operation (actually a deserialized form a a job; see CreamJob.h for job description)
	    \throw jnlFile_ex& if some I/O error occurred during writing
	    \throw jnlFileReadOnly_ex& if the journal file is in read only mode
	    \sa CreamJob, readonly_mode()
           */
	  void logOperation(glite::wms::ice::util::operation op, 
                            const std::string& params) throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);

	  //! Gets next operation from journal file
          /*!
            Gets the next operation from the journal file. If no
            operations are logged on the journal (that is, we reached
            the end of file) this method returns false.
            Before to use this method the journal file must be put in read only mode by calling read_only(true)
            \param op the variable that will contain the code of the next operation (the codes are described in jobCacheOperation.h)
            \param params a string that will contain the deserialized form of the job
            \return true iff the line was succesfully fetched and parsed from the journal fila
	    \throw jnlFile_ex& if some I/O error occurred during reading
	    \throw jnlFileReadOnly_ex& if the journal file is NOT in read only mode
	    \sa CreamJob, readonly_mode()
           */
          bool getOperation( glite::wms::ice::util::operation& op,
                             std::string& params ) throw (glite::wms::ice::util::jnlFile_ex&, glite::wms::ice::util::jnlFileReadOnly_ex&);

	  //! Set/unset the readonly mode flag on the journal.
          /*!
            \param b the new value of the readonly flag.
	    \throw jnlFile_ex& if some I/O error occured during operation (this operation implies that the file is closed and re-opened in readonly mode)
           */
	  void readonly_mode(const bool& b) throw (glite::wms::ice::util::jnlFile_ex&);
	  
	  //! Rewind the journal file
	  /*!
	    each getOperation() call move the file pointer to the next line. rewind()
	    put this pointer back the the journal file origin
	    \throw jnlFile_ex& if the istream::seekg(0) operation failed
	  */
	  void rewind() throw (glite::wms::ice::util::jnlFile_ex&);

	  //! Puts a lock on the journal file; any other operation on it will block until the lock is released
	  void lock(void);
	  
	  //! Removes the lock from the journal file
	  void unlock(void);
	};

      }
    }
  }
}
#endif
	
