/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010.
See http://www.eu-egee.org/partners/ for details on the copyright
holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.
See the License for the specific language governing permissions and
limitations under the License.

END LICENSE */

#include "DelegationManager.h"
#include "DNProxyManager.h"
#include "CreamProxyMethod.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "iceUtils.h"
#include <stdexcept>
#include <sys/time.h>

#include "iceDb/Transaction.h"
#include "iceDb/GetDelegation.h"
#include "iceDb/GetFieldsCount.h"
#include "iceDb/GetAllDelegation.h"
#include "iceDb/GetDelegationByID.h"
#include "iceDb/CreateDelegation.h"
#include "iceDb/RemoveDelegation.h"
#include "iceDb/CheckDelegationByID.h"
#include "iceDb/RemoveDelegationByID.h"
#include "iceDb/UpdateDelegationTimesByID.h"
#include "iceDb/RemoveDelegationByDNMyProxy.h"


namespace cream_api = glite::ce::cream_client_api::soap_proxy;
namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice;
using namespace std;

util::Delegation_manager* util::Delegation_manager::s_instance = 0;
boost::recursive_mutex util::Delegation_manager::s_mutex;

typedef map<string, boost::tuple<string, string, time_t, int, string,bool,string> > DelegInfo;


//______________________________________________________________________________
util::Delegation_manager::Delegation_manager( ) :
    m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
    m_operation_count( 0 ),
    m_max_size( 1000 ), // FIXME: Hardcoded default
    m_operation_count_max( 2000 ) // FIXME: hardcoded default
{
}

//______________________________________________________________________________
util::Delegation_manager* util::Delegation_manager::instance( ) 
{
    boost::recursive_mutex::scoped_lock L( s_mutex );
    if ( 0 == s_instance ) 
        s_instance = new Delegation_manager( );
    return s_instance;
}

