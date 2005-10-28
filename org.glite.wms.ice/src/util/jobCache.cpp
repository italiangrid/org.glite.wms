
// PROJECT INCLUDES
#include "jobCache.h"
#include "jnlFileManager.h"
#include "Mutex.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
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

extern int errno;

using namespace std;

using namespace glite::wms::ice::util;
namespace apiutil = glite::ce::cream_client_api::util;

jobCache* jobCache::_instance = 0;
string jobCache::jnlFile = DEFAULT_JNLFILE;
string jobCache::snapFile = DEFAULT_SNAPFILE;

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

    Job J = this->unparse(Buf);
    hash[J.grid_jobid] = J.cream_job;
    cream_grid_hash[J.cream_job.jobid] = J.grid_jobid;
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
  while(jnlMgr->getNextOperation(line)) {
    apiutil::string_manipulation::chomp(line);
    string match = string("^[0-9]+") + OPERATION_SEPARATOR + ".+";
    if(!apiutil::string_manipulation::matches(line, match.c_str()))
      throw jnlFile_ex(string("Bad journal line ")+line);

    string restOfLine;
    operation op = UNKNOWN;
    this->getOperation(line, op, restOfLine);
    
    if(op == PUT) {
      Job J = this->unparse(restOfLine); // can raise ClassadSyntax_ex
      hash[J.grid_jobid] = J.cream_job;
      cream_grid_hash[J.cream_job.jobid] = J.grid_jobid;
    }
    
    if(op == ERASE) {
      Job J = this->unparse(restOfLine); // can raise ClassadSyntax_ex
      hash.erase(J.grid_jobid);
      cream_grid_hash.erase(J.cream_job.jobid);
    }
  }
}

