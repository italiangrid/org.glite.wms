
// PROJECT INCLUDES
#include "jobCache.h"
#include "jnlFileManager.h"
#include "Mutex.h"
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
#include <sys/types.h> // for ::getpid()
#include <sys/stat.h>
#include <unistd.h> // for ::getpid()
#include <cstdlib>
#include <sys/time.h>
#include <cstring> // for memset(...)
#include <cerrno>
#include <string>
#include <utility> // for make_pair

extern int errno;

using namespace std;

using namespace glite::wms::ice::util;
namespace apiutil = glite::ce::cream_client_api::util;

jobCache* jobCache::_instance = 0;
string jobCache::jnlFile = DEFAULT_JNLFILE;
string jobCache::snapFile = DEFAULT_SNAPFILE;

// classad::ClassAdParser jc_parser;
// classad::ClassAdUnParser jc_unp;

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
  : hash(),
    cream_grid_hash(),
    operation_counter(0)
{ 
  pthread_mutex_init(&mutexSnapFile, NULL);
  pthread_mutex_init(&mutexHash, NULL);
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

  //  cout << "Loading SNAPSHOT..."<<endl;

  ifstream tmpIs(snapFile.c_str(), ios::in );
  /**
   * Loads jobs from snapshot file
   */
  Mutex M(&mutexSnapFile);
  string Buf;
  while(tmpIs.peek() != EOF) {
    getline(tmpIs, Buf, '\n');
    if(tmpIs.fail() || tmpIs.bad()) {
      tmpIs.close(); // redundant: ifstream's dtor also closes file
      throw jnlFile_ex("Error reading snapshot file");
    }

    CreamJob cj = this->unparse(Buf); // can raise a ClassadSyntax_ex
    // hash[cj.getGridJobID()] = cj;

//     cout << "Found in SNAPSHOT job: "<<cj.getGridJobID()<<" -> ("
// 	 << cj.getJobID() << ", " << cj.getStatus() <<")"<<endl;

    map<string, CreamJob>::iterator it = hash.find( cj.getGridJobID() );
    if( it != hash.end() )
      hash.erase( it );
    hash.insert( make_pair( cj.getGridJobID(), cj ) );
    operation_counter++;
    cream_grid_hash[ cj.getJobID() ] = cj.getGridJobID();
  }
  tmpIs.close(); // redundant: ifstream's dtor also closes file
}

//______________________________________________________________________________
void jobCache::loadJournal(void) 
  throw(jnlFile_ex&, ClassadSyntax_ex&, jnlFileReadOnly_ex&)
{
  jnlMgr->readonly_mode(true);

  /**
   * Creating a lockJournalManager means calling jnlMgr->lock().
   * When this object leaves the current scope its dtor calls
   * jnlMgr->unlock();
   */
  lockJournalManager lJ(jnlMgr);

  string line;

  //  cout << "Loading JOURNAL..."<<endl;

  while(jnlMgr->getNextOperation(line)) {

    apiutil::string_manipulation::chomp(line);

    string match = string("^[0-9]+") + OPERATION_SEPARATOR + ".+";
    if(!apiutil::string_manipulation::matches(line, match.c_str()))
      throw jnlFile_ex(string("Bad journal line ")+line);

    string restOfLine;
    operation op = UNKNOWN;
    this->getOperation(line, op, restOfLine);
    
    CreamJob cj = this->unparse(restOfLine); // can raise ClassadSyntax_ex
        


    if(op == PUT) {
      map<string, CreamJob>::iterator it = hash.find( cj.getGridJobID() );
      if(it != hash.end() )
	hash.erase( it );

//       cout << "Inserting from JOURNAL job: "<<cj.getGridJobID()<<" -> ("
// 	 << cj.getJobID() << ", " << cj.getStatus() <<")"<<endl;

      hash.insert( make_pair(cj.getGridJobID(), cj) );
      cream_grid_hash[cj.getJobID()] = cj.getGridJobID();
      operation_counter++;
    }
    
    if(op == ERASE) {
      hash.erase( cj.getGridJobID() );
      cream_grid_hash.erase( cj.getJobID() );
      operation_counter++;
    }
  }
  //  operation_counter += hash.size();
}