//______________________________________________________________________________
string 
util::Delegation_manager::delegate( const CreamJob& job, 
			      const bool USE_NEW,
			      bool force ) 
  throw( std::exception& )
{
    boost::recursive_mutex::scoped_lock L( s_mutex );
    static const char* method_name = "Delegation_manager::delegate() - ";

    if( force )
      CREAM_SAFE_LOG( m_log_dev->debugStream()
		      << method_name
		      << "WARNING: force_delegation is set to TRUE." 
		      );  

    string myproxy_address = job.myproxy_address();

    
    string delegation_id; // delegation ID to return as a result

    
    const string cream_url( job.cream_address() );
    const string cream_deleg_url( job.cream_deleg_address() );
    string str_sha1_digest;

    if(USE_NEW) {
      CREAM_SAFE_LOG( m_log_dev->debugStream()
		      << method_name
		      << "Using new delegation method, DNFQAN=[" 
		      << job.user_dn() << "]"
		      );  
      str_sha1_digest = job.user_dn();
    }
    else {
      str_sha1_digest = utilities::computeSHA1Digest( job.user_proxyfile() );
    }

    
    CREAM_SAFE_LOG( m_log_dev->debugStream()
		    << method_name
		    << "Searching for delegation with key [" 
		    << str_sha1_digest << "] && CREAM_URL ["
		    << cream_url << "]"
		    );  

    bool found = false;
    table_entry deleg_info("", "", 0, 0, "", "", 0, "");
    try {
      db::GetDelegation getter( str_sha1_digest, cream_url, myproxy_address, method_name );
      db::Transaction tnx(false, false);
      //tnx.begin();
      tnx.execute( &getter );
      found = getter.found();
      if(found)
	deleg_info = getter.get_delegation();
    } catch( db::DbOperationException& ex ) {
      throw runtime_error( string("Delegation failed: ") + ex.what() );
    }
    
    if( force && found ) {
      try {
	db::RemoveDelegation remover( str_sha1_digest, cream_url, myproxy_address, method_name );
	db::Transaction tnx(false, false);
	tnx.execute( &remover );
      } catch( db::DbOperationException& ex ) {
	throw runtime_error( string("Delegation failed: ") + ex.what() );
      }

      found = false;
    }

    time_t expiration_time;
    int duration;
    if ( !found ) {

        // Delegation id not found (or force). Performs a new delegation   

        // The delegation ID is the "canonized" GRID job id
      delegation_id   = utilities::canonizeString( /*job.getGridJobID() + cream_url*/ this->generateDelegationID() );
      expiration_time = job.isbproxy_time_end(); 
      duration        = job.isbproxy_time_end() - time(0);
      
        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Creating new delegation "
                        << "with delegation id ["
                        << delegation_id
                        << "] CREAM URL ["
                        << cream_url
                        << "] Delegation URL ["
                        << cream_deleg_url
                        << "] user DN ["
                        << job.user_dn()
                        << "] proxy hash ["
                        << str_sha1_digest
			<< "] MyProxy Server ["
			<< myproxy_address << "] Expiring on [" 
			<< utilities::time_t_to_string( expiration_time ) << "]"
                         );
        
        try {
	  // Gets the proxy expiration time
	  //expiration_time = V.getProxyTimeEnd( );
	  string certfile( job.user_proxyfile() );
	  CreamProxy_Delegate( cream_deleg_url, certfile, delegation_id ).execute( 3 );
        } catch( exception& ex ) {
	  // Delegation failed
	  CREAM_SAFE_LOG( m_log_dev->errorStream()
			  << method_name
			  << "FAILED Creation of a new delegation "
			  << "with delegation id ["
			  << delegation_id
			  << "] CREAM URL ["
			  << cream_url
			  << "] Delegation URL ["
			  << cream_deleg_url
			  << "] user DN ["
			  << job.user_dn()
			  << "] proxy hash ["
			  << str_sha1_digest
			  << "] MyProxy Server ["
			  << myproxy_address << "] - ERROR is: ["
			  << ex.what() << "]"
			  );
	  throw runtime_error(ex.what());
        }  catch( ... ) {
            // Delegation failed
            CREAM_SAFE_LOG( m_log_dev->errorStream()
                            << method_name
                            << "FAILED Creation of a new delegation "
                            << "with delegation id ["
                            << delegation_id
                            << "] CREAM URL ["
                            << cream_url
                            << "] Delegation URL ["
                            << cream_deleg_url
                            << "] user DN ["
                            << job.user_dn()
                            << "] proxy hash ["
                            << str_sha1_digest << "]"
			    << " MyProxy Server ["
			    << myproxy_address << "]"
                             );
            throw runtime_error( "Delegation failed" );
        }     
        // Inserts the new delegation ID into the delegation set
	/**
	   if USE_NEW is true it means that we're using the new delegation mechanism because the user set the MYPROXYSERVER
	*/
	try {
	  db::CreateDelegation creator( str_sha1_digest, 
					cream_url, 
					expiration_time, 
					duration, 
					delegation_id, 
					job.user_dn(), 
					USE_NEW, 
					myproxy_address,
					method_name);
	  db::Transaction tnx(false, false);
	  tnx.execute( &creator );
	} catch( db::DbOperationException& ex ) {
	  throw runtime_error( string("Delegation failed: ") + ex.what() );
	}

    } else { // if( !found )

      /**
	 Check the remaining time of delegation. Should not be
	 less than 1 hour, because it means that the 
	 proxy renewal service hasn't been able to renew the
	 'super' better proxy.
      */
      
 

      CREAM_SAFE_LOG( m_log_dev->debugStream()
		      << method_name
		      << "FOUND delegation with key [" 
		      << str_sha1_digest << "]"
		      );

        // Delegation id FOUND. Returns it
        delegation_id   = deleg_info.m_delegation_id;

        CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << "Using existing delegation id ["
                        << delegation_id
                        << "] for CREAM URL ["
                        << cream_url
                        << "] Delegation URL ["
                        << cream_deleg_url
                        << "] user DN ["
                        << job.user_dn() <<"] MyProxy Server ["
			<< myproxy_address << "]"
                         );
    }
      
    //return make_pair(delegation_id, expiration_time);
    //return boost::make_tuple( delegation_id, expiration_time, duration );
    return delegation_id;

}


