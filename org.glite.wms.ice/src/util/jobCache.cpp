
// PROJECT INCLUDES
#include "jobCache.h"
#include "jnlFileManager.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "classad_distribution.h"
#include "source.h" // classad's stuff
#include "sink.h"   // classad's stuff

// System INCLUDES
#include <iostream>
#include <sstream>
#include <exception>
#include <cstdio> // for ::rename(...)
#include <string>
#include <utility> // for make_pair

extern int errno;

using namespace std;

using namespace glite::wms::ice::util;
namespace apiutil = glite::ce::cream_client_api::util;

jobCache* jobCache::_instance = 0;
string jobCache::jnlFile = DEFAULT_JNLFILE;
string jobCache::snapFile = DEFAULT_SNAPFILE;

// 
// Inner class definitions
//

//-----------------------------------------------------------------------------
jobCache::jobCacheTable::jobCacheTable( ) :
    _jobs( ),
    _cidMap( ),
    _gidMap( )
{

}

//-----------------------------------------------------------------------------
void jobCache::jobCacheTable::putJob( const CreamJob& c )
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
    if ( c.getJobID().compare("") ) {
        _cidMap.insert( make_pair( c.getJobID(), pos ) );
    }
}

//-----------------------------------------------------------------------------
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
        if ( c.getJobID().compare("") ) {
            _cidMap.erase( c.getJobID() );
        }
    }
}

//-----------------------------------------------------------------------------
jobCache::jobCacheTable::iterator jobCache::jobCacheTable::findJobByGID( const std::string& gid )
{
    _gidMapType::iterator it = _gidMap.find( gid );

    if ( it != _gidMap.end() ) {
        return it->second;
    } else {
        return _jobs.end();
    }
}

//-----------------------------------------------------------------------------
jobCache::jobCacheTable::iterator jobCache::jobCacheTable::findJobByCID( const std::string& cid )
{
    _cidMapType::iterator it = _cidMap.find( cid );

    if ( it != _cidMap.end() ) {
        return it->second;
    } else {
        return _jobs.end();
    }
}

jobCache::jobCacheTable::iterator jobCache::jobCacheTable::begin( void )
{
    return _jobs.begin( );
}

jobCache::jobCacheTable::iterator jobCache::jobCacheTable::end( void )
{
    return _jobs.end( );
}

jobCache::jobCacheTable::const_iterator jobCache::jobCacheTable::begin( void ) const
{
    return _jobs.begin( );
}

jobCache::jobCacheTable::const_iterator jobCache::jobCacheTable::end( void ) const
{
    return _jobs.end( );
}

//
// jobCache
//

//______________________________________________________________________________
jobCache* jobCache::getInstance() throw(jnlFile_ex&, ClassadSyntax_ex&) {
  if(!_instance)
    _instance = new jobCache(snapFile, jnlFile); // can throw jnlFile_ex or 
                                                 // ClassadSyntax_ex
  return _instance;
}

//______________________________________________________________________________
jobCache::jobCache(const string& _snapFile,
		   const string& journalFile) 
  throw(jnlFile_ex&, ClassadSyntax_ex&) 
    : jobCacheMutex( ),
      theMutex( ),
      theLock( theMutex, false ),
      _jobs( ),
      operation_counter(0)
{ 
    jnlMgr = new jnlFileManager(jnlFile);
    loadSnapshot();
    loadJournal();
}


//______________________________________________________________________________
jobCache::~jobCache() {
    delete(jnlMgr);
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
      throw jnlFile_ex(string("Error loading snapshot file: ") + strerror(saveerr));

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

    CreamJob cj = this->unparse(Buf); // can raise a ClassadSyntax_ex
    _jobs.putJob( cj ); // update in-memory data structure
    operation_counter++;
  }
  tmpIs.close(); // redundant: ifstream's dtor also closes file
}

//______________________________________________________________________________
void jobCache::loadJournal(void) 
  throw(jnlFile_ex&, ClassadSyntax_ex&, jnlFileReadOnly_ex&)
{
  //jnlMgr->readonly_mode(true);

  operation op;
  string param;

  while ( jnlMgr->getOperation(op, param) ) {

    CreamJob cj = this->unparse(param); // can raise ClassadSyntax_ex
        
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
        cerr << "Unknown operation parsing the journal" << endl;
    }
  }

}

