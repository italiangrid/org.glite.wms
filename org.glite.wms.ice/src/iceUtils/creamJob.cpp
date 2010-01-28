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

/**
 *
 * ICE Headers
 *
 */
#include "creamJob.h"
#include "iceConfManager.h"
#include "DNProxyManager.h"
#include "subscriptionManager.h"
#include "ice-core.h"

/**
 *
 * WMS and CE Headers
 *
 */
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"

/**
 *
 * ClassAd Headers
 *
 */
#include "classad_distribution.h"

/**
 *
 * Boost Headers
 *
 */
#include <boost/filesystem/operations.hpp>
//#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/regex.hpp>

/**
 *
 * OS and STL C++ Header
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>

namespace api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;
namespace fs = boost::filesystem;

using namespace std;
using namespace glite::wms::ice::util;

//boost::recursive_mutex CreamJob::serialize_mutex;

//______________________________________________________________________________
CreamJob::CreamJob( ) :
    m_prev_status( api::job_statuses::UNKNOWN ),
    m_status( api::job_statuses::UNKNOWN ),
    m_num_logged_status_changes( 0 ),
    m_last_seen( time(0) ),
    m_statusPollRetryCount( 0 ),
    m_exit_code( 0 ),
    m_is_killed_byice( false ),
    m_last_empty_notification( time(0) ),
    m_proxy_renew( false ),
    m_myproxy_address( "" ),
    m_complete_cream_jobid( "" )
{
}

//______________________________________________________________________________
CreamJob::CreamJob( const std::string& gid,
		    const std::string& cid,
		    const std::string& ccid,
		    const std::string& jdl,
		    const std::string& userproxy,
		    const std::string& ceid,
		    const std::string& endpoint,
		    const std::string& creamurl,
		    const std::string& creamdelegurl,
		    const std::string& userdn,
		    const std::string& myproxyurl,
		    const std::string& proxy_renewable,
		    const std::string& failure_reason,
		    const std::string& sequence_code,
		    const std::string& wn_sequence_code,
		    const std::string& prev_status,
		    const std::string& status,
		    const std::string& num_logged_status_changes,
		    const std::string& leaseid,
		    const std::string& status_poller_retry_count,
		    const std::string& exit_code,
		    const std::string& worker_node,
		    const std::string& is_killed_byice,
		    const std::string& delegationid,
		    const std::string& last_empty_notification,
		    const std::string& last_seen,
		    const std::string& isbproxy_time_end,
		    const std::string& modif_jdl)
  : m_cream_jobid( cid ),
    m_grid_jobid( gid ),
    m_jdl( jdl ),
    m_ceid( ceid ),
    m_endpoint( endpoint ),
    m_cream_address( creamurl ),
    m_cream_deleg_address( creamdelegurl ),
    m_user_proxyfile( userproxy ),
    m_user_dn( userdn ),
    m_sequence_code( sequence_code ),
    m_delegation_id( delegationid ),
    m_wn_sequence_code( wn_sequence_code ),
    m_prev_status( (glite::ce::cream_client_api::job_statuses::job_status)atoi(prev_status.c_str()) ),
    m_status( (glite::ce::cream_client_api::job_statuses::job_status)atoi(status.c_str())),
    m_num_logged_status_changes( atoi(num_logged_status_changes.c_str()) ),
    m_last_seen( (time_t)atoi(last_seen.c_str()) ),
    m_lease_id( leaseid ),
    m_statusPollRetryCount( atoi(status_poller_retry_count.c_str()) ),
    m_exit_code( atoi(exit_code.c_str()) ),
    m_failure_reason( failure_reason ),
    m_worker_node( worker_node ),
    m_is_killed_byice( (bool)atoi(is_killed_byice.c_str()) ),
    m_last_empty_notification( (time_t)atoi(last_empty_notification.c_str()) ),
    m_proxy_renew( (bool)atoi(proxy_renewable.c_str()) ),
    m_myproxy_address( myproxyurl ),
    m_complete_cream_jobid( ccid ),
    m_isbproxy_time_end( (time_t)atoi(isbproxy_time_end.c_str() )),
    m_modified_jdl( modif_jdl )
{
}

//______________________________________________________________________________
void CreamJob::set_jdl( const string& j ) throw( ClassadSyntax_ex& )
{
  /**
   * Classad-mutex protected region
   */
  boost::recursive_mutex::scoped_lock M_classad( glite::wms::ice::Ice::ClassAd_Mutex );
  classad::ClassAdParser parser;
  classad::ClassAd *jdlAd = parser.ParseClassAd( j );
  
  if ( 0 == jdlAd ) {
    throw ClassadSyntax_ex( string("unable to parse jdl=[") + j + string("]") );
  }
  
  boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( jdlAd );
  
  m_jdl = j;
  
  // Look for the "ce_id" attribute
  if ( !classad_safe_ptr->EvaluateAttrString( "ce_id", m_ceid ) ) {
    throw ClassadSyntax_ex("ce_id attribute not found, or is not a string");
  }
  boost::trim_if(m_ceid, boost::is_any_of("\"") );
  
  // Look for the "X509UserProxy" attribute
  if ( !classad_safe_ptr->EvaluateAttrString( "X509UserProxy", m_user_proxyfile ) ) {
    throw ClassadSyntax_ex("X509UserProxy attribute not found, or is not a string");
  }
  
  string tmp;
  if ( classad_safe_ptr->EvaluateAttrString( "MYPROXYSERVER", tmp ) ) {
    m_proxy_renew = true;
    m_myproxy_address = tmp;
  } else {
    m_proxy_renew = false;
  }
  
  boost::trim_if(m_user_proxyfile, boost::is_any_of("\""));
  
  // Look for the "LBSequenceCode" attribute (if this attribute is not in the classad, the sequence code is set to the empty string
  if ( classad_safe_ptr->EvaluateAttrString( "LB_sequence_code", m_sequence_code ) ) {
    boost::trim_if(m_sequence_code, boost::is_any_of("\""));
  }
  
  // Look for the "edg_jobid" attribute
  if ( !classad_safe_ptr->EvaluateAttrString( "edg_jobid", m_grid_jobid ) ) {
    throw ClassadSyntax_ex( "edg_jobid attribute not found, or is not a string" );
  }
  boost::trim_if(m_grid_jobid, boost::is_any_of("\"") );
  
  vector<string> pieces;
  try{
    api::util::CEUrl::parseCEID(m_ceid, pieces);
  } catch(api::util::CEUrl::ceid_syntax_ex& ex) {
    throw ClassadSyntax_ex(ex.what());
  }
  
  m_endpoint = pieces[0] + ":" + pieces[1];
  
  m_cream_address = iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_prefix() 
    + m_endpoint + iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix();
  
  m_cream_deleg_address = iceConfManager::getInstance()->getConfiguration()->ice()->creamdelegation_url_prefix() 
    + m_endpoint + iceConfManager::getInstance()->getConfiguration()->ice()->creamdelegation_url_postfix();
  
  // It is important to get the jdl from the job itself, rather
  // than using the m_jdl attribute. This is because the
  // sequence_code attribute inside the jdl classad has been
  // modified by the L&B calls, and we have to pass to CREAM the
  // "last" sequence code as the job wrapper will need to log
  // the "really running" event.
  creamJdlHelper( this->get_jdl(), m_modified_jdl );// can throw ClassadSyntax_ex
  
  // release of Classad-mutex
}

