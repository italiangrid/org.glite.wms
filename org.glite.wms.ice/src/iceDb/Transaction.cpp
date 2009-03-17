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

sqlite3* Transaction::m_db = 0;

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
                string sqlcmd = 
		  "CREATE TABLE jobs ( "	          \
		  " gridjobid text primary key, "         \
		  " creamjobid text, "		          \
		  " complete_cream_jobid text,"           \
		  " jdl blob not null, "		  \
		  " serialized blob not null, "           \
		  " userproxy text not null, "            \
		  " ceid text, "                          \
		  " endpoint text, "                      \
		  " creamurl text not null, "             \
		  " creamdelegurl text not null, "        \
		  " userdn text not null, "               \
		  " myproxyurl text not null, "           \
		  " proxy_renewable integer(1) not null," \
		  " failure_reason blob,"                 \
		  " sequence_code text,"                  \
		  " wn_sequence_code text,"               \
		  " prev_status integer(1),"              \
		  " status integer(1),"                   \
		  " num_logged_status_changes integer(1),"\
		  " leaseid text,"                        \
		  " proxycert_timestamp integer(4),"      \
		  " status_poller_retry_count integer(1),"\
		  " exit_code integer(1),"                \
		  " worker_node text,"                    \
		  " is_killed_byice integer(1),"          \
		  " delegationid text,"                   \
		  " delegation_exptime integer(4),"       \
		  " delegation_duration integer(4),"      \
		  " last_empty_notification integer(4),"  \
		  " last_seen integer(4) "		  \
		  ")";
                do_query( db, sqlcmd );
            } catch( ... ) {
            }
            try {
                string sqlcmd = 
                    "create unique index gid_index on jobs (gridjobid)"; 
                do_query( db, sqlcmd );
            } catch( ... ) {
            }
            try { 
                string sqlcmd = 
                    "create index cid_index on jobs (creamjobid)";
                do_query( db, sqlcmd );
            } catch( ... ) {
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
                sqlcmd = string( "begin transaction;");
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


Transaction::Transaction( ) :
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    m_begin( false ),
    m_commit( true )
{
    if ( 0 == m_db ) { // create the db
        create_db();
    }
}


void Transaction::create_db( void ) 
{
    static const char* method_name = "Transaction::create_db() - ";

    string persist_dir( iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir() );

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
    int rc = sqlite3_open( string(persist_dir + "/ice.db").c_str(), &m_db );
    if ( SQLITE_OK != rc ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                        << "Failed to open/create DB. Error message is "
                        << sqlite3_errmsg( m_db ) );
        exit(-1);
    }
    // Create table if not exists
    CreateDb().execute( m_db );
    // Database intentionally left open
}


Transaction& Transaction::begin( void )
{
    m_begin = true;
    BeginTransaction().execute( m_db ); // FIXME: catch exception
    return *this;
}

Transaction& Transaction::begin_exclusive( void )
{
    m_begin = true;
    BeginTransaction(true).execute( m_db ); // FIXME: catch exception
    return *this;
}

void Transaction::commit( void )
{
    m_commit = true;
}

void Transaction::abort( void )
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
            op->execute( m_db );
            return *this; // normal termination
        } catch ( DbLockedException& ex ) {
            if ( ++retry_cnt < retry_cnt_max ) {
                CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                                << "Database is locked; retrying operation"
                                << " in " << retry_cnt << " seconds" );
                sleep( retry_cnt );                
            } else {
                throw ex; // rethrow
            }
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
                CommitTransaction().execute(m_db);
            } else {
                RollbackTransaction().execute(m_db);           
            }
        } catch( DbOperationException& ex ) {
            CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
                            << "End transaction failed: "
                            << ex.what() );
            abort();
        }
    }
}
