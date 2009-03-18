/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE cream job
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

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
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

/**
 *
 * OS and STL C++ Header
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <cerrno>

extern int errno;

//extern int *__errno_location(void);
//#define errno (*__errno_location())

namespace api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;
namespace fs = boost::filesystem;

using namespace std;
using namespace glite::wms::ice::util;

boost::recursive_mutex CreamJob::serialize_mutex;
boost::recursive_mutex CreamJob::globalICEMutex;

//______________________________________________________________________________
CreamJob::CreamJob( ) :
    m_prev_status( api::job_statuses::UNKNOWN ),
    m_status( api::job_statuses::UNKNOWN ),
    m_num_logged_status_changes( 0 ),
    m_last_seen( time(0) ),
    m_statusPollRetryCount( 0 ),
    m_exit_code( 0 ),
    m_is_killed_by_ice( false ),
    m_last_empty_notification( time(0) ),
    m_proxy_renew( false ),
    m_myproxy_address( "" )
{

}

//______________________________________________________________________________
CreamJob::CreamJob( const std::string& gid,
		    const std::string& cid,
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
		    const std::string& proxycert_timestamp,
		    const std::string& status_poller_retry_count,
		    const std::string& exit_code,
		    const std::string& worker_node,
		    const std::string& is_killed_byice,
		    const std::string& delegationid,
		    const std::string& delegation_exptime,
		    const std::string& delegation_duration,
		    const std::string& last_empty_notification,
		    const std::string& last_seen) 
{
  m_cream_jobid                = cid;
  m_grid_jobid                 = gid; 
  m_jdl                        = jdl;
  m_ceid                       = ceid;
  m_endpoint                   = endpoint;
  m_cream_address              = creamurl;
  m_cream_deleg_address        = creamdelegurl; 
  m_user_proxyfile             = userproxy;
  m_user_dn                    = userdn;
  m_sequence_code              = sequence_code;
  m_delegation_id              = delegationid; 
  m_delegation_exptime         = (time_t)atoi(delegation_exptime.c_str());
  m_delegation_duration        = atoi(delegation_duration.c_str());
  m_wn_sequence_code           = wn_sequence_code;
  m_prev_status                = (glite::ce::cream_client_api::job_statuses::job_status)atoi(prev_status.c_str());
  m_status                     = (glite::ce::cream_client_api::job_statuses::job_status)atoi(status.c_str());
  m_num_logged_status_changes  = atoi(num_logged_status_changes.c_str());
  m_last_seen                  = (time_t)atoi(last_seen.c_str());
  m_lease_id                   = leaseid;
  m_proxyCertTimestamp         = (time_t)atoi(proxycert_timestamp.c_str());
  m_statusPollRetryCount       = atoi(status_poller_retry_count.c_str());
  m_exit_code                  = atoi(exit_code.c_str());
  m_failure_reason             = failure_reason;
  m_worker_node                = worker_node;
  m_is_killed_by_ice           = (bool)atoi(is_killed_byice.c_str());
  m_last_empty_notification    = (time_t)atoi(last_empty_notification.c_str());
  m_proxy_renew                = (bool)atoi(proxy_renewable.c_str());
  m_myproxy_address            = myproxyurl;
}

//______________________________________________________________________________
CreamJob::CreamJob( const vector< string >& src ) 
{
  m_grid_jobid                 = src.at(0);//cid;
  m_cream_jobid                = src.at(1);//gid; 
  m_jdl                        = src.at(3);//jdl;
  m_user_proxyfile	       = src.at(4);
  m_ceid                       = src.at(5);//ceid;
  m_endpoint                   = src.at(6);//endpoint;
  m_cream_address              = src.at(7);//creamurl;
  m_cream_deleg_address        = src.at(8);//creamdelegurl; 
  m_user_dn 	               = src.at(9);//userproxy;
  m_myproxy_address	       = src.at(10);
  m_proxy_renew                = (bool)atoi(src.at(11).c_str());
  m_failure_reason             = src.at(12);
  m_sequence_code              = src.at(13);
  m_wn_sequence_code           = src.at(14);
  m_prev_status                = (glite::ce::cream_client_api::job_statuses::job_status)atoi(src.at(15).c_str());
  m_status                     = (glite::ce::cream_client_api::job_statuses::job_status)atoi(src.at(16).c_str());
  m_num_logged_status_changes  = atoi(src.at(17).c_str());
  m_lease_id                   = src.at(18);
  m_proxyCertTimestamp         = (time_t)atoi(src.at(19).c_str());
  m_statusPollRetryCount       = atoi(src.at(20).c_str());
  m_exit_code                  = atoi(src.at(21).c_str());
  m_worker_node                = src.at(22);
  m_is_killed_by_ice           = (bool)atoi(src.at(23).c_str());
  m_delegation_id              = src.at(24);
  m_delegation_exptime         = (time_t)atoi(src.at(25).c_str());
  m_delegation_duration        = atoi(src.at(26).c_str());
  m_last_empty_notification    = (time_t)atoi(src.at(27).c_str());
  m_last_seen                  = (time_t)atoi(src.at(28).c_str());
  
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
    // int res = 0;

    if ( 0 == jdlAd ) {
        throw ClassadSyntax_ex( string("CreamJob::set_jdl unable to parse jdl=[") + j + string("]") );
    }

    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( jdlAd );

    m_jdl = j;

    // Look for the "ce_id" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "ce_id", m_ceid ) ) {
        throw ClassadSyntax_ex("CreamJob::setJdl: ce_id attribute not found, or is not a string");
    }
    boost::trim_if(m_ceid, boost::is_any_of("\"") );
    
    // Look for the "X509UserProxy" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "X509UserProxy", m_user_proxyfile ) ) {
        throw ClassadSyntax_ex("CreamJob::setJdl: X509UserProxy attribute not found, or is not a string");
    }
    
    string tmp;
    if ( classad_safe_ptr->EvaluateAttrString( "MYPROXYSERVER", tmp ) ) {
      m_proxy_renew = true;
      m_myproxy_address = tmp;
    } else {
      m_proxy_renew = false;
    }
    
    boost::trim_if(m_user_proxyfile, boost::is_any_of("\""));
    
    struct stat stat_buf;
    if( ::stat( m_user_proxyfile.c_str(), &stat_buf ) == -1 )
    {
	int saverr = errno;
	CREAM_SAFE_LOG(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->warnStream()
		       << "creamJob::setJdl() - The user proxy file ["
		       << m_user_proxyfile << "] is not stat-able:" << strerror(saverr) 
		       <<". This could compromise the correct working of proxy renewal thread"
		       );
    } else {
	m_proxyCertTimestamp = stat_buf.st_mtime;
    }

    // Look for the "LBSequenceCode" attribute (if this attribute is not in the classad, the sequence code is set to the empty string
    if ( classad_safe_ptr->EvaluateAttrString( "LB_sequence_code", m_sequence_code ) ) {
        boost::trim_if(m_sequence_code, boost::is_any_of("\""));
    }
    
    // Look for the "edg_jobid" attribute
    if ( !classad_safe_ptr->EvaluateAttrString( "edg_jobid", m_grid_jobid ) ) {
        throw ClassadSyntax_ex( "CreamJob::setJdl: edg_jobid attribute not found, or is not a string" );
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
    api::soap_proxy::VOMSWrapper V( getUserProxyCertificate() );
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
    result.append( getGridJobID() );
    result.append( "\" CREAMJobID=\"" );
    result.append( getCompleteCreamJobID() );
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
		   << "CreamJob::set_sequencecode() - ClassAdParser::ParseClassAd() returned a NULL pointer while parsing the jdl=["
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

//______________________________________________________________________________
// string CreamJob::getBetterProxy( void ) const
// {
//   //string better = DNProxyManager::getInstance()->getBetterProxyByDN( m_user_dn ).get<0>();
//   string better = DNProxyManager::getInstance()->getBetterProxy( m_user_dn ).get<0>();
//   if( better.empty() ) return m_user_proxyfile;
//   return better;
// }

//______________________________________________________________________________
// string CreamJob::_getCEMonURL( void ) const 
// {
//     string cemon_url;
//     subscriptionManager* submgr( subscriptionManager::getInstance() );
// 
//     string proxy = DNProxyManager::getInstance()->getAnyBetterProxyByDN( this->getUserDN() ).get<0>();
// 
//     submgr->getCEMonURL( proxy, m_cream_address, cemon_url );
//     return cemon_url;
// }

//______________________________________________________________________________
// string CreamJob::_getSubscriptionID( void ) const
// {
//     subscriptionManager* submgr( subscriptionManager::getInstance() );
//     iceSubscription subscription;
//     string cemon_url( _getCEMonURL() );
//     // Gets the CEMon Subscription ID from the (user_dn, cemon_url) pair. FIXME: should we check the returned value?
//     submgr->getSubscriptionByDNCEMon( m_user_dn, cemon_url, subscription );
//     return subscription.getSubscriptionID();
// }

//______________________________________________________________________________
// Returns the DN for the CEMon which send snotifications for this job
// string CreamJob::_get_cemon_dn( void ) const
// {
//   subscriptionManager* submgr( subscriptionManager::getInstance() );
//   string cemondn;
//   string cemon_url( _getCEMonURL() );
//   // Gets the CEMon Subscription ID from the (user_dn, cemon_url) pair. FIXME: should we check the returned value?
//   submgr->getCEMonDN( m_user_dn, cemon_url, cemondn );
//   return cemondn;
// }

//______________________________________________________________________________
string CreamJob::getCompleteCreamJobID( void ) const 
{
  if ( getCreamURL().empty() || getCreamJobID().empty() ) 
      return "";

  string creamURL = this->getCreamURL();

  boost::replace_all( creamURL, iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix(), "" );

  creamURL += "/" + this->getCreamJobID();

  return creamURL;
}

//______________________________________________________________________________
void CreamJob::get_fields( vector<string>& target ) const
{
  ostringstream tmp("");
  target.push_back( m_grid_jobid );
  target.push_back( m_cream_jobid );
  target.push_back( getCompleteCreamJobID() );
  target.push_back( m_jdl );
  target.push_back( m_user_proxyfile );
  target.push_back( m_ceid );
  target.push_back( m_endpoint );
  target.push_back( m_cream_address );
  target.push_back( m_cream_deleg_address );
  target.push_back( m_user_dn );
  target.push_back( m_myproxy_address );
  target.push_back( (m_proxy_renew ? "1" : "0" ) );
  target.push_back( m_failure_reason );
  target.push_back( m_sequence_code );
  target.push_back( m_wn_sequence_code );
  tmp << m_prev_status;
  target.push_back( tmp.str() );
  tmp.str("");
  tmp << m_status;
  target.push_back( tmp.str() );
  tmp.str("");
  tmp << m_num_logged_status_changes;
  target.push_back( tmp.str() );
  target.push_back( m_lease_id );
  tmp.str("");
  tmp << m_proxyCertTimestamp;
  target.push_back( tmp.str() );
  tmp.str("");
  tmp << m_statusPollRetryCount;
  target.push_back( tmp.str() );
  tmp.str("");
  tmp << m_exit_code;
  target.push_back( tmp.str() );
  target.push_back( m_worker_node );
  target.push_back( (m_is_killed_by_ice ? "1" : "0" ));
  target.push_back( m_delegation_id );
  tmp.str("");
  tmp << m_delegation_exptime;
  target.push_back( tmp.str() );
  tmp.str("");
  tmp << m_delegation_duration;
  target.push_back( tmp.str() );
  tmp.str("");
  tmp << m_last_empty_notification;
  target.push_back( tmp.str() );
  tmp.str("");
  tmp << m_last_seen;
  target.push_back( tmp.str() );

}
