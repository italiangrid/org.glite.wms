/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * ICE job DB manager
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "jobDbManager.h"
#include "dbCursorWrapper.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <sys/vfs.h> // for statfs
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

namespace iceUtil = glite::wms::ice::util;
using namespace std;
extern int errno;

#define MAX_ICE_OP_BEFORE_PURGE 10000
#define MAX_ICE_OP_BEFORE_CHECKPOINT 50

//____________________________________________________________________
iceUtil::jobDbManager::jobDbManager( const string& envHome, const bool recover, const bool autopurge, const bool read_only)
  : m_env(0),
    m_envHome( envHome ), // The directory pointed by
    			  // envHome MUST exist
    m_valid( false ),
    m_invalid_cause( ),
    m_cream_open( false ),
    m_cid_open( false ),
    m_gid_open( false ),
    m_env_open( false ),
    m_op_counter(0),
    m_op_counter_chkpnt(0),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
  if( !boost::filesystem::exists( boost::filesystem::path(m_envHome, boost::filesystem::native) ) ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << "jobDbManager::jobDbManager() - "
                        << "job DB Path "
                        << m_envHome 
                        << " does not exist. Job DB initializatoin failed."
                        << log4cpp::CategoryStream::ENDLINE
                        );      
        
        m_invalid_cause = string("Path [")+m_envHome+"] doesn't exist and cannot be created";
        return;
    }
    
    if( !boost::filesystem::is_directory( boost::filesystem::path(m_envHome, boost::filesystem::native) ) ) {
        m_invalid_cause = string("Path [")+m_envHome+"] does exist but it is not a directory";
        return;
    }
    
    struct stat buf;
    if(-1==stat( m_envHome.c_str() , &buf) ) {
        m_invalid_cause = string(strerror(errno));
        return;
    }
    
    if( !(buf.st_mode & S_IRUSR)) {
        m_invalid_cause = string("Path [")+m_envHome+"] is not readable by the owner";
        return;
    } 

    if( !(buf.st_mode & S_IWUSR)) {
        m_invalid_cause = string("Path [")+m_envHome+"] is not writable by the owner";
        return;
    } 
    
    if( !(buf.st_mode & S_IXUSR)) {
        m_invalid_cause = string("Path [")+m_envHome+"] is not executable by the owner (cannot cd into it)";
        return;
    }
    
    // Get the filesystem blocksize
    struct statfs fsinfo;
    statfs( m_envHome.c_str(), &fsinfo );
    
    int envFlags;
    
    // FIXME: FOR NOW we do not use the flag DB_THREAD because
    // the concurrency protection will be made at higher level
    // (by the client of this class, i.e. by the jobCache object)
    envFlags = DB_CREATE | DB_INIT_LOCK | 
        DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
    if( recover ) envFlags |= DB_RECOVER; // this flags causes an exception raising by
					  // the DbEnv::txn_begin call when a another
					  // process opens the database that is already
					  // handled by the process doing the txn_begin call.
           
    // FIXME: for now we do not use the flag DB_THREAD because
    // the concurrency protection will be made at higher level
    // (by the client of this class, i.e. by the jobCache object)
    try {

        m_env.open(m_envHome.c_str(), envFlags, 0);
        
        m_env_open = true;
        
        m_creamJobDb = new Db(&m_env, 0);
        m_cidDb      = new Db(&m_env, 0);
        m_gidDb      = new Db(&m_env, 0);
        
        /**
         * "Essentially, DB performs data transfer based on the 
         * database page size. That is, it moves data to and from
         * disk a page at a time. For this reason, if the pase size 
         * does not match the I/O block size, then the operating
         * system can introduce inefficiencies in how it responds to
         * DB's I/O requests.
         */
        if(!read_only) {
            m_creamJobDb->set_pagesize( fsinfo.f_bsize );
            m_cidDb->set_pagesize( fsinfo.f_bsize );
            m_gidDb->set_pagesize( fsinfo.f_bsize );
        }
        
        // FIXME: for now we do not use the flag DB_THREAD because
        // the concurrency protection will be made at higher level
        // (by the client of this class, i.e. by the jobCache object)
        if(!read_only) {
            m_creamJobDb->open(NULL, "cream_job_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0);
            m_cidDb->open(NULL, "cream_jobid_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0);
            m_gidDb->open(NULL, "grid_jobid_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0);
        } else {
            m_creamJobDb->open(NULL, "cream_job_table.db", NULL, DB_BTREE, DB_RDONLY, 0);
            m_cidDb->open(NULL, "cream_jobid_table.db", NULL, DB_BTREE, DB_RDONLY, 0);
            m_gidDb->open(NULL, "grid_jobid_table.db", NULL, DB_BTREE, DB_RDONLY, 0);
        }
        
        m_cream_open = true;  
        m_cid_open = true;
        m_gid_open = true;
        
    } catch(DbException& dbex) {
        m_invalid_cause = dbex.what();
        return;
    } catch(exception& ex) {
        m_invalid_cause = ex.what();
        return;
    } catch(...) {
        m_invalid_cause = "jobDbManager::CTOR - Unknown exception catched";
        return;
    }
    
    m_valid = true;
    
    this->dbLogPurge();
    
    CREAM_SAFE_LOG( m_log_dev->infoStream() 
                            << "jobDbManager::CTOR() - "
                            << "Created database for jobs caching in [" 
                            << m_envHome << "]" 
                            << log4cpp::CategoryStream::ENDLINE
                            );
}

//____________________________________________________________________
iceUtil::jobDbManager::~jobDbManager() throw()
{
    // FIXME: only one thread at time can call ->close()
    // so these operations should be 'mutex-ed'
    
    // Db MUST be close BEFORE closing the DbEnv
    try {
        if(m_creamJobDb && m_cream_open) m_creamJobDb->close(0);
        if(m_cidDb && m_cid_open) m_cidDb->close(0);
        if(m_gidDb && m_gid_open) m_gidDb->close(0);
        if(m_env_open) m_env.close(0);
    } catch(DbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()                      
                        << "jobDbManager::~jobDbManager() -"
                        << "Error \"" << dbex.what() << "\" "
                        << "occurred closing database or environment. "
                        << "Database corruption is possibile!"
                        << log4cpp::CategoryStream::ENDLINE
                        );
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
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException( dbex.what() );
    } catch(exception& ex) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException( ex.what() );
    } catch(...) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException("jobDbManager::put() - Unknown exception catched");
    }
    m_op_counter++;
    m_op_counter_chkpnt++;
    
    if(m_op_counter_chkpnt > MAX_ICE_OP_BEFORE_CHECKPOINT) {
        try {
            m_env.txn_checkpoint(0,0,0);
        } catch(DbException& dbex) {
            throw iceUtil::JobDbException( dbex.what() );
        }
        m_op_counter_chkpnt = 0;
    }
    
    if(m_op_counter>MAX_ICE_OP_BEFORE_PURGE) {
        this->dbLogPurge();
        m_op_counter = 0;
    }
}