//______________________________________________________________________________
void 
util::Delegation_manager::updateDelegation( const boost::tuple<string, time_t, int>& newDeleg )
  throw( std::exception& )
{
  const char* method_name = "Delegation_manager::updateDelegation() - ";

  boost::recursive_mutex::scoped_lock L( s_mutex );
  
  bool found = false;
  table_entry tb;
  try {
    db::GetDelegationByID getter( newDeleg.get<0>(), method_name );
    db::Transaction tnx(false, false);
    //tnx.begin();
    tnx.execute( &getter );
    found = getter.found();
    if(found)
      tb = getter.get_delegation();
  } catch( db::DbOperationException& ex ) {
    throw runtime_error( string("Delegation failed: ") + ex.what() );
  }

  if( found )
    {
      CREAM_SAFE_LOG( m_log_dev->debugStream()
		      << method_name
		      << "Old Delegation was: ID=[" 
		      << tb.m_delegation_id << "] user_dn=["
		      << tb.m_user_dn << "] expiration time=["
		      << utilities::time_t_to_string(tb.m_expiration_time) << "] CEUrl=["
		      << tb.m_cream_url << "]"
		      );
      
      CREAM_SAFE_LOG( m_log_dev->debugStream()
		      << method_name
		      << "New Delegation id: ID=[" 
		      << tb.m_delegation_id << "] user_dn=["
		      << tb.m_user_dn << "] expiration time=["
		      << utilities::time_t_to_string(newDeleg.get<1>()) << "] CEUrl=["
		      << tb.m_cream_url << "]"
		      );
      try {
	db::UpdateDelegationTimesByID updater( newDeleg.get<0>(), newDeleg.get<1>(), newDeleg.get<2>(), method_name );
	db::Transaction tnx(false, false);
	//tnx.begin_exclusive( );
	tnx.execute( &updater );
      } catch( db::DbOperationException& ex ) {
	throw runtime_error( string("Delegation failed: ") + ex.what() );
      }
    }
  
}

//______________________________________________________________________________
void util::Delegation_manager::removeDelegation( const string& delegToRemove )
  throw( std::exception& )
{
  boost::recursive_mutex::scoped_lock L( s_mutex );

  CREAM_SAFE_LOG( m_log_dev->debugStream()
		  << "Delegation_manager::removeDelegation() - "
		  << "Removing Delegation ID [" 
		  << delegToRemove << "]"
		  );

  try {
    db::RemoveDelegationByID remover( delegToRemove, "Delegation_manager::removeDelegation" );
    db::Transaction tnx(false, false);
    //tnx.begin_exclusive();
    tnx.execute( &remover );
  } catch( db::DbOperationException& ex ) {
    throw runtime_error( string("Delegation failed: ") + ex.what() );
  }
  
}

//______________________________________________________________________________
void util::Delegation_manager::removeDelegation( const string& userDN, 
					   const string& myproxyurl )
  throw( std::exception& )
{
  boost::recursive_mutex::scoped_lock L( s_mutex );

  CREAM_SAFE_LOG( m_log_dev->debugStream()
		  << "Delegation_manager::removeDelegation() - "
		  << "Removing Delegation for DN [" 
		  << userDN << "] MyProxy URL ["
		  << myproxyurl << "]"
		  );

  try {
    db::RemoveDelegationByDNMyProxy remover( userDN, myproxyurl, "Delegation_manager::removeDelegation" );
    db::Transaction tnx(false, false);
    //tnx.begin_exclusive();
    tnx.execute( &remover );
  } catch( db::DbOperationException& ex ) {
    throw runtime_error( string("Delegation failed: ") + ex.what() );
  }
  
}

