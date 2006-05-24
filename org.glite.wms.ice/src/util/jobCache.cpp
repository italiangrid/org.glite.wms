
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

jobCache* jobCache::s_instance = 0;
string jobCache::s_jnlFile = DEFAULT_JNLFILE;
string jobCache::s_snapFile = DEFAULT_SNAPFILE;
boost::recursive_mutex jobCache::mutex;

// 
// Inner class definitions
//

//______________________________________________________________________________
jobCache::jobCacheTable::jobCacheTable( ) :
    m_jobs( ),
    m_cidMap( ),
    m_gidMap( )
{
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator jobCache::jobCacheTable::putJob( const CreamJob& c )
{
    // Note: the GridJobID of a job MUST always be defined; the creamJobId
    // could be initially empty, so we need to check this.

    _gidMapType::iterator it = m_gidMap.find( c.getGridJobID() );
    jobCacheTable::iterator pos;
    if ( it == m_gidMap.end() ) {
        // Inserts a new job
        m_jobs.push_back( c );
        pos = --m_jobs.end();
    } else {
        pos = it->second;
        // overwrites existing job
        *pos = c;
    }
    m_gidMap.insert( make_pair( c.getGridJobID(), pos ) );
    if ( !c.getJobID().empty() ) {
        m_cidMap.insert( make_pair( c.getJobID(), pos ) );
    }
    return pos;
}

//______________________________________________________________________________
void jobCache::jobCacheTable::delJob( const jobCacheTable::iterator& pos )
{
    if ( pos == end() )
        return;

    // Removes from the gidMap
    m_gidMap.erase( pos->getGridJobID() );

    // Removes from the cidMap
    if ( !pos->getJobID().empty() ) {
        m_cidMap.erase( pos->getJobID() );
    }

    // Finally, removes from the joblist
    m_jobs.erase( pos );
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::findJobByGID( const std::string& gid )
{
    _gidMapType::iterator it = m_gidMap.find( gid );

    if ( it != m_gidMap.end() ) {
        return it->second;
    } else {
        return m_jobs.end();
    }
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::findJobByCID( const std::string& cid )
{
    _cidMapType::iterator it = m_cidMap.find( cid );

    if ( it != m_cidMap.end() ) {
        return it->second;
    } else {
        return m_jobs.end();
    }
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::begin( void )
{
    return m_jobs.begin( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::end( void )
{
    return m_jobs.end( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::const_iterator 
jobCache::jobCacheTable::begin( void ) const
{
    return m_jobs.begin( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::const_iterator 
jobCache::jobCacheTable::end( void ) const
{
    return m_jobs.end( );
}

//______________________________________________________________________________
jobCache* jobCache::getInstance() throw(jnlFile_ex&, ClassadSyntax_ex&) {
    if(!s_instance)
        s_instance = new jobCache( ); // can throw jnlFile_ex or
    // ClassadSyntax_ex
    return s_instance;
}

//______________________________________________________________________________
jobCache::jobCache( void )
  throw(jnlFile_ex&, ClassadSyntax_ex&) 
    : m_log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
      m_jobs( ),
      m_operation_counter(0)
{ 
    jnlFileManager *man = new jnlFileManager( s_jnlFile );
    m_jnlMgr.reset( man );
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
  if(-1==::stat(s_snapFile.c_str(), &stat_buf)) {
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
  ifstream tmpIs(s_snapFile.c_str(), ios::in );
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
    m_jobs.putJob( cj ); // update in-memory data structure
    m_operation_counter++;
  }
  tmpIs.close(); // redundant: ifstream's dtor also closes file
}

//______________________________________________________________________________
void jobCache::loadJournal(void) 
  throw(jnlFile_ex&, ClassadSyntax_ex&, jnlFileReadOnly_ex&)
{

    operation op;
    string param;
    
    while ( m_jnlMgr->getOperation(op, param) ) {
        
        CreamJob cj( param );
        
        switch ( op ) {
        case PUT:
            m_jobs.putJob( cj );
            m_operation_counter++;
            break;
        case ERASE:
            m_jobs.delJob( lookupByGridJobID( cj.getGridJobID() ) );
            m_operation_counter++;
            break;
        default:
	  CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR,
					"Unknown operation parsing the journal"));
        }
    }
}

//______________________________________________________________________________
jobCache::iterator jobCache::put(const CreamJob& cj) throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
    string param= cj.serialize();
//    std::cout << std::endl << "*** ALVISE DEBUG: PUTTING job [" << cj.getJobID() << "] - status ["<<cj.getStatus()<<"]"<<std::endl;
    logOperation( PUT, param );
    return m_jobs.putJob( cj );
}

//______________________________________________________________________________
void jobCache::print(ostream& os) {
    jobCacheTable::const_iterator it;
    for ( it=m_jobs.begin(); it!=m_jobs.end(); it++ ) {
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

    string tmpSnapFile = boost::str( boost::format("%1%.tmp.%2%") % s_snapFile % ::getpid() );

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
        
	CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::INFO,
				      "jobCache::dump() - Dumping snapshot file"));
        
        for (it=m_jobs.begin(); it != m_jobs.end(); it++ ) {
            
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
    
    if(-1==::rename(tmpSnapFile.c_str(), s_snapFile.c_str()))
        {
            string err = string("Error renaming temp snapshot file into snapshot file")+
                strerror(errno);
            
	    CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR,
					  string("jobCache::dump() - Could't rename snapshot file: ")+err));
            
            throw jnlFile_ex(err);
        }
}

//-----------------------------------------------------------------------------
void jobCache::logOperation( const operation& op, const std::string& param )
{
    /**
     * Updates journal file
     *
     */
    m_jnlMgr->readonly_mode(false);
    m_jnlMgr->logOperation(op, param); // can raise jnlFile_ex and jnlFileReadOnly_ex

    /**
     * checks if cache-dump and journal-truncation are needed
     */
    m_operation_counter++;
    int max_op_cntr;
    {
      boost::recursive_mutex::scoped_lock M( iceConfManager::mutex );
      max_op_cntr = iceConfManager::getInstance()->getMaxJobCacheOperationBeforeDump();
    }

    if( m_operation_counter >= max_op_cntr )
    {
      m_operation_counter = 0;
      try {
	this->dump(); // can raise a jnlFile_ex
	m_jnlMgr->truncate(); // can raise a jnlFile_ex of jnlFileReadOnly_ex
      } catch(jnlFile_ex& ex) {
	CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR,
				      string("jobCache::logOperation() - ")
				      + ex.what()));
	exit(1);
      } catch(jnlFileReadOnly_ex& ex) {
	CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR,
				      ex.what()));
	exit(1);
      } catch(std::exception& ex) {
	CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR,
				      string("jobCache::logOperation() - ")
				      + ex.what()));
	exit(1);
      } catch(...) {
	CREAM_SAFE_LOG(m_log_dev->log(log4cpp::Priority::ERROR,
				      string("jobCache::logOperation() - Catched unknown exception")));
	exit(1);
      }
    }
}

jobCache::iterator jobCache::lookupByCreamJobID( const string& creamJID )
{
    return m_jobs.findJobByCID( creamJID );
}

jobCache::iterator jobCache::lookupByGridJobID( const string& gridJID )
{
    return m_jobs.findJobByGID( gridJID );
}

jobCache::iterator jobCache::erase( jobCache::iterator& it )
{
    if ( it == m_jobs.end() ) {
        return it;
    }

//    std::cout << "*** ALVISE DEBUG: ERASING job ["<<it->getJobID() << "] status ["<<it->getStatus()<<"]"<<std::endl;

    jobCache::iterator result = it;
    result++; // advance iterator
    string to_string = it->serialize();
    // job found, log operation and remove
    logOperation( ERASE, to_string );
    m_jobs.delJob( it );    
    return result;
}