//____________________________________________________________________
void iceUtil::jobDbManager::mput(const map<string, pair<string, string> >& key_and_data)
  throw(iceUtil::JobDbException&)
{
    DbTxn* txn_handler;
    
    try {
        txn_handler = NULL; // this is required because after a commit/abort the
        // transaction handler is not valid anymore
        // a new fresh handler is required for another 
        // transaction
        m_env.txn_begin(NULL, &txn_handler, 0);
        
        for(map<string, pair<string, string> >::const_iterator cit = key_and_data.begin();
            cit != key_and_data.end();
            ++cit)
            {
                Dbt cidKey( (void*)(cit->first).c_str(), (cit->first).length()+1);
                Dbt gidKey( (void*)(cit->second.first).c_str(), (cit->second.first).length()+1);
                Dbt data( (void*)(cit->second.second).c_str(), (cit->second.second).length()+1);
                m_creamJobDb->put( txn_handler, &cidKey, &data, 0);
                m_cidDb->put( txn_handler, &cidKey, &gidKey, 0);
                m_gidDb->put( txn_handler, &gidKey, &cidKey, 0);
                m_op_counter++;
                m_op_counter_chkpnt++;
                
                if(m_op_counter_chkpnt > MAX_ICE_OP_BEFORE_CHECKPOINT) {
                    try {
                        m_env.txn_checkpoint(0,0,0);
                    } catch(DbException& dbex) {
                        throw iceUtil::JobDbException( dbex.what() );
                    }
                    m_op_counter_chkpnt = 0;
                }
                if(m_op_counter>MAX_ICE_OP_BEFORE_PURGE) {
                    this->dbLogPurge();
                    m_op_counter = 0;
                }
                
            }
        
        txn_handler->commit(0);
        
    } catch(DbException& dbex) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException( dbex.what() );
    } catch(exception& ex) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException( ex.what() );
    } catch(...) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException("jobDbManager::mput() - Unknown exception catched");
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
    throw JobDbException( "jobDbManager::getByCid() - Unknown exception catched" );
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
  Dbt key, data;
  int ret;
  
  try {
  
    // FIXME: Going to READ from database.
    // For now the client (jobCache) put a lock
    // but the transaction mechanism should make the 
    // modifications visible to this READ only if the 
    // transaction itself is commited.
    // In future we could put a BDB-lock but probably it's
    // NOT needed thanks to the isolation provided by the
    // transaction
    
    iceUtil::cursorWrapper C( m_creamJobDb );
    //m_creamJobDb->cursor(NULL, &cursor, 0);
    while( (ret = C.get(&key, &data)) == 0 ) {
      destVec.push_back( (char*)data.get_data());
    }
    
  } catch(DbException& dbex) {
    //if(cursor) cursor->close();
    throw JobDbException( dbex.what() );
  } catch(exception& ex) {
    //if(cursor) cursor->close();
    throw JobDbException( ex.what() );
  } catch(...) {
    //if(cursor) cursor->close();
    throw JobDbException( "jobDbManager::getAllRecords() - Unknown exception catched" );
  }
  //cursor->close();
}

