
// PROJECT INCLUDES
#include "jobCache.h"
#include "jnlFileManager.h"
#include "iceConfManager.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
//#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"

#include "classad_distribution.h" // classad's stuff
#include "source.h" // classad's stuff
#include "sink.h"   // classad's stuff

// System INCLUDES
#include <iostream>
#include <sstream>
#include <exception>
#include <cstdio> // for ::rename(...)
#include <string>
#include <sstream>
#include <utility> // for make_pair
#include <ctime>
#include <cstdlib>

extern int errno;

using namespace std;

using namespace glite::wms::ice::util;
namespace apiutil = glite::ce::cream_client_api::util;

jobCache* jobCache::_instance = 0;
string jobCache::jnlFile = DEFAULT_JNLFILE;
string jobCache::snapFile = DEFAULT_SNAPFILE;
boost::recursive_mutex jobCache::mutex;

// 
// Inner class definitions
//

//______________________________________________________________________________
jobCache::jobCacheTable::jobCacheTable( ) :
    _jobs( ),
    _cidMap( ),
    _gidMap( )
{
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator jobCache::jobCacheTable::putJob( const CreamJob& c )
{
    // Note: the GridJobID of a job MUST always be defined; the creamJobId
    // could be initially empty, so we need to check this.

    _gidMapType::iterator it = _gidMap.find( c.getGridJobID() );
    jobCacheTable::iterator pos;
    if ( it == _gidMap.end() ) {
        // Inserts a new job
        _jobs.push_back( c );
        pos = --_jobs.end();
    } else {
        pos = it->second;
        // overwrites existing job
        *pos = c;
    }
    _gidMap.insert( make_pair( c.getGridJobID(), pos ) );
    if ( !c.getJobID().empty() ) {
        _cidMap.insert( make_pair( c.getJobID(), pos ) );
    }
    return pos;
}

//______________________________________________________________________________
void jobCache::jobCacheTable::delJob( const CreamJob& c )
{
    _gidMapType::iterator it = _gidMap.find( c.getGridJobID() );
    if ( it != _gidMap.end() ) {
        // Deletes a new job
        jobCacheTable::iterator pos = it->second;
        // Removes the job from the list
        _jobs.erase( pos );
        // Removes the job from the _gidMap
        _gidMap.erase( it );
        // If necessary, removes the job from the _cidMap
        if ( !c.getJobID().empty() ) {
            _cidMap.erase( c.getJobID() );
        }
    }
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::findJobByGID( const std::string& gid )
{
    _gidMapType::iterator it = _gidMap.find( gid );

    if ( it != _gidMap.end() ) {
        return it->second;
    } else {
        return _jobs.end();
    }
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::findJobByCID( const std::string& cid )
{
    _cidMapType::iterator it = _cidMap.find( cid );

    if ( it != _cidMap.end() ) {
        return it->second;
    } else {
        return _jobs.end();
    }
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::begin( void )
{
    return _jobs.begin( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::end( void )
{
    return _jobs.end( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::const_iterator 
jobCache::jobCacheTable::begin( void ) const
{
    return _jobs.begin( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::const_iterator 
jobCache::jobCacheTable::end( void ) const
{
    return _jobs.end( );
}

//______________________________________________________________________________
jobCache* jobCache::getInstance() throw(jnlFile_ex&, ClassadSyntax_ex&) {
    if(!_instance)
        _instance = new jobCache( ); // can throw jnlFile_ex or
    // ClassadSyntax_ex
    return _instance;
}

//______________________________________________________________________________
jobCache::jobCache( void )
  throw(jnlFile_ex&, ClassadSyntax_ex&) 
    : log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
      _jobs( ),
      operation_counter(0)
{ 
    jnlFileManager *man = new jnlFileManager( jnlFile );
    jnlMgr.reset( man );
    loadSnapshot();
    loadJournal();
}

//______________________________________________________________________________
jobCache::~jobCache( ) 
{

}

//______________________________________________________________________________
void jobCache::loadSnapshot() throw(jnlFile_ex&, ClassadSyntax_ex&)
{
  /**
   * Checks is the snapshot file exists
   */
  struct stat stat_buf;
  int saveerr = 0;
  if(-1==::stat(snapFile.c_str(), &stat_buf)) {
    saveerr = errno;
    if(saveerr==ENOENT) return;
    else
      throw jnlFile_ex(string("Error loading snapshot file: ") + 
		       strerror(saveerr));

  }

  /**
   * The creation of a filestreamOpenManager object means calling
   * is.open(snapFile.c_str(), ios::in).
   * When this object leaves the current scope the is.close() is called
   * by its dtor. This ensure file closing under any circumstance 
   * (unexpected exception raising, forgotting to call ::close() etc.)
   */
  ifstream tmpIs(snapFile.c_str(), ios::in );
  /**
   * Loads jobs from snapshot file
   */
  string Buf;
  while(tmpIs.peek() != EOF) {
    getline(tmpIs, Buf, '\n');
    if(tmpIs.fail() || tmpIs.bad()) {
      tmpIs.close(); // redundant: ifstream's dtor also closes file
      throw jnlFile_ex("Error reading snapshot file");
    }

    // CreamJob cj = this->unparse(Buf); // can raise a ClassadSyntax_ex
    CreamJob cj( Buf ); // can raise ClassadSyntax_ex
    _jobs.putJob( cj ); // update in-memory data structure
    operation_counter++;
  }
  tmpIs.close(); // redundant: ifstream's dtor also closes file
}

//______________________________________________________________________________
void jobCache::loadJournal(void) 
  throw(jnlFile_ex&, ClassadSyntax_ex&, jnlFileReadOnly_ex&)
{

  operation op;
  string param;

  while ( jnlMgr->getOperation(op, param) ) {

      CreamJob cj( param );
        
    switch ( op ) {
    case PUT:
        _jobs.putJob( cj );
        operation_counter++;
        break;
    case ERASE:
        _jobs.delJob( cj );
        operation_counter++;
        break;
    default:
      log_dev->log(log4cpp::Priority::ERROR,
		   "Unknown operation parsing the journal");
    }
  }

}

//______________________________________________________________________________
jobCache::iterator jobCache::put(const CreamJob& cj) throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
    string param= cj.serialize();
    logOperation( PUT, param );
    return _jobs.putJob( cj );
}

//______________________________________________________________________________
void jobCache::print(ostream& os) {
    jobCacheTable::const_iterator it;
    for ( it=_jobs.begin(); it!=_jobs.end(); it++ ) {
        os << "GID=" << it->getGridJobID() 
           << " CID=" << it->getJobID() 
           << " STATUS=" << it->getStatus()
           << endl;
    }
}

//______________________________________________________________________________
void jobCache::dump() throw (jnlFile_ex&)
{
//    string tmpSnapFile = snapFile + ".tmp." +
//        apiutil::string_manipulation::make_string(::getpid());

    string tmpSnapFile = boost::str( boost::format("%1%.tmp.%2%") % snapFile % ::getpid() );

    int saveerr = 0;
        
    if(-1==::unlink(tmpSnapFile.c_str())) {
        saveerr = errno;
        if(saveerr == ENOENT);
        else {
            string err = string("Error removing old snapshot temp file:") +
                strerror(saveerr);
            throw jnlFile_ex(err);
        }
    }
    ofstream ofs;
    {
        ofstream tmpOs(tmpSnapFile.c_str(), ios::out);
        if ((void*)tmpOs == 0) 
            throw jnlFile_ex("Error opening temp snapshot file");
        
        jobCacheTable::iterator it;
        
	log_dev->log(log4cpp::Priority::INFO,
		     "jobCache::dump() - Dumping snapshot file");
        
        for (it=_jobs.begin(); it != _jobs.end(); it++ ) {
            
            string param = it->serialize();
            
            try{tmpOs << param << endl;}
            catch(std::exception& ex) {
                tmpOs.close(); // redundant: ofstream's dtor also closes the file
                throw jnlFile_ex(string("Error dumping cache: ")+ex.what());
            }
            if(tmpOs.fail() || tmpOs.bad() || (!tmpOs.good()) ) {
                tmpOs.close(); // redundant: ofstream's dtor also closes the file
                throw jnlFile_ex("Error after writing into temp snapshot file");
            }
        }
        tmpOs.close(); // redundant: ofstream's dtor also closes the file
    }
    
    if(-1==::rename(tmpSnapFile.c_str(), snapFile.c_str()))
        {
            string err = string("Error renaming temp snapshot file into snapshot file")+
                strerror(errno);
            
	    log_dev->log(log4cpp::Priority::ERROR,
			 string("jobCache::dump() - Could't rename snapshot file: ")+err);
            
            throw jnlFile_ex(err);
        }
}

//______________________________________________________________________________
// void jobCache::getActiveCreamJobIDs(vector<string>& target)
// {
//     // boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
//     jobCacheTable::const_iterator it;
//     for ( it = _jobs.begin(); it!=_jobs.end(); it++ ) {
//         target.push_back( it->getJobID());
//     }
// }

//-----------------------------------------------------------------------------
void jobCache::logOperation( const operation& op, const std::string& param )
{
    /**
     * Updates journal file
     *
     */
    jnlMgr->readonly_mode(false);
    jnlMgr->logOperation(op, param); // can raise jnlFile_ex and jnlFileReadOnly_ex

    /**
     * checks if cache-dump and journal-truncation are needed
     */
    operation_counter++;
    int max_op_cntr;
    {
      boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
      max_op_cntr = iceConfManager::getInstance()->getMaxJobCacheOperationBeforeDump();
    }

    if( operation_counter >= max_op_cntr )
    {
      operation_counter = 0;
      try {
	this->dump(); // can raise a jnlFile_ex
	jnlMgr->truncate(); // can raise a jnlFile_ex of jnlFileReadOnly_ex
      } catch(jnlFile_ex& ex) {
	log_dev->log(log4cpp::Priority::ERROR,
		     string("jobCache::logOperation() - ")
		     + ex.what());
	exit(1);
      } catch(jnlFileReadOnly_ex& ex) {
	log_dev->log(log4cpp::Priority::ERROR,
		     ex.what());
	exit(1);
      } catch(std::exception& ex) {
	log_dev->log(log4cpp::Priority::ERROR,
		     string("jobCache::logOperation() - ")
		     + ex.what());
	exit(1);
      } catch(...) {
	log_dev->log(log4cpp::Priority::ERROR,
		     string("jobCache::logOperation() - Catched unknown exception"));
	exit(1);
      }
    }
}

jobCache::iterator jobCache::lookupByCreamJobID( const string& creamJID )
{
    return _jobs.findJobByCID( creamJID );
}

jobCache::iterator jobCache::lookupByGridJobID( const string& gridJID )
{
    return _jobs.findJobByGID( gridJID );
}

jobCache::iterator jobCache::remove( const jobCache::iterator& it )
{    
    if ( it == _jobs.end() ) {
        return it;
    }

    jobCache::iterator result = it;
    result++; // advance iterator
    string to_string = it->serialize();
    // job found, log operation and remove
    logOperation( ERASE, to_string );
    _jobs.delJob( *it );    
    return result;
}