//______________________________________________________________________________
void jobCache::put(const CreamJob& cj) throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); // locks the cache
    string param;
    this->toString(cj, param);
    // string param = string(OPERATION_SEPARATOR) + tmp;

    logOperation( PUT, param );
    _jobs.putJob( cj );
}

//______________________________________________________________________________
void jobCache::remove_by_grid_jobid(const string& gid)
  throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex);
    
    jobCacheTable::iterator it = _jobs.findJobByGID( gid );
    if ( it == _jobs.end() ) {
        return ;
    }

    string to_string;
    // job found, log operation and remove
    this->toString(*it, to_string);
    // to_string = string(OPERATION_SEPARATOR) + to_string;
    logOperation( ERASE, to_string );
    _jobs.delJob( *it );
}

//______________________________________________________________________________
void jobCache::remove_by_cream_jobid(const string& cid)
  throw (jnlFile_ex&, jnlFileReadOnly_ex&, elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex );
    remove_by_grid_jobid( get_grid_jobid_by_cream_jobid(cid) );
}

//______________________________________________________________________________
string jobCache::get_grid_jobid_by_cream_jobid(const std::string& cid) 
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex);
    jobCacheTable::const_iterator it = _jobs.findJobByCID( cid );
    if( it == _jobs.end() )
        throw elementNotFound_ex(string("get_grid_jobid_by_cream_jobid: Not found the CID=")+cid+" in job cache");
    return it->getGridJobID();
}

//______________________________________________________________________________
string jobCache::get_cream_jobid_by_grid_jobid(const std::string& gid) 
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex);
    jobCacheTable::const_iterator it = _jobs.findJobByGID( gid );
    if ( it == _jobs.end() )
        throw elementNotFound_ex(string("get_cream_jobid_by_grid_jobid: Not found the GID=")+gid+" in job cache");

    return it->getJobID();
}

//______________________________________________________________________________
CreamJob jobCache::getJobByCreamJobID(const std::string& cid)
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex);
    jobCacheTable::iterator it = _jobs.findJobByCID( cid );
    if ( it == _jobs.end() )
        throw elementNotFound_ex(string("Job with Cream JobID=")+cid+" not found" );
    return *it;
}

//______________________________________________________________________________
CreamJob jobCache::getJobByGridJobID(const std::string& gid)
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex);
    jobCacheTable::iterator it = _jobs.findJobByGID( gid );
    if ( it == _jobs.end() )
        throw elementNotFound_ex(string("Job with Cream JobID=")+gid+" not found" );
    return *it;
}

//______________________________________________________________________________
bool jobCache::isFinished_by_grid_jobid(const std::string& gid)
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex);
    return api::job_statuses::isFinished(getStatus_by_grid_jobid( gid ));
}

//______________________________________________________________________________
bool jobCache::isFinished_by_cream_jobid(const std::string& cid) 
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex);
    return jobCache::isFinished_by_grid_jobid(get_grid_jobid_by_cream_jobid(cid));
}

//______________________________________________________________________________
api::job_statuses::job_status 
jobCache::getStatus_by_grid_jobid(const string& gid)
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
    jobCacheTable::const_iterator it = _jobs.findJobByGID( gid );
    if ( it == _jobs.end() )
        throw elementNotFound_ex(string("Not found the key ")+gid+" in job cache");
    return it->getStatus();
}

//______________________________________________________________________________
api::job_statuses::job_status 
jobCache::getStatus_by_cream_jobid(const string& cid)
  throw (elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
    jobCacheTable::const_iterator it = _jobs.findJobByCID( cid );
    if ( it == _jobs.end() )
        throw elementNotFound_ex(string("Not found the key ")+cid+" in job cache");
    return it->getStatus();
}

//______________________________________________________________________________
void jobCache::print(ostream& os) {
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
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
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
    string tmpSnapFile = snapFile + ".tmp." +
        apiutil::string_manipulation::make_string(::getpid());

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
        
        cout << "Dumping snapshot file"<<endl;
        
        for (it=_jobs.begin(); it != _jobs.end(); it++ ) {
            
            string param;
            this->toString(*it, param);
            
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
            
            cerr << "error renaming: "<<err<<endl;
            
            throw jnlFile_ex(err);
        }
}