//______________________________________________________________________________
void jobCache::put(const CreamJob& cj) throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
  lockJournalManager lJ(jnlMgr); // locks the journal manager
  Mutex M(&mutexHash); // locks the memory cache (the map<>)
  string tmp;
  this->toString(cj, tmp);
  string param = string(OPERATION_SEPARATOR) + tmp;

  /**
   * Updates journal file
   *
   */
  jnlMgr->readonly_mode(false);
  jnlMgr->log(PUT, param); // can raise jnlFile_ex and jnlFileReadOnly_ex

  /**
   * Updates the memory cache
   */
  map<string, CreamJob>::iterator it = hash.find( cj.getGridJobID() );
  if( it != hash.end() )
    hash.erase( it );
  hash.insert( make_pair(cj.getGridJobID(), cj) );
  cream_grid_hash[cj.getJobID()] = cj.getGridJobID();
  /**
   * checks if cache-dump and journal-truncation are needed
   */
  operation_counter++;
  if(operation_counter >= MAX_OPERATION_COUNTER) {
    //cout << "Dumping jobCache snapshot and truncating journal file"<<endl;
    try {
      this->dump(); // can raise a jnlFile_ex
    } catch(std::exception& ex) {
      cerr << "dump raised an std::exception: "<<ex.what()<<endl;
      exit(1);
    }
    jnlMgr->truncate(); // can raise a jnlFile_ex of jnlFileReadOnly_ex
    operation_counter = 0;
  }
}

//______________________________________________________________________________
void jobCache::remove_by_grid_jobid(const string& gid)
  throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
  lockJournalManager lJ(jnlMgr);
  Mutex M(&mutexHash);
  /**
   * Updates journal file
   *
   */
  jnlMgr->readonly_mode(false);
