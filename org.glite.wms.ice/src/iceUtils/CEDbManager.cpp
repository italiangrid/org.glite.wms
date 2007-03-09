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
 * ICE url&DN DB manager
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
 
#include "CEDbManager.h"
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
#include <cstring>

#include <sstream>

namespace iceUtil = glite::wms::ice::util;
using namespace std;
extern int errno;

#define MAX_ICE_OP_BEFORE_PURGE 10000
#define MAX_ICE_OP_BEFORE_CHECKPOINT 50



//____________________________________________________________________
iceUtil::CEDbManager::CEDbManager( const string& envHome, const bool recover, const bool autopurge, const bool read_only)
  : m_env(0),
    m_envHome( envHome ), // The directory pointed by
    			  // envHome MUST exist
    m_valid( false ),
    m_invalid_cause( ),
    m_ce_open( false ),
    m_dn_open( false ),
    m_env_open( false ),
    m_op_counter(0),
    m_op_counter_chkpnt(0),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
  if( !boost::filesystem::exists( boost::filesystem::path(m_envHome, boost::filesystem::native) ) ) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()
                        << "CEDbManager::CEDbManager() - "
                        << "CE DB Path "
                        << m_envHome 
                        << " does not exist. CE DB initializatoin failed."
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
        m_invalid_cause = string( strerror(errno) );
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
        
        m_CEDb      = new Db(&m_env, 0);
	m_DNDb	    = new Db(&m_env, 0);
        
        /**
         * "Essentially, DB performs data transfer based on the 
         * database page size. That is, it moves data to and from
         * disk a page at a time. For this reason, if the pase size 
         * does not match the I/O block size, then the operating
         * system can introduce inefficiencies in how it responds to
         * DB's I/O requests.
         */
        if(!read_only) {
            m_CEDb->set_pagesize( fsinfo.f_bsize );
	    m_DNDb->set_pagesize( fsinfo.f_bsize );
        }
        
        // FIXME: for now we do not use the flag DB_THREAD because
        // the concurrency protection will be made at higher level
        // (by the client of this class, i.e. by the jobCache object)
        if(!read_only) {
            m_CEDb->open(NULL, "ce_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0);
            m_DNDb->open(NULL, "dn_table.db", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0);
        } else {
            m_CEDb->open(NULL, "ce_table.db", NULL, DB_BTREE, DB_RDONLY, 0);
	    m_DNDb->open(NULL, "dn_table.db", NULL, DB_BTREE, DB_RDONLY, 0);
        }
        
        m_ce_open = true;
	m_dn_open = true;
        //m_cid_open = true;
        //m_gid_open = true;
        
    } catch(DbException& dbex) {
        m_invalid_cause = dbex.what();
        return;
    } catch(exception& ex) {
        m_invalid_cause = ex.what();
        return;
    } catch(...) {
        m_invalid_cause = "CEDbManager::CTOR - Unknown exception catched";
        return;
    }
    
    m_valid = true;
    
    this->dbLogPurge();
    CREAM_SAFE_LOG( m_log_dev->infoStream() 
                            << "CEDbManager::CTOR() - "
                            << "Created database for CEMon URL/DN caching in [" 
                            << m_envHome << "]" 
                            << log4cpp::CategoryStream::ENDLINE
                            );
}

//____________________________________________________________________
iceUtil::CEDbManager::~CEDbManager() throw()
{
    // FIXME: only one thread at time can call ->close()
    // so these operations should be 'mutex-ed'
    
    // Db MUST be close BEFORE closing the DbEnv
    try {
        if(m_CEDb && m_ce_open) m_CEDb->close(0);
	if(m_DNDb && m_dn_open) m_DNDb->close(0);
        if(m_env_open) m_env.close(0);
    } catch(DbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->errorStream()                      
                        << "CEDbManager::~CEDbManager() -"
                        << "Error \"" << dbex.what() << "\" "
                        << "occurred closing database or environment. "
                        << "Database corruption is possibile!"
                        << log4cpp::CategoryStream::ENDLINE
                        );
    }
    
    delete(m_CEDb); 
    delete(m_DNDb);
}


