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
 * ICE job DB Transaction Manager
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "Transaction.h"
#include "AbsDbOperation.h"

#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "iceUtils/iceConfManager.h" // iceConfManager
#include "iceUtils/creamJob.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"

#include <sys/vfs.h> // for statfs
#include "sqlite3.h" // for sqlite3
#include <cerrno>
#include <string>

namespace iceUtil = glite::wms::ice::util;

using namespace std;
using namespace glite::wms::ice::db;

sqlite3* Transaction::s_db = 0;

//
// Local namespace, for locally visible only operation
//
namespace {

    /**
     * This operation represents the SQL query used to create
     * the databse.
     */ 
    class CreateDb : public AbsDbOperation {
    public:
      CreateDb() : AbsDbOperation() { };
      virtual ~CreateDb() { };
    
      
        virtual void execute( sqlite3* db ) throw() {            
            try {
	      ostringstream sqlcmd;
	      sqlcmd << "CREATE TABLE IF NOT EXISTS jobs ( "
		     << iceUtil::CreamJob::get_createdb_query()
		     << ")";
		   

	      do_query( db, sqlcmd.str() );
		
	    } catch( DbOperationException& ex ) {
	    
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating database table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
            }
	    try {
	      string sqlcmd = 
		"CREATE TABLE IF NOT EXISTS proxy ( "	\
		"userdn text primary key not null, "	\
		"proxyfile text not null, "		\
		"exptime integer(4) not null, "		\
		"counter integer(8) not null"		\
		")";
	      do_query( db, sqlcmd );

	    } catch( DbOperationException& ex ) {
	    
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating database table proxy: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
	    }
	    try {
	      string sqlcmd = 
		"CREATE TABLE IF NOT EXISTS delegation (" \
		"digest text not null, "		  \
		"creamurl text not null, "		  \
		"exptime integer(4) not null, "		  \
		"duration integer(4) not null,"		  \
		"delegationid text not null,"		  \
		"userdn text not null,"			  \
		"renewable integer(1),"			  \
		"myproxyurl text not null"		  \
		")";
	      do_query( db, sqlcmd );
	      
	    } catch( DbOperationException& ex ) {
	    
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
                        << "CreateDb::execute() - "
			<< "Error creating database table delegation: "
			<< ex.what() << ". STOP!"
                        );
	      abort();
	      
	    }
	    try {
	      string sqlcmd = 
		"CREATE TABLE IF NOT EXISTS lease ( "	\
		"userdn text not null, "		\
		"creamurl text not null, "		\
		"exptime integer(4) not null, "		\
		"leaseid text not null"			\
		")";
	      do_query( db, sqlcmd );
	      
	    } catch( DbOperationException& ex ) {
	    
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating database table lease: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
	    }
            try {
	      string sqlcmd = 
		"CREATE UNIQUE INDEX IF NOT EXISTS gid_index ON jobs (gridjobid)"; 
	      do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating index gid_index on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
            }
            try { 
	      string sqlcmd = 
		"CREATE INDEX IF NOT EXISTS cid_index ON jobs (creamjobid)";
	      do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	    
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating index cid_index on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
            }
            try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS ccid_index ON jobs (complete_cream_jobid)";
              do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating index ccid_index on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
            }
	    try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS lastseen ON jobs (last_seen)";
              do_query( db, sqlcmd );
            } catch(DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			     << "CreateDb::execute() - "
			     << "Error creating index lastseen on table jobs: "
			     << ex.what() << ". STOP!"
			     );
	      abort();
	    
            }
	    try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS lastemptynotification ON jobs (last_empty_notification)";
              do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error creating index lastemptynotification on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
            }
	    try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS stat ON jobs (status)";
              do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating index stat on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
            }
	    try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS udn ON jobs (userdn)";
              do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating index udn on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
            }
	    try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS curl ON jobs (creamurl)";
              do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating index curl on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
            }
	    
	    try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS lastpollervisit ON jobs (last_poller_visited)";
              do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
			      << "CreateDb::execute() - "
			      << "Error creating index lastpollervisit on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
            }
	    try {
              string sqlcmd =
                "CREATE INDEX IF NOT EXISTS killedbyice ON jobs (is_killed_byice)";
              do_query( db, sqlcmd );
            } catch( DbOperationException& ex ) {
	    
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error creating index killedbyice on table jobs: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
            }
	    try {
	      string sqlcmd = 
		"CREATE UNIQUE INDEX IF NOT EXISTS userdn_index ON proxy (userdn)";
	      do_query( db, sqlcmd );
	    } catch(DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error creating index userdn_index on table proxy: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
	    }
	    try {
	      string sqlcmd = 
		"CREATE UNIQUE INDEX IF NOT EXISTS delegkey ON delegation (digest,creamurl)";
	      do_query( db, sqlcmd );
	    } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error creating index delegkey on table delegation: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	    
	    }
	    try {
	      string sqlcmd = 
		"CREATE UNIQUE INDEX IF NOT EXISTS leasekey ON lease (userdn,creamurl)";
	      do_query( db, sqlcmd );
	    } catch( DbOperationException& ex ) {
	    
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error creating index leasekey on table lease: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
	    }
	    try {
	      string sqlcmd = 
		"PRAGMA default_cache_size=200;";
	      do_query( db, sqlcmd );
	    } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error setting database's default cache size: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
	    }
	    
	    try {
	      string sqlcmd = 
		"PRAGMA synchronous=OFF;";
	      do_query( db, sqlcmd );
	    } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error setting database's synchronous: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
	    }
	    
	    try {
	      string sqlcmd = 
		"PRAGMA temp_store=2;";
	      do_query( db, sqlcmd );
	    } catch( DbOperationException& ex ) {
	      
	      CREAM_SAFE_LOG( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream() 
			      << "CreateDb::execute() - "
			      << "Error setting database's temp_store: "
			      << ex.what() << ". STOP!"
			      );
	      abort();
	      
	    }
        };
    };

    /**
     * This operation begins a transaction on the database
     */
    class BeginTransaction : public AbsDbOperation {
    protected:
        bool m_exclusive;
    public:
        BeginTransaction( bool exclusive = false ) : AbsDbOperation(), m_exclusive( exclusive ) { };
        virtual ~BeginTransaction() { };
        virtual void execute( sqlite3* db ) throw( DbOperationException& ) {
            string sqlcmd;
            if ( m_exclusive ) 
                sqlcmd = string( "begin exclusive transaction;" );
            else
                sqlcmd = string( "begin immediate transaction;");
            do_query( db, sqlcmd );
        };
    };

    /**
     * This operation commits a transaction on the database
     */
    class CommitTransaction : public AbsDbOperation {
    public:
        CommitTransaction() : AbsDbOperation() { };
        virtual ~CommitTransaction() { };
        virtual void execute( sqlite3* db ) throw( DbOperationException& ) {
            string sqlcmd( "commit transaction;" );
            do_query( db, sqlcmd );
        };
    };


    /**
     * This operation rollback a transaction on the database
     */
    class RollbackTransaction : public AbsDbOperation {
    public:
        RollbackTransaction() : AbsDbOperation() { };
        virtual ~RollbackTransaction() { };
        virtual void execute( sqlite3* db ) throw( DbOperationException& ) {
            string sqlcmd( "rollback transaction;" );
            do_query( db, sqlcmd );
        };
    };

}
//
// End local namespace
//