//______________________________________________________________________________
int util::Delegation_manager::getDelegationEntries( vector< table_entry >& target, 
					      const bool only_renewable )
  throw( std::exception& )
{
  boost::recursive_mutex::scoped_lock L( s_mutex );

  vector< table_entry > allDelegations;
  try {
    db::GetAllDelegation getter( only_renewable,  "Delegation_manager::getDelegationEntries" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );
    allDelegations = getter.get_delegations();
  } catch( db::DbOperationException& ex ) {
    throw runtime_error( string("Delegation failed: ") + ex.what() );
  }

  int counter = 0; 
  for(vector< table_entry >::const_iterator it=allDelegations.begin();
      it != allDelegations.end();
      ++it)
    {
      
      target.push_back( *it );
      ++counter;
    }
  return counter;
}

//----------------------------------------------------------------------------
void util::Delegation_manager::redelegate( const string& certfile, 
                                     const string& delegation_url,
                                     const string& delegation_id )
  throw( std::exception& )
{
    boost::recursive_mutex::scoped_lock L( s_mutex );

    static const char* method_name = "Delegation_manager::redelegate() - ";


    bool found = false;
    try {
      db::CheckDelegationByID checker( delegation_id, method_name );
      db::Transaction tnx(false, false);
      //tnx.begin();
      tnx.execute( &checker );
      found = checker.found();
    } catch( db::DbOperationException& ex ) {
      throw runtime_error( string("Delegation failed: ") + ex.what() );
    }

    if(!found) {
      CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
		      << "Could not find delegaion id ["
		      << delegation_id << "]. Giving up"
		      );
      abort(); // FIXME
    }

    try {
        CreamProxy_Delegate( delegation_url, certfile, delegation_id ).execute( 3 );
    } catch( exception& ex ) {
        // Delegation failed. Ignore it, it is supposed to fail
        CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                        << "Redelegation failed (as probably expected) "
                        << "with delegation id "
                        << delegation_id
                        << " Delegation URL "
                        << delegation_url
                        << " - ERROR is: ["
                        << ex.what() << "]. "
                        << "This error will be ignored"
                        );
        // ignore error
    }  catch( ... ) {
        CREAM_SAFE_LOG( m_log_dev->warnStream() << method_name
                        << "Redelegation failed (as probably expected) "
                        << "with delegation id "
                        << delegation_id
                        << " Delegation URL "
                        << delegation_url
                        << " - Unknown exception. " 
                        << "This error will be ignored"
                        );
        // throw runtime_error( "Delegation failed" );
    }     

}

//----------------------------------------------------------------------------
util::Delegation_manager::table_entry
util::Delegation_manager::getDelegation( const string& userdn, 
				   const string& ceurl, 
				   const string& myproxy ) 
  throw( std::exception& )
{  
  bool found = false;
  table_entry deleg_info("", "", 0, 0, "", "", 0, "");

  try {
    db::GetDelegation getter( userdn, ceurl, myproxy, "Delegation_manager::getDelegation" );
    db::Transaction tnx(false, false);
    tnx.execute( &getter );

    found = getter.found();
    if(found)
      deleg_info = getter.get_delegation();
  } catch( db::DbOperationException& ex ) {
    throw runtime_error( string("Delegation failed: ") + ex.what() );
  }
  
  //if( deleg_info.m_sha1_digest != "" ) {
  return deleg_info;
    //  }
}


//----------------------------------------------------------------------------
string util::Delegation_manager::generateDelegationID( ) throw()
{
  struct timeval T;
  ::gettimeofday( &T, 0 );

  //  ostringstream id;
  string id = util::utilities::to_string( T.tv_sec ) + "." + util::utilities::to_string(T.tv_usec) + utilities::getHostName();
  //id << T.tv_sec << "." << T.tv_usec << getHostName();
  //return id.str();
  return id;
}