//______________________________________________________________________________
bool CreamJob::is_active( void ) const
{
    if( this->is_killed_by_ice() ) return false;

    return ( ( m_status == api::job_statuses::REGISTERED ) ||
             ( m_status == api::job_statuses::PENDING ) ||
             ( m_status == api::job_statuses::IDLE ) ||
             ( m_status == api::job_statuses::RUNNING ) ||
	     ( m_status == api::job_statuses::REALLY_RUNNING) ||
             ( m_status == api::job_statuses::HELD ) );
}

//______________________________________________________________________________
bool CreamJob::can_be_purged( void ) const
{
    return ( ( m_status == api::job_statuses::DONE_OK ) ||
             ( m_status == api::job_statuses::CANCELLED ) ||
             ( m_status == api::job_statuses::DONE_FAILED ) ||
             ( m_status == api::job_statuses::ABORTED ) );
}

//______________________________________________________________________________
bool CreamJob::can_be_resubmitted( void ) const
{ 
    int threshold( iceConfManager::getInstance()->getConfiguration()->ice()->job_cancellation_threshold_time() );
    api::soap_proxy::VOMSWrapper V( this->get_user_proxy_certificate() );
    if ( !V.IsValid() || 
         ( V.getProxyTimeEnd() < time(0) + threshold ) ) {
        return false;
    }
    return ( ( m_status == api::job_statuses::DONE_FAILED ) ||
             ( m_status == api::job_statuses::ABORTED ) );
}

//______________________________________________________________________________
string CreamJob::describe( void ) const
{
    string result;
    result.append( "gridJobID=\"" );
    result.append( m_grid_jobid );
    result.append( "\" CREAMJobID=\"" );
    result.append( m_complete_cream_jobid );
    result.append( "\"" );
    return result;
}

//______________________________________________________________________________
void CreamJob::set_sequence_code( const std::string& seq )
{
  /**
   * mutex-protected region: REM that ClassAd is not
   * thread-safe
   */
  boost::recursive_mutex::scoped_lock M_classad( Ice::ClassAd_Mutex );
  
  m_sequence_code = seq;
  string old_jdl( m_jdl );
  
  // Update the jdl
  classad::ClassAdParser parser;
  classad::ClassAd* jdl_ad = parser.ParseClassAd( m_jdl );
  
  if (!jdl_ad) {
    CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->fatalStream()
		   << "CreamJob::set_sequencecode() - ClassAdParser::ParseClassAd() "
		   << "returned a NULL pointer while parsing the jdl=["
		   << m_jdl << "]. STOP!"
		   );
    abort();
  }
  
  boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( jdl_ad );
  
  jdl_ad->InsertAttr( "LB_sequence_code", m_sequence_code );
  
  classad::ClassAdUnParser unparser;
  m_jdl.clear(); // This is necessary because apparently unparser.Unparse *appends* the serialization of jdl_ad to m_jdl
  unparser.Unparse( m_jdl, jdl_ad );
}