Transaction::Transaction( const bool create_chek ) :
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_begin( false ),
    m_commit( true )
{
    if ( 0 == s_db ) { // create the db
        create_db( create_chek );
    }
}


void Transaction::create_db( const bool create_check ) 
{
    static const char* method_name = "Transaction::create_db() - ";
    string persist_dir( iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() );

    if( create_check ) {
      
      if( !boost::filesystem::exists( boost::filesystem::path(persist_dir, boost::filesystem::native) ) ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << "job DB Path "
                        << persist_dir
                        << " does not exist. Job DB initializatoin failed."
                        );      
        exit(-1);
      }
      
      if( !boost::filesystem::is_directory( boost::filesystem::path(persist_dir, boost::filesystem::native) ) ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << "Path [" << persist_dir
                        << "] does exist but it is not a directory"
                        );
        exit(-1);
      }
      
      struct stat buf;
      if( -1==stat( persist_dir.c_str() , &buf) ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << strerror(errno) );
        exit(-1);
      }
      
      if( !(buf.st_mode & S_IRUSR)) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << "Path [" << persist_dir
                        << "] is not readable by the owner" );
        exit(-1);
      } 
      
      if( !(buf.st_mode & S_IWUSR)) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << "Path [" << persist_dir
                        << "] is not writable by the owner" );
        exit(-1);
      } 
      
      if( !(buf.st_mode & S_IXUSR)) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << "Path [" << persist_dir
                        << "] is not executable by the owner (cannot cd into it)" );
        exit(-1);
      }
    } // if( create_check )

    int rc = sqlite3_open( string(persist_dir + "/ice.db").c_str(), &s_db );
    if ( SQLITE_OK != rc ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << "Failed to open/create DB. Error message is "
                        << sqlite3_errmsg( s_db ) );
        exit(-1);
    }
    sqlite3_soft_heap_limit( 10485760 );
    sqlite3_busy_timeout( s_db, 5000) ;
    // Create table if not exists
    if( create_check )
      CreateDb().execute( s_db );
    // Database intentionally left open
}


Transaction& Transaction::Begin( void )
{
    m_begin = true;
    BeginTransaction().execute( s_db ); // FIXME: catch exception
    return *this;
}

Transaction& Transaction::Begin_exclusive( void )
{
    m_begin = true;
    BeginTransaction(true).execute( s_db ); // FIXME: catch exception
    return *this;
}

void Transaction::Commit( void )
{
    m_commit = true;
}

void Transaction::Abort( void )
{ 
    m_commit = false;
}

Transaction& Transaction::execute( AbsDbOperation* op ) throw( DbOperationException& )
{
    if ( !op ) 
        return *this; // nothing to do

    static const char* method_name = "Transaction::execute() - ";
    int retry_cnt = 1;
    const int retry_cnt_max = 5;
    while( 1 ) {
        try {
            op->execute( s_db );
	    int freed = sqlite3_release_memory( 104800000 );
	    

            return *this; // normal termination
        } catch ( DbLockedException& ex ) {
            //if ( ++retry_cnt < retry_cnt_max ) {
	    
                CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                                << "Database is locked; retrying operation"
                                << " in " << retry_cnt << " seconds" );
                sleep( retry_cnt++ );
		                
            //} else {
            //    throw ex; // rethrow
            //}
        }
    }
    return *this;
}

Transaction::~Transaction( ) 
{
    static const char* method_name = "Transaction::~Transaction() - ";

    // If the user started a transaction, we need to either
    // commit or rollback it
    if ( m_begin ) {
        try {
            if ( m_commit ) {
                CommitTransaction().execute( s_db );
		
            } else {
                RollbackTransaction().execute( s_db );           
            }
        } catch( DbOperationException& ex ) {
            CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                            << "End transaction failed: "
                            << ex.what() );
            abort();
        }
    }
    //    sqlite3_close( m_db );
    //    m_db = 0;

}