//____________________________________________________________________
void iceUtil::jobDbManager::delByCid( const string& cid )
  throw(iceUtil::JobDbException&)
{
  Dbt key( (void*)cid.c_str(), cid.length()+1 );
  Dbt gidData;
  DbTxn* txn_handler;
  
  try {
    txn_handler = NULL;
    m_env.txn_begin(NULL, &txn_handler, 0);
    m_creamJobDb->del( txn_handler, &key, 0);
    m_cidDb->get( txn_handler, &key, &gidData, 0);
    m_gidDb->del( txn_handler, &gidData, 0);
    m_cidDb->del( txn_handler, &key, 0);
    txn_handler->commit(0);
  } catch(DbException& dbex) {
    if(txn_handler) txn_handler->abort();
    throw iceUtil::JobDbException( dbex.what() );
  } catch(exception& ex) {
    if(txn_handler) txn_handler->abort();
    throw iceUtil::JobDbException( ex.what() );
  } catch(...) {
    if(txn_handler) txn_handler->abort();
    throw iceUtil::JobDbException( "jobDbManager::delByCid() - Unknown exception catched" );
  }
  m_op_counter++;
  m_op_counter_chkpnt++;
  
  if(m_op_counter_chkpnt > MAX_ICE_OP_BEFORE_CHECKPOINT)
  {
    try{m_env.txn_checkpoint(0,0,0);}
    catch(DbException& dbex) {
      throw iceUtil::JobDbException( dbex.what() );
    }
    m_op_counter_chkpnt = 0;
  }
  if(m_op_counter>MAX_ICE_OP_BEFORE_PURGE) {
    this->dbLogPurge();
    m_op_counter = 0;
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
        txn_handler = NULL;
        m_env.txn_begin(NULL, &txn_handler, 0);
        
        m_gidDb->get( txn_handler, &key, &cidData, 0);
        m_creamJobDb->del( txn_handler, &cidData, 0);
        m_cidDb->del( txn_handler, &cidData, 0);
        m_gidDb->del( txn_handler, &key, 0);
        
        txn_handler->commit(0);
    } catch(DbException& dbex) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException( dbex.what() );
    } catch(exception& ex) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException( ex.what() );
    } catch(...) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::JobDbException( "jobDbManager::delByGid() - Unknown exception catched" );
    }
    m_op_counter++;
    m_op_counter_chkpnt++;
    
    if(m_op_counter_chkpnt > MAX_ICE_OP_BEFORE_CHECKPOINT) {
        try {
            m_env.txn_checkpoint(0,0,0);
        } catch(DbException& dbex) {
            throw iceUtil::JobDbException( dbex.what() );
        }
        m_op_counter_chkpnt = 0;
    }
    if(m_op_counter>MAX_ICE_OP_BEFORE_PURGE) {
        this->dbLogPurge();
        m_op_counter = 0;
    }
}

//____________________________________________________________________
void iceUtil::jobDbManager::dbLogPurge( void )
{
    char **file, **list;
    try {
        
        m_env.log_archive( &list, DB_ARCH_ABS );
        
        if(list == NULL) {m_op_counter = 0; return;}
        
        for (file = list; *file != NULL; ++file) {
            CREAM_SAFE_LOG( m_log_dev->infoStream() 
                            << "jobDbManager::dbLogPurge() - "
                            << "Removing unused DB logfile [" 
                            << *file << "]" 
                            << log4cpp::CategoryStream::ENDLINE
                            );
            
            if(-1==::unlink(*file)) {
                int saveerr = errno;
                CREAM_SAFE_LOG( m_log_dev->errorStream() 
                                << "jobDbManager::dbLogPurge() - "
                                << "Error removing DB logfile [" 
                                << *file << "]: " 
                                << strerror(saveerr)
                                << log4cpp::CategoryStream::ENDLINE
                                );
            }
        }
        free(list); // must only free the main reference (NOT all strings in the memory, see DB doc)
    } catch(DbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "jobDbManager::dbLogPurge() - "
                        << "DbException raised when trying to purge "
                        << "unsed DB log files: \"" << dbex.what() << "\""
                        << log4cpp::CategoryStream::ENDLINE
                        );
        // free(files); // FIXME: uncomment this if you decide to continue to run
        abort();
    } catch(exception& ex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "jobDbManager::dbLogPurge() - "
                        << "Exception raised when trying to purge "
                        << "unsed DB log files: \"" << ex.what() << "\""
                        << log4cpp::CategoryStream::ENDLINE
                        );
        // free(files); // FIXME: uncomment this if you decide to continue to run
        abort();
    } catch(...) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "jobDbManager::dbLogPurge() - "
                        << "Unknown exception raised when trying to purge "
                        << "unsed DB log files"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        // free(files); // FIXME: uncomment this if you decide to continue to run
        abort();
    }
}

//____________________________________________________________________
void checkPointing( void ) 
{

  /**
   * MUST think about this carefully. 
   * Checkpointing reduce recover time, but also increase cache flush
   * frequency.
   */

  //if(m_op_counter > MAX_OP_BEFORE_CHECKPOINT)
  //{
    
  //}
}
