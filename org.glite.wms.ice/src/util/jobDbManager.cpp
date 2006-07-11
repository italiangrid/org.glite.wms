
#include "jobDbManager.h"
#include <boost/filesystem/operations.hpp>

namespace iceUtil = glite::wms::ice::util;
using namespace std;

//____________________________________________________________________
iceUtil::jobDbManager::jobDbManager( const string& envHome )
  : m_env(0),
    m_envHome( envHome ), // The directory pointed by
    			  // envHome MUST exist
    m_valid( false ),
    m_invalid_cause(""),
    m_cream_open( false ),
    m_cid_open( false ),
    m_gid_open( false ),
    m_env_open( false )
{
  if( !boost::filesystem::exists( boost::filesystem::path(m_envHome) ) ) {
    m_invalid_cause = string("Path [")+m_envHome+"] doesn't exist";
    return;
  }
  
  if( !boost::filesystem::is_directory( boost::filesystem::path(m_envHome) ) ) {
     m_invalid_cause = string("Path [")+m_envHome+"] does exist but it is not a directory";
     return;
  }


  // FIXME: for now we do not use the flag DB_THREAD because
  // the concurrency protection will be made at higher level
  // (by the client of this class, i.e. by the jobCache object)
  try {
  
    // FIXME: for now we do not use the flag DB_THREAD because
    // the concurrency protection will be made at higher level
    // (by the client of this class, i.e. by the jobCache object)
    m_env.open(m_envHome.c_str(), DB_CREATE |
    		 		  DB_INIT_LOCK |
				  DB_INIT_LOG |
				  DB_INIT_MPOOL |
				  DB_INIT_TXN,
				  0);
    m_env_open = true;
    // -------------------------------------------------------
			  
    m_creamJobDb = new Db(&m_env, 0);
    m_cidDb      = new Db(&m_env, 0);
    m_gidDb      = new Db(&m_env, 0);
    
    // -------------------------------------------------------
    
    // FIXME: for now we do not use the flag DB_THREAD because
    // the concurrency protection will be made at higher level
    // (by the client of this class, i.e. by the jobCache object)
    m_creamJobDb->open(NULL, "cream_job_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT,0);
    m_cream_open = true;
    m_cidDb->open(NULL, "cream_jobid_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT,0);
    m_cid_open = true;
    m_gidDb->open(NULL, "grid_jobid_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT,0);
    m_gid_open = true;
    
    // -------------------------------------------------------
    
  } catch(DbException& dbex) {
    m_invalid_cause = dbex.what();
    return;
  } catch(exception& ex) {
    m_invalid_cause = ex.what();
    return;
  } catch(...) {
    m_invalid_cause = "Unknown error occurred";
    return;
  }
  
  m_valid = true;
}

//____________________________________________________________________
iceUtil::jobDbManager::~jobDbManager() throw()
{
  // FIXME: only one thread at time can call ->close()
  // so these operations should be 'mutex-ed'
  try {
    if(m_creamJobDb && m_cream_open) m_creamJobDb->close(0);
    if(m_cidDb && m_cid_open) m_cidDb->close(0);
    if(m_gidDb && m_gid_open) m_gidDb->close(0);
    if(m_env_open) m_env.close(0);
  } catch(DbException& dbex) {
    cerr << dbex.what() << endl;
    cerr << "An error occurred closing database or environment. database corruption is possibile!" << endl;
  }
  
  delete(m_creamJobDb);
  delete(m_cidDb);
  delete(m_gidDb);
  
}

//____________________________________________________________________
void iceUtil::jobDbManager::put(const string& creamjob, const string& cid, const string& gid)
  throw(iceUtil::JobDbException&)
{
  Dbt cidData( (void *)cid.c_str(), cid.length()+1);
  Dbt data( (void*)creamjob.c_str(), creamjob.length()+1 );
  Dbt gidData( (void*)gid.c_str(), gid.length()+1 );
  
  DbTxn* txn_handler;
  
  try {
    txn_handler = NULL; // this is required because after a commit/abort the
    			  // transaction handler is not valid anymore
			  // a new fresh handler is required for another 
			  // transaction
    m_env.txn_begin(NULL, &txn_handler, 0);
    m_creamJobDb->put( txn_handler, &cidData, &data, 0);
    m_cidDb->put( txn_handler, &cidData, &gidData, 0);
    m_gidDb->put( txn_handler, &gidData, &cidData, 0);
    txn_handler->commit(0);
  } catch(DbException& dbex) {
    txn_handler->abort();
    throw iceUtil::JobDbException( dbex.what() );
  } catch(exception& ex) {
    txn_handler->abort();
    throw iceUtil::JobDbException( ex.what() );
  } catch(...) {
    txn_handler->abort();
    throw iceUtil::JobDbException("Unknown exception catched");
  }
  
}

//____________________________________________________________________
string iceUtil::jobDbManager::getByCid( const string& cid ) 
  throw(iceUtil::JobDbException&)
{
    // FIXME: Going to READ from database.
    // For now the client (jobCache) put a lock
    // but the transaction mechanism should make the 
    // modifications visible to this READ only if the 
    // transaction itself is commited.
    // In future we could put a BDB-lock

  try {
    Dbt data, key( (void*)cid.c_str(), cid.length()+1);
    m_creamJobDb->get( NULL, &key, &data, 0);
    return string( (char*)data.get_data() );
  } catch(DbException& dbex) {
    throw iceUtil::JobDbException( dbex.what() );
  } catch(exception& ex) {
    throw JobDbException( ex.what() );
  } catch(...) {
    throw JobDbException( "Unknown exception catched" );
  }
}

//____________________________________________________________________
string iceUtil::jobDbManager::getByGid( const string& gid ) 
  throw(iceUtil::JobDbException&)
{
    // FIXME: Going to READ from database.
    // For now the client (jobCache) put a lock
    // but the transaction mechanism should make the 
    // modifications visible to this READ only if the 
    // transaction itself is commited.
    // In future we could put a BDB-lock
  
  Dbt key( (void*)gid.c_str(), gid.length()+1 );
  Dbt cid, jdata;
  m_gidDb->get( NULL, &key, &cid, 0);
  // now cid contains the CreamJobID
    
  m_creamJobDb->get(NULL, &cid, &jdata, 0); // can raise a JobDbException
  return string( (char*)jdata.get_data() );
}

//____________________________________________________________________
void iceUtil::jobDbManager::getAllRecords( vector<string>& destVec )
  throw(iceUtil::JobDbException&)
{
  Dbc *cursor;
  Dbt key, data;
  int ret;
  
  try {
  
    // FIXME: Going to READ from database.
    // For now the client (jobCache) put a lock
    // but the transaction mechanism should make the 
    // modifications visible to this READ only if the 
    // transaction itself is commited.
    // In future we could put a BDB-lock
  
    m_creamJobDb->cursor(NULL, &cursor, 0);
    while((ret=cursor->get(&key, &data, DB_NEXT)) == 0) {
      destVec.push_back( (char*)data.get_data());
    }
    
  } catch(DbException& dbex) {
    if(cursor) cursor->close();
    throw JobDbException( dbex.what() );
  } catch(exception& ex) {
    if(cursor) cursor->close();
    throw JobDbException( ex.what() );
  } catch(...) {
    if(cursor) cursor->close();
    throw JobDbException( "Unknown exception catched" );
  }
  cursor->close();
}

//____________________________________________________________________
void iceUtil::jobDbManager::delByCid( const string& cid )
  throw(iceUtil::JobDbException&)
{
  Dbt key( (void*)cid.c_str(), cid.length()+1 );
  Dbt gidData;
  DbTxn* txn_handler;
  
  try {
    txn_handler = NULL; // this is required because after a commit/abort the
    			  // transaction handler is not valid anymore and
			  // a new fresh handler is required for another 
			  // transaction. Setting it to NULL make it "fresh"
			  // for the txn_begin func.
			  
    m_env.txn_begin(NULL, &txn_handler, 0);
    m_creamJobDb->del( txn_handler, &key, 0);
    m_cidDb->get( txn_handler, &key, &gidData, 0);
    m_gidDb->del( txn_handler, &gidData, 0);
    m_cidDb->del( txn_handler, &key, 0);
    txn_handler->commit(0);
  } catch(DbException& dbex) {
    txn_handler->abort();
    throw iceUtil::JobDbException( dbex.what() );
  } catch(exception& ex) {
    txn_handler->abort();
    throw iceUtil::JobDbException( ex.what() );
  } catch(...) {
    txn_handler->abort();
    throw iceUtil::JobDbException( "Unknown exception catched" );
  }
}

//____________________________________________________________________
void iceUtil::jobDbManager::delByGid( const string& gid )
  throw(iceUtil::JobDbException&)
{
  Dbt key( (void*)gid.c_str(), gid.length()+1 );
  Dbt cidData;
  DbTxn* txn_handler;
  
  try {
    txn_handler = NULL; // this is required because after a commit/abort the
    			  // transaction handler is not valid anymore
			  // a new fresh handler is required for another 
			  // transaction
    m_env.txn_begin(NULL, &txn_handler, 0);
    
    m_gidDb->get( txn_handler, &key, &cidData, 0);
    m_creamJobDb->del( txn_handler, &cidData, 0);
    m_cidDb->del( txn_handler, &cidData, 0);
    m_gidDb->del( txn_handler, &key, 0);
    
    txn_handler->commit(0);
  } catch(DbException& dbex) {
    txn_handler->abort();
    throw iceUtil::JobDbException( dbex.what() );
  } catch(exception& ex) {
    txn_handler->abort();
    throw iceUtil::JobDbException( ex.what() );
  } catch(...) {
    txn_handler->abort();
    throw iceUtil::JobDbException( "Unknown exception catched" );
  }
}
