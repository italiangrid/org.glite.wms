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

//extern int errno;

extern int *__errno_location(void);
#define errno (*__errno_location())

namespace api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;
namespace fs = boost::filesystem;

using namespace std;
using namespace glite::wms::ice::util;

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
    m_proxy_renew( false )
{

}

//______________________________________________________________________________
void CreamJob::setJdl( const string& j ) throw( ClassadSyntax_ex& )
{
  /**
   * Classad-mutex protected region
   */
    boost::recursive_mutex::scoped_lock M_classad( glite::wms::ice::Ice::ClassAd_Mutex );
    classad::ClassAdParser parser;
    classad::ClassAd *jdlAd = parser.ParseClassAd( j );
    // int res = 0;

    if ( 0 == jdlAd ) {
        throw ClassadSyntax_ex( string("CreamJob::setJdl unable to parse jdl=[") + j + string("]") );
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
		       << log4cpp::CategoryStream::ENDLINE);
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
void CreamJob::setSequenceCode( const std::string& seq )
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
		   << "CreamJob::setSequenceCode() - ClassAdParser::ParseClassAd() returned a NULL pointer while parsing the jdl=["
		   << m_jdl << "]. STOP!"
		   << log4cpp::CategoryStream::ENDLINE);
    abort();
  }
  
  boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( jdl_ad );
  
  jdl_ad->InsertAttr( "LB_sequence_code", m_sequence_code );
  
  classad::ClassAdUnParser unparser;
  m_jdl.clear(); // This is necessary because apparently unparser.Unparse *appends* the serialization of jdl_ad to m_jdl
  unparser.Unparse( m_jdl, jdl_ad );
}

//______________________________________________________________________________
string CreamJob::getBetterProxy( void ) const
{
    string better = DNProxyManager::getInstance()->getBetterProxyByDN( m_user_dn );
    if( better.empty() ) return m_user_proxyfile;
    return better;
}

//______________________________________________________________________________
string CreamJob::getCEMonURL( void ) const 
{
    string cemon_url;
    subscriptionManager* submgr( subscriptionManager::getInstance() );
    submgr->getCEMonURL( getBetterProxy(), m_cream_address, cemon_url );
    return cemon_url;
}

//______________________________________________________________________________
string CreamJob::getSubscriptionID( void ) const
{
    subscriptionManager* submgr( subscriptionManager::getInstance() );
    iceSubscription subscription;
    string cemon_url( getCEMonURL() );
    // Gets the CEMon Subscription ID from the (user_dn, cemon_url) pair. FIXME: should we check the returned value?
    submgr->getSubscriptionByDNCEMon( m_user_dn, cemon_url, subscription );
    return subscription.getSubscriptionID();
}

// Returns the DN for the CEMon which send snotifications for this job
string CreamJob::get_cemon_dn( void ) const
{
  subscriptionManager* submgr( subscriptionManager::getInstance() );
  string cemondn;
  string cemon_url( getCEMonURL() );
  // Gets the CEMon Subscription ID from the (user_dn, cemon_url) pair. FIXME: should we check the returned value?
  submgr->getCEMonDN( m_user_dn, cemon_url, cemondn );
  return cemondn;
}

//______________________________________________________________________________
string CreamJob::getCompleteCreamJobID( void ) const 
{
  if( this->getCreamURL().empty() ) return "";

  string creamURL = this->getCreamURL();

//   string::size_type loc = creamURL.find(iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix());
  
//   if( loc != string::npos)
//     {
//       creamURL = creamURL.substr(0,loc);
//     }

  boost::replace_all( creamURL, iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix(), "" );

  creamURL += "/" + this->getCreamJobID();

  return creamURL;
}
