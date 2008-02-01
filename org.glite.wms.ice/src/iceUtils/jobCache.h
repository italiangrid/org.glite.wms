/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE job cache
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_JOBCACHE_H
#define GLITE_WMS_ICE_UTIL_JOBCACHE_H


// ICE stuff
#include "jnlFile_ex.h"
#include "jobCacheOperation.h"
#include "elementNotFound_ex.h"
#include "creamJob.h"
#include "jobDbManager.h"
#include "jobCacheIterator.h"

// GLite stuff
#include "ClassadSyntax_ex.h"

// STL stuff
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <ostream>

// Boost stuff
#include "boost/thread/recursive_mutex.hpp"
#include "boost/scoped_ptr.hpp"

#define DEFAULT_PERSIST_DIR "/tmp/ice_persist_dir"

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
          
          //! jobCache data structure, which holds informations about jobs known to ICE
	class jobCache {

	private:
            static jobCache *s_instance; ///< The singleton instance of the jobCache class
	    static bool s_recoverable_db;
	    // static bool s_auto_purge_log;
 	    static bool s_read_only;

	    static std::string s_persist_dir; ///< The name of the directory used to save persistency information
 
          /**
           * This class represents a sort of "double-keyed hash
           * table", that is, a hash table where each element can be
           * indexed by two different (both unique) keys. Elements
           * stored in this table are of type CreamJob; a
           * jobCacheTable provides methods to put a single job, and
           * retrieve a job with a given Cream jobID or a given Grid
           * jobID. Both these keys are strings.
           *
           * This class should be used by methods of the jobCache
           * class <em>only</em>. The implemeentation (and even only
           * the existence) of the jobCacheTable should be kept opaque
           * to the user.
           */

	  log4cpp::Category* m_log_dev;

	  std::set<std::string> m_GridJobIDSet;

	  void load( void ) throw();
	  
	  boost::scoped_ptr< glite::wms::ice::util::jobDbManager > m_dbMgr;
	  
	protected:
	  jobCache( );

	public:
	  
	  typedef jobCacheIterator iterator;

          static boost::recursive_mutex mutex; ///< Lock exported to users of the jobCache. This is used to handle client-side mutual exclusion to the jobCache.


          /**
           * Gets the singleton instance of this class. Prior to
           * calling this method, the user is responsible to set the
           * path files with the setJournalFile() and
           * setSnapshotFile() methods. If these methods are not used,
           * then the default snapshot and journal file name will be
           * used.
           *
           * @return the singleton instance of this class
           */ 
	  static jobCache* getInstance() throw();

	  // Call this once and before invokation of getInstance()
	  static void setRecoverableDb( const bool recover ) { s_recoverable_db=recover; }
	  static void setReadOnly( const bool rdonly ) { s_read_only = rdonly; }

          /**
           * Changes the directory containing the persistency
           * information. This method, if used at all, must be called
           * _before_ the first invokation of the getInstance()
           * method.
           *
           * @param dir the directory containing the database of persistency
           */
	  static void setPersistDirectory(const std::string& dir) { s_persist_dir = dir; }  

          /**
           * Destructor.
           */
	  virtual ~jobCache();

          // Modifiers

          /**
           * Inserts a job on the jobCache; it is possible to insert a
           * job which is already in the cache: in this case the old
           * job is replaced by the new one. All operations are logged
           * on the journal file. Jobs are uniquely identified by their
           * grid IDs. Two jobs are considered the same if they have
           * the same Grid job ID.
           *
           * @param c the job to insert into the cache. Job c may
           * already be in the cache; in this case the old job is
           * overwritten
           * @return an iterator to the inserted job
           */
	  iterator put(const CreamJob& c );

          /**
           * Looks up a job by its cream Job ID.
           *
           * @param id the Cream job ID of the job to look for 
           *
           * @return an iterator to the creamJob object with the given
           * id. end() if no job has been found.
           */
	  iterator lookupByCompleteCreamJobID(const std::string& id) throw();

          /**
           * Looks up a job by its grid Job ID
           *
           * @param id the grid job ID of the job to look for 
           *
           * @return an iterator to the creamJob object with the given
           * id. end() if no job has been found.
           */
	  iterator lookupByGridJobID(const std::string& id) throw();

          /**
           * Removes a job from the cache
           *
           * @param it the iterator pointing to the job to remove. If
           * it == end(), no job is removed (this method does
           * nothing).
           *
           * @return an iterator to the element immediately following
           * the one being removed; end() if it==end().
           */
          iterator erase( iterator it );


          // Accessors used to expose jobCacheTable iterator methods

          /**
           * Returns an iterator to the first job in the jobCache
           *
           * @return an iterator to the first job
           */
          iterator begin( void );
          
          /**
           * Returns an iterator to the end of the jobCache
           *
           * @return an iterator to the end of the jobCache
           */
          iterator end( void );
          
	  jobDbManager* getDbManager( void ) const throw() {
	    return m_dbMgr.get();
	  }

	};
      }
    }
  }
}

#endif