//   string cream = hash[gid].getJobID();//jobid;
//   api::job_statuses::job_status status = hash[gid].getStatus();
  string to_string;

  //  CreamJob cj = hash[gid];
  map<string, CreamJob>::iterator it;
  it = hash.find( gid );
  if( it == hash.end() ) return;

  string cid = it->second.getJobID();
  this->toString(it->second, to_string);
  to_string = string(OPERATION_SEPARATOR) + to_string;
  jnlMgr->log(ERASE, to_string); // can raise jnlFile_ex and jnlFileReadOnly_ex
  hash.erase(gid);
  cream_grid_hash.erase(cid);
  
  operation_counter++;
  //cout << "operation_counter="<<operation_counter<<endl;
  try {
    if(operation_counter>=MAX_OPERATION_COUNTER) {
      this->dump(); // can raise a jnlFile_ex
      jnlMgr->truncate(); // can raise a jnlFile_ex
      operation_counter = 0;
    }
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

//______________________________________________________________________________
void jobCache::remove_by_cream_jobid(const string& cid)
  throw (jnlFile_ex&, jnlFileReadOnly_ex&, elementNotFound_ex&)
{
  remove_by_grid_jobid( get_grid_jobid_by_cream_jobid(cid) );
}

//______________________________________________________________________________
string jobCache::get_grid_jobid_by_cream_jobid(const std::string& id) 
  throw (elementNotFound_ex&)
{
  Mutex M(&mutexHash);
  if(cream_grid_hash.find( id ) == cream_grid_hash.end() )
    throw elementNotFound_ex(string("Not found the key ")+id+" in job cache");
  return cream_grid_hash[id];
}

//______________________________________________________________________________
string jobCache::get_cream_jobid_by_grid_jobid(const std::string& id) 
  throw (elementNotFound_ex&)
{
  Mutex M(&mutexHash);
  if( hash.find( id ) == hash.end( ) )
    throw elementNotFound_ex(string("Not found the key ")+id+" in job cache");
  
  return hash.find( id )->second.getJobID();
}

//______________________________________________________________________________
CreamJob jobCache::getJobByCreamJobID(const std::string& cid)
  throw (elementNotFound_ex&)
{
  return hash.find( get_grid_jobid_by_cream_jobid(cid) )->second;
}

//______________________________________________________________________________
CreamJob jobCache::getJobByGridJobID(const std::string& gid)
  throw (elementNotFound_ex&)
{
  if( hash.find( gid ) == hash.end( ) )
    throw elementNotFound_ex(string("Not found the key ")+gid+" in job cache");
  return hash.find( gid )->second;
}

//______________________________________________________________________________
bool jobCache::isFinished_by_grid_jobid(const std::string& gid)
  throw (elementNotFound_ex&)
{
  Mutex M(&mutexHash);
  return api::job_statuses::isFinished(getStatus_by_grid_jobid( gid ));
}

//______________________________________________________________________________
bool jobCache::isFinished_by_cream_jobid(const std::string& cid) 
  throw (elementNotFound_ex&)
{
  Mutex M(&mutexHash);
  return jobCache::isFinished_by_grid_jobid(get_grid_jobid_by_cream_jobid(cid));
}

//______________________________________________________________________________
api::job_statuses::job_status 
jobCache::getStatus_by_grid_jobid(const string& gid)
  throw (elementNotFound_ex&)
{
  Mutex M(&mutexHash);
  if( hash.find( gid ) == hash.end() )
    throw elementNotFound_ex(string("Not found the key ")+gid+" in job cache");
  return hash.find( gid )->second.getStatus();
}

//______________________________________________________________________________
api::job_statuses::job_status 
jobCache::getStatus_by_cream_jobid(const string& cid)
  throw (elementNotFound_ex&)
{
  Mutex M(&mutexHash);
  return getJobByCreamJobID( cid ).getStatus();
}

//______________________________________________________________________________
void jobCache::print(ostream& os) {
  Mutex M(&mutexHash);
  map<string, CreamJob>::iterator it;
  for(it=hash.begin(); it!=hash.end(); it++) {
    os << (*it).first.c_str() << " -> ("
       << (*it).second.getJobID() <<", "<<(*it).second.getStatus()<<")"<<endl;
//     fprintf(
// 	    out, "%s -> ( %s, %d )\n", (*it).first.c_str(), 
// 	    (*it).second.getJobID().c_str(),
// 	    (*it).second.getStatus()
// 	    );
  }
}

//______________________________________________________________________________
void jobCache::dump() throw (jnlFile_ex&)
{
  Mutex M(&mutexSnapFile);
  string tmpSnapFile = snapFile + ".tmp." +
    apiutil::string_manipulation::make_string(::getpid());

  int saveerr = 0;


  if(-1==::unlink(tmpSnapFile.c_str()))
    {
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
    
    map<string, CreamJob>::iterator it;
    
    cout << "Dumping snapshot file"<<endl;
    
    for(it=hash.begin(); it!=hash.end(); it++) {

      string param;
      this->toString((*it).second, param);

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
  //ad = classad::ClassAdParser.ParseClassAd(Buf);
  
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

  //  cout << "jobCache::unparse - gid="<<gid<<endl;

  // cout << "jobCache::unparse - st="<<st<<endl;

  //api::job_statuses::job_status stNum = api::job_statuses::getStatusNum(st);
  api::job_statuses::job_status stNum;
  try {
    stNum = 
      (api::job_statuses::job_status)apiutil::string_manipulation::string2int(st);
  } catch(apiutil::NumericException& ex) {
    cerr << ex.what()<<endl;
    exit(1);
  }

  //cout << "jobCache::unparse - stNum="<<stNum<<endl;

  try {
    return CreamJob(jdl, cid, gid, stNum);
  } catch(ClassadSyntax_ex& ex) {
    throw ClassadSyntax_ex(string("Error creating a creamJob: ")+ex.what());
  }
}

//______________________________________________________________________________
void jobCache::getOperation(const string& S,
			    operation& op,
			    string& restOfLine) 
{
  unsigned int pos = S.find(OPERATION_SEPARATOR);
  op = UNKNOWN;
  restOfLine = "";
  if(pos == string::npos) return;

  restOfLine = S.substr(pos+1, S.length() - pos);
  string Op = S.substr(0, pos);
  if(Op == apiutil::string_manipulation::make_string((int)PUT))
    op = PUT;
  if(Op == apiutil::string_manipulation::make_string((int)ERASE))
    op = ERASE;
}

//______________________________________________________________________________
void jobCache::getActiveCreamJobIDs(vector<string>& target) 
{
  map<string, CreamJob>::const_iterator it;
  for( it = hash.begin(); it != hash.end(); it++) {
//     if( api::job_statuses::isFinished((*it).second.getStatus()) ) 
//       continue;
    target.push_back((*it).second.getJobID());
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
  this->updateStatusByGridJobID( get_grid_jobid_by_cream_jobid( cid ),
				 status );
}

//______________________________________________________________________________
void jobCache::updateStatusByGridJobID(const std::string& gid, 
				       const api::job_statuses::job_status& status) 
  throw(elementNotFound_ex&)
{
  lockJournalManager lJ(jnlMgr); // locks the journal manager
  Mutex M(&mutexHash); // locks the memory cache (the map<>)

  map<string, CreamJob>::iterator it = hash.find( gid );

  if( it == hash.end() )
    throw elementNotFound_ex(string("Not found the key ")+gid+" in job cache");

  string tmp;
  it->second.setStatus( status );
  this->toString( it->second, tmp );
  string param = string(OPERATION_SEPARATOR) + tmp;

  /**
   * Updates journal file
   *
   */
  jnlMgr->readonly_mode(false);
  jnlMgr->log(PUT, param); // can raise jnlFile_ex and jnlFileReadOnly_ex

  /**
   * checks if cache-dump and journal-truncation are needed
   */
  operation_counter++;

  //cout << "operation_counter="<<operation_counter<<endl;

  if(operation_counter >= MAX_OPERATION_COUNTER) {
    //cout << "Dumping jobCache snapshot and truncating journal file"<<endl;
    try {
      this->dump(); // can raise a jnlFile_ex
      jnlMgr->truncate(); // can raise a jnlFile_ex of jnlFileReadOnly_ex
      operation_counter = 0;
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