//______________________________________________________________________________
CreamJob jobCache::unparse(const string& Buf) throw(ClassadSyntax_ex&)
{
  classad::ClassAd *ad;
  string jobExpr, gid, cid, st, jdl;
  classad::ClassAdParser parser;
  classad::ClassAdUnParser unp;
  ad = parser.ParseClassAd(Buf);
  
  if(!ad)
    throw ClassadSyntax_ex(string("ClassAd parser returned a NULL pointer parsing entire classad ")+Buf);

  
  if(ad->Lookup("grid_jobid") && ad->Lookup("cream_jobid") &&
     ad->Lookup("status") && ad->Lookup("jdl")) {
    unp.Unparse(gid, ad->Lookup("grid_jobid"));
    unp.Unparse(cid, ad->Lookup("cream_jobid"));
    unp.Unparse(st,  ad->Lookup("status"));
    unp.Unparse(jdl, ad->Lookup("jdl"));
  } else {
    throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for 'grid_jobid' or 'status' or 'jdl' attributes");
  }
    
  apiutil::string_manipulation::trim(cid, '"');
  apiutil::string_manipulation::trim(st, '"');
  apiutil::string_manipulation::trim(gid, '"');
  apiutil::string_manipulation::trim(jdl, '"');

  api::job_statuses::job_status stNum;
  try {
    stNum = 
      (api::job_statuses::job_status)apiutil::string_manipulation::string2int(st);
  } catch(apiutil::NumericException& ex) {
    cerr << ex.what()<<endl;
    exit(1);
  }

  try {
    return CreamJob(jdl, cid, gid, stNum);
  } catch(ClassadSyntax_ex& ex) {
    throw ClassadSyntax_ex(string("Error creating a creamJob: ")+ex.what());
  }
}

//______________________________________________________________________________
void jobCache::getActiveCreamJobIDs(vector<string>& target) 
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
    jobCacheTable::const_iterator it;
    for ( it = _jobs.begin(); it!=_jobs.end(); it++ ) {
        target.push_back( it->getJobID());
    }
}

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

    //cout << "operation_counter="<<operation_counter<<endl;

    if(operation_counter >= MAX_OPERATION_COUNTER) {

        operation_counter = 0;

        try {
            this->dump(); // can raise a jnlFile_ex
            jnlMgr->truncate(); // can raise a jnlFile_ex of jnlFileReadOnly_ex
        } catch(std::exception& ex) {
            cerr << "dump raised an std::exception: "<<ex.what()<<endl;
            exit(1);
        } catch(jnlFile_ex& ex) {
            cerr << ex.what()<<endl;
            exit(1);
        } catch(jnlFileReadOnly_ex& ex) {
            cerr << ex.what()<<endl;
            exit(1);
        } catch(...) {
            cerr << "Something catched!"<<endl;
            exit(1);
        }
    }
}

//______________________________________________________________________________
void jobCache::toString(const CreamJob& cj, string& target)
{
  target = string("[grid_jobid=\"" + cj.getGridJobID() + "\";cream_jobid=\"" 
		  + cj.getJobID() + "\";status=\"" 
		  + apiutil::string_manipulation::make_string((int)cj.getStatus()) 
		  + "\"; jdl=" + cj.getJDL() + "]");
}

//______________________________________________________________________________
void jobCache::updateStatusByCreamJobID(const std::string& cid, 
			      const api::job_statuses::job_status& status) 
  throw(elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
    this->updateStatusByGridJobID( get_grid_jobid_by_cream_jobid( cid ),
                                   status );
}

//______________________________________________________________________________
void jobCache::updateStatusByGridJobID(const std::string& gid, 
				       const api::job_statuses::job_status& status) 
  throw(elementNotFound_ex&)
{
    boost::recursive_mutex::scoped_lock M(jobCacheMutex); 
    jobCacheTable::iterator it = _jobs.findJobByGID( gid );

    if( it == _jobs.end() )
        throw elementNotFound_ex(string("Not found the key ")+gid+" in job cache");

  string param;
  it->setStatus( status );
  this->toString( *it, param );
  logOperation( PUT, param );
}


//-----------------------------------------------------------------------------
void jobCache::lock( void )
{
    theLock.lock( );    
}

//-----------------------------------------------------------------------------
void jobCache::unlock( void )
{
    theLock.unlock( );
}