//______________________________________________________________________________
void jobCache::put(const string& grid, 
		   const string& cream, 
		   const api::job_statuses::job_status& status)
  throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
  lockJournalManager lJ(jnlMgr); // locks the journal manager
  Mutex M(&mutexHash); // locks the memory cache (the map<>)
  string param = string(OPERATION_SEPARATOR) + makeClassad(grid, cream, status);
  
  /**
   * Updates journal file
   *
   */
  
  jnlMgr->readonly_mode(false);
  jnlMgr->log(PUT, param); // can raise jnlFile_ex and jnlFileReadOnly_ex

  /**
   * Updates the memory cache
   */
  hash[grid] = CreamJob(cream, status);
  cream_grid_hash[cream] = grid;
  /**
   * checks if cache-dump and journal-truncation are needed
   */
  operation_counter++;
  if(operation_counter>=MAX_OPERATION_COUNTER) {
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
  string cream = hash[gid].jobid;
  api::job_statuses::job_status status = hash[gid].status;
  jnlMgr->log(ERASE, makeClassad(gid, cream, status)); // can raise jnlFile_ex and jnlFileReadOnly_ex

  CreamJob cj = hash[gid];
  hash.erase(gid);
  cream_grid_hash.erase(cj.jobid);
  
  operation_counter++;
  try {
    if(operation_counter>=MAX_OPERATION_COUNTER) {
      cout << "Dumping snapshot and truncating journal file"<<endl;
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
  throw (jnlFile_ex&, jnlFileReadOnly_ex&)
{
  string gid = get_grid_jobid_by_cream_jobid(cid);
  remove_by_grid_jobid(gid);
  //  cream_grid_hash.erase(cid);
}

//______________________________________________________________________________
string jobCache::get_grid_jobid_by_cream_jobid(const std::string& id) 
{
  Mutex M(&mutexHash);
  return cream_grid_hash[id];
}

//______________________________________________________________________________
string jobCache::get_cream_jobid_by_grid_jobid(const std::string& id) 
{
  Mutex M(&mutexHash);
  return hash[id].jobid;
}

//______________________________________________________________________________
bool jobCache::isFinished_by_grid_jobid(const std::string& id) 
{
  Mutex M(&mutexHash);
  return api::job_statuses::isFinished(hash[id].status);
}

//______________________________________________________________________________
bool jobCache::isFinished_by_cream_jobid(const std::string& id) 
{
  Mutex M(&mutexHash);
  return jobCache::isFinished_by_grid_jobid(get_grid_jobid_by_cream_jobid(id));
}

//______________________________________________________________________________
api::job_statuses::job_status 
jobCache::getStatus_by_grid_jobid(const string& id)
{
  Mutex M(&mutexHash);
  return hash[id].status;
}

//______________________________________________________________________________
api::job_statuses::job_status 
jobCache::getStatus_by_cream_jobid(const string& id)
{
  Mutex M(&mutexHash);
  return hash[get_grid_jobid_by_cream_jobid(id)].status;
}

//______________________________________________________________________________
void jobCache::print(FILE* out) {
  Mutex M(&mutexHash);
  map<string, CreamJob>::iterator it;
  for(it=hash.begin(); it!=hash.end(); it++) {
    fprintf(out, "%s -> ( %s, %d )\n", (*it).first.c_str(), 
	    (*it).second.jobid.c_str(),
	    (*it).second.status
	    /*api::job_statuses::job_status_str[(*it).second.status]*/);
  }
}

//______________________________________________________________________________
void jobCache::dump() throw (jnlFile_ex&)
{
  Mutex M(&mutexSnapFile);
  string tmpSnapFile = snapFile + ".tmp." +
    apiutil::string_manipulation::make_string(::getpid());

  int saveerr = 0;

  //  cout << "Unlinking...."<<endl;

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

  //  cout << "Step #..."<<endl;

  {
    ofstream tmpOs(tmpSnapFile.c_str(), ios::out);
    if ((void*)tmpOs == 0) 
      throw jnlFile_ex("Error opening temp snapshot file");
    
    //    cout << "stream aperto!"<<endl;

    map<string, CreamJob>::iterator it;
    for(it=hash.begin(); it!=hash.end(); it++) {
      //      cout << "Dumping snapshot file and:"<<endl;
      //      cout << "it.first="<<(*it).first 
// 	   << " - it.second.jobid="<<(*it).second.jobid 
// 	   << " - it.second.status="<<(*it).second.status<<endl;
      string param = this->makeClassad((*it).first, 
				       (*it).second.jobid, 
				       (*it).second.status);
      
      //      cout << "param="<<param<<endl;

      try{tmpOs << param << endl;}
      catch(std::exception&ex) {
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
  //  cout << "Renaming..."<<endl;
  if(-1==::rename(tmpSnapFile.c_str(), snapFile.c_str()))
    {
      string err = string("Error renaming temp snapshot file into snapshot file")+
	strerror(errno);

      cerr << "error renaming: "<<err<<endl;

      throw jnlFile_ex(err);
    }
}

//______________________________________________________________________________
Job jobCache::unparse(const string& Buf) throw(ClassadSyntax_ex&) 
{
  classad::ClassAd *ad, *subad;
  classad::ExprTree *jobtree;
  string jobExpr, gid,cid,st;
  
  ad = parser.ParseClassAd(Buf);
  
  if(!ad)
    throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire classad");

  if((jobtree=ad->Lookup("Job"))!=NULL) {    
    unp.Unparse(jobExpr, jobtree);
    subad = parser.ParseClassAd(jobExpr);
    if ( subad == NULL )
      throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for 'Job'");

    if(subad->Lookup("grid_jobid") && subad->Lookup("cream_jobid") &&
       subad->Lookup("status")) {
      unp.Unparse(gid, subad->Lookup("grid_jobid"));
      unp.Unparse(cid, subad->Lookup("cream_jobid"));
      unp.Unparse(st,  subad->Lookup("status"));
    } else {
      throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for 'grid_jobid'/'cream_jobid'/'status'");
    }
  }

  apiutil::string_manipulation::trim(cid, '"');
  apiutil::string_manipulation::trim(st, '"');
  
  CreamJob cj = CreamJob(cid, api::job_statuses::getStatusNum(st));
  Job J = {gid, cj};
  return J;
}

//______________________________________________________________________________
string jobCache::makeClassad(const string& grid, 
			     const string& cream, 
			     const api::job_statuses::job_status& status)  
 {
   string _grid = grid;
   string _cream= cream;
   apiutil::string_manipulation::trim(_grid, "\"");
   apiutil::string_manipulation::trim(_cream, "\"");

   //cout << "trimmed _grid="<<_grid<<" - _cream="<<_cream<<endl;

   string expr = string("[Job=[grid_jobid=\"") + _grid + "\";cream_jobid=\"" + 
     _cream + "\";status=\"" 
     + apiutil::string_manipulation::make_string((int)status) 
     + "\"]]";

   return expr;
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
// int main(int argc, char *argv[]) {

//   jobCache *cache;
//   try {
//     cache = new jobCache(argv[1], argv[2]);
//     cache->print(stdout);
//   } catch(std::exception& ex) {
//     cerr << ex.what()<<endl;
//   }

//   try{cache->dump();}
//   catch(std::exception ex) {
//     cerr << ex.what()<<endl;
//   }
// }
