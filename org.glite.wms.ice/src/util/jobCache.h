
#ifndef __GLITE_WMS_ICE_UTIL_JOBCACHE_H__
#define __GLITE_WMS_ICE_UTIL_JOBCACHE_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <ostream>
#include "jnlFile_ex.h"
#include "jobCacheOperation.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "jnlFileManager.h"
#include "ClassadSyntax_ex.h"
#include "elementNotFound_ex.h"
#include "creamJob.h" 
#include "boost/thread/recursive_mutex.hpp"

// #define OPERATION_SEPARATOR ":"
#define MAX_OPERATION_COUNTER 10

#define DEFAULT_JNLFILE "/tmp/jobCachePersistFile"
#define DEFAULT_SNAPFILE "/tmp/jobCachePersistFile.snapshot"

namespace api = glite::ce::cream_client_api;

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
            static jobCache *_instance; ///< The singleton instance of the jobCache class
            static std::string jnlFile; ///< The name of the journal file used to log operations on the cache
            static std::string snapFile; ///< The name of the cache snapshot file, used to save snapshots of the cache content

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

	      log4cpp::Category* log_dev;

          class jobCacheTable {
          protected:
              std::list< CreamJob > _jobs; ///< Jobs in the jobCacheTable are stored in a list
              std::map< std::string, std::list< CreamJob >::iterator > _cidMap; ///< This hash table associates Cream jobIDs with job positions in the list _jobs
              std::map< std::string, std::list< CreamJob >::iterator > _gidMap; ///< This hash table associates Grid jobIDs with job positions in the list _jobs

              // Some useful typedefs, for internal use only...
              typedef std::list< CreamJob > _jobsType;
              typedef std::map< std::string, _jobsType::iterator > _cidMapType;
              typedef std::map< std::string, _jobsType::iterator > _gidMapType;

          public:

              // Typedefs
              typedef _jobsType::iterator iterator; ///< The standard (modifying) iterator used for containers
              typedef _jobsType::const_iterator const_iterator; ///< The standard (non-modifying) iterator used for containers

              jobCacheTable();
              virtual ~jobCacheTable() { };             
              
              /**
               * Inserts a job into the in-memory hash table. If the job
               * is already in the table, the old job is overwritten by the new
               * one. This method <em>does not</em> log anything on the journal
               *
               * @param c The job to insert
               */
              void putJob( const CreamJob& c );
              
              /**
               * Removes a job from the in-memory hash table. If the job
               * is not in the table, this method does nothing. This method
               * <em>does not</em> log anything on the journal.
               *
               * @param c The job to remove. If the job does not exist,
               * nothing is done.
               */
              void delJob( const CreamJob& c );
              
              /**
               * Looks up for a job with a given Grid ID
               * (gid). Returns an iterator pointing to the requested
               * job in the _jobs data structure.
               *
               * @param gid the Grid JobID to look for 
               *
               * @return an iterator to the requested job in the _jobs
               * list; returns an iterator pointing to the end of list
               * (_jobs.end()) if the job was not found
               */
              jobCacheTable::iterator findJobByGID( const std::string& gid );
                            
              /**
               * Looks up for a job with a given Cream ID (cid). Returns
               * an iterator pointint to the requested job in the
               * _jobs data structure.
               *
               * @param cid the Cream JobID to look for 
               *
               * @return an iterator to the requested job in the _jobs
               * list; returns an iterator pointing to the end of list
               * (_jobs.end()) if the job was not found
               */
              jobCacheTable::iterator findJobByCID( const std::string& cid );

              /**
               * Returns an iterator to the first job in the jobCacheTable
               *
               * @return an iterator to the first job
               */
              jobCacheTable::iterator begin( void );

              /**
               * Returns an iterator to the end of the jobCacheTable
               *
               * @return an iterator to the end of the jobCacheTable
               */
              jobCacheTable::iterator end( void );

              /**
               * Returns a const_iterator to the first job in the jobCacheTable
               *
               * @return a const_iterator to the first job
               */
              jobCacheTable::const_iterator begin( void ) const;

              /**
               * Returns a const_iterator to the end of the jobCacheTable
               *
               * @return a const_iterator to the end of the jobCacheTable
               */
              jobCacheTable::const_iterator end( void ) const;

          };

          jobCacheTable _jobs; ///< The in-core data structure holding the set of jobs

	  int operation_counter; ///< Number of operations logged on the journal

          /**
           * Loads the journal handled by the jnlMgr object.
           */
	  void loadJournal(void) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&, jnlFileReadOnly_ex&);

          /**
           * Loads the snapshot handled by the jnlMgr object.
           */
	  void loadSnapshot(void) 
	    throw(jnlFile_ex&, ClassadSyntax_ex&);

	  glite::wms::ice::util::jnlFileManager* jnlMgr; ///< The journal manager used to handle access to the journal/snapshot file

	  // CreamJob unparse(const std::string&) throw(ClassadSyntax_ex&);

          /**
           * Serializes a CreamJob object to a string
           *
           * @param c the job to serialize
           * @param s the string where the serialized job will be stored
           */
	  // void toString(const CreamJob& c, std::string& s);

	protected:
	  jobCache( )
	    throw(jnlFile_ex&, ClassadSyntax_ex&);

          /**
           * Logs an operation to the journal manager. The operation
           * count is increased; if the maximum number of operations
           * is reached, the journal file is truncated, and the cache
           * content is stored on disk.
           *
           * @param op the operation to log
           * @param param the parameters to that operation
           */
          void logOperation( const operation& op, const std::string& param );

	public:

          typedef jobCacheTable::iterator iterator; ///< Iterator to the jobCache CREAM jobs. If it is of type jobCache::iterator, then *it is of type creamJob;
          typedef jobCacheTable::const_iterator const_iterator; ///< Const iterator to the jobCache elements. If it is of type jobCache::iterator, then *it is of type creamJob;

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
	  static jobCache* getInstance() throw(jnlFile_ex&, ClassadSyntax_ex&);

          /**
           * Changes the path and filename of the journal file. This method,
           * if used at all, must be called _before_ the first invokation of
           * the getInstance() method.
           *
           * @param jnl the full pathname of the journal file
           */
	  static void setJournalFile(const std::string& jnl) { jnlFile = jnl; }

          /**
           * Changes the path and filename of the snapshot file. This method,
           * if used at all, must be called _before_ the first invokation of
           * the getInstance() method.
           *
           * @param snap the full pathname of the snapshot file
           */
	  static void setSnapshotFile(const std::string& snap) { snapFile = snap; }

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
           */
	  void put(const CreamJob& c ) throw(jnlFile_ex&, jnlFileReadOnly_ex&);

          /**
           * Looks up a job by its cream Job ID.
           *
           * @param id the Cream job ID of the job to look for 
           *
           * @return an iterator to the creamJob object with the given
           * id. end() if no job has been found.
           */
	  iterator lookupByCreamJobID(const std::string& id);

          /**
           * Looks up a job by its grid Job ID
           *
           * @param id the grid job ID of the job to look for 
           *
           * @return an iterator to the creamJob object with the given
           * id. end() if no job has been found.
           */
	  iterator lookupByGridJobID(const std::string& id);

          /**
           * Removes a job from the cache
           *
           * @param it the iterator pointing to the job to remove. If
           * it == end(), no job is removed (this method does
           * nothing).
           */
          void remove( const iterator& it );

          // Accessors used to expose jobCacheTable iterator methods

          /**
           * Returns an iterator to the first job in the jobCache
           *
           * @return an iterator to the first job
           */
          iterator begin( void ) {
              return _jobs.begin();
          };
          
          /**
           * Returns an iterator to the end of the jobCache
           *
           * @return an iterator to the end of the jobCache
           */
          iterator end( void ) {
              return _jobs.end();
          };
          
          /**
           * Returns a const_iterator to the first job in the jobCache
           *
           * @return a const_iterator to the first job 
           */
          const_iterator begin( void ) const {
              return _jobs.begin();
          };
          
          /**
           * Returns a const_iterator to the end of the jobCache
           *
           * @return a const_iterator to the end of the jobCache
           */
          const_iterator end( void ) const {
              return _jobs.end();
          };

          /**
           * Gets the list of all IDs of active CREAM jobs.
           *
           * @param target the vector where CREAM IDs will be stored
           */           
	  void getActiveCreamJobIDs(std::vector<std::string>& target) ;  

        protected:	  
	  void dump(void) throw(jnlFile_ex&);
	  void print(std::ostream&);
	};
      }
    }
  }
}

#endif