//____________________________________________________________________
void iceUtil::CEDbManager::put(const string& cream_url, const vector< pair<string, string> >& cemonDN)
    throw(iceUtil::CEDbException&)
{
    
    //Dbt cidData( (void *)cid.c_str(), cid.length()+1);
    //Dbt data( (void*)cream_url.c_str(), cream_url.length()+1 );
    //Dbt gidData( (void*)gid.c_str(), gid.length()+1 );
    
    ostringstream cemonBuf("");
    map<string, string> dnMap;
    
    Dbt creamURLData( (void *)cream_url.c_str(), cream_url.length()+1);
    
//     int bloblen = 0, dnlen = 0;
    for( vector< pair<string, string> >::const_iterator it = cemonDN.begin();
         it != cemonDN.end();
 	 ++it) 
     {
	cemonBuf << it->first << "|";
	dnMap[it->first] = it->second;
     }
//     char *blobCEMon = (char*)malloc( bloblen+1+(cemonDN.length()-1) );
//     char *blobDN    = (char*)malloc( dnlen+1+(cemonDN.length()-1) );
//     memset((void*) blobCEMon, 0, bloblen+1+(cemonDN.length()-1) );
//     memset((void*) dnlen, 0, dnlen+1+(cemonDN.length()-1) );
//     
//     for( vector< pair<string, string> >::const_iterator it = cemonDN.begin();
//          it != cemonDN.end();
// 	 ++it) 
//     {
//       blobCEMon
//     }
    
//    Dbt CEMonsData
    
    Dbt cemonListData( (void *)cemonBuf.str().c_str(), cemonBuf.str().length()+1 );
    
    
    
    
    DbTxn* txn_handler;
    
    try {
        txn_handler = NULL; // this is required because after a commit/abort the
        // transaction handler is not valid anymore
        // a new fresh handler is required for another 
        // transaction
        
        m_env.txn_begin(NULL, &txn_handler, 0);
        m_CEDb->put( txn_handler, &creamURLData, &cemonListData, 0);
	
	for(map<string, string>::const_iterator it = dnMap.begin();
	    it != dnMap.end();
	    ++it)
	{
	  Dbt cemon((void *)it->first.c_str(), it->first.length()+1 );
	  Dbt dn((void *)it->second.c_str(), it->second.length()+1 );
	  m_DNDb->put( txn_handler, &cemon, &dn, 0);
	}
      
        txn_handler->commit(0);
	
    } catch(DbException& dbex) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::CEDbException( dbex.what() );
    } catch(exception& ex) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::CEDbException( ex.what() );
    } catch(...) {
        if(txn_handler) txn_handler->abort();
        throw iceUtil::CEDbException("jobDbManager::put() - Unknown exception catched");
    }
    m_op_counter++;
    m_op_counter_chkpnt++;
    
    if(m_op_counter_chkpnt > MAX_ICE_OP_BEFORE_CHECKPOINT) {
        try {
            m_env.txn_checkpoint(0,0,0);
        } catch(DbException& dbex) {
            throw iceUtil::CEDbException( dbex.what() );
        }
        m_op_counter_chkpnt = 0;
    }
    
    if(m_op_counter>MAX_ICE_OP_BEFORE_PURGE) {
        this->dbLogPurge();
        m_op_counter = 0;
    }
}

//____________________________________________________________________
void iceUtil::CEDbManager::dbLogPurge( void )
{
    char **file, **list;
    try {
        
        m_env.log_archive( &list, DB_ARCH_ABS );
        
        if(list == NULL) {m_op_counter = 0; return;}
        
        for (file = list; *file != NULL; ++file) {
            CREAM_SAFE_LOG( m_log_dev->infoStream() 
                            << "CEDbManager::dbLogPurge() - "
                            << "Removing unused DB logfile [" 
                            << *file << "]" 
                            << log4cpp::CategoryStream::ENDLINE
                            );
            
            if(-1==::unlink(*file)) {
                int saveerr = errno;
                CREAM_SAFE_LOG( m_log_dev->errorStream() 
                                << "CEDbManager::dbLogPurge() - "
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
                        << "CEDbManager::dbLogPurge() - "
                        << "DbException raised when trying to purge "
                        << "unsed DB log files: \"" << dbex.what() << "\""
                        << log4cpp::CategoryStream::ENDLINE
                        );
        // free(files); // FIXME: uncomment this if you decide to continue to run
        abort();
    } catch(exception& ex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "CEDbManager::dbLogPurge() - "
                        << "Exception raised when trying to purge "
                        << "unsed DB log files: \"" << ex.what() << "\""
                        << log4cpp::CategoryStream::ENDLINE
                        );
        // free(files); // FIXME: uncomment this if you decide to continue to run
        abort();
    } catch(...) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "CEDbManager::dbLogPurge() - "
                        << "Unknown exception raised when trying to purge "
                        << "unsed DB log files"
                        << log4cpp::CategoryStream::ENDLINE
                        );
        // free(files); // FIXME: uncomment this if you decide to continue to run
        abort();
    }
}
//____________________________________________________________________
void iceUtil::CEDbManager::getByCreamUrl( const std::string& creamurl, 
					  std::string& cemonURL, std::string& cemonDN ) 
  throw(CEDbException&)
{
  Dbt key( (void*)creamurl.c_str(), creamurl.length()+1 );
  Dbt cemon, dn;
  m_CEDb->get( NULL, &key, &cemon, 0);
  m_DNDb->get(NULL, &cemon, &dn, 0);
  //return make_pair( (char*)cemon.get_data(), (char*)dn.get_data() );
  
  cemonURL = (char*)cemon.get_data();
  cemonDN  = (char*)dn.get_data();
}

//____________________________________________________________________
void iceUtil::CEDbManager::getAllRecords( std::map<std::string, std::pair<std::string, std::string> >& target ) throw(CEDbException&)
{
  Dbt key, data; // creamurl, cemonurl
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
    
    iceUtil::cursorWrapper C( m_CEDb );
    //iceUtil::cursorWrapper D( m_DNDb );
    //m_creamJobDb->cursor(NULL, &cursor, 0);
    while( (ret = C.get(&key, &data)) == 0 ) {
      //destVec.push_back( (char*)data.get_data());
      Dbt cemonURL( data.get_data(), strlen((char*)data.get_data())+1 );
      Dbt cemonDN;
      m_DNDb->get(NULL, &cemonURL, &cemonDN, 0);
      target[(char*)key.get_data() ] = make_pair( string((char*)cemonURL.get_data()), string( (char*)cemonDN.get_data()) );
    }
    
  } catch(DbException& dbex) {
    //if(cursor) cursor->close();
    throw CEDbException( dbex.what() );
  } catch(exception& ex) {
    //if(cursor) cursor->close();
    throw CEDbException( ex.what() );
  } catch(...) {
    //if(cursor) cursor->close();
    throw CEDbException( "CEDbManager::getAllRecords() - Unknown exception catched" );
  }
}
