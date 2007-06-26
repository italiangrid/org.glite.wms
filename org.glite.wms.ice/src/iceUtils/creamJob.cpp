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
#include "creamJob.h"
#include "iceConfManager.h"
#include "DNProxyManager.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include "classad_distribution.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

/* includes needed by stat function */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>

extern int errno;

namespace api = glite::ce::cream_client_api;
namespace api_util = glite::ce::cream_client_api::util;
namespace fs = boost::filesystem;

using namespace std;
using namespace glite::wms::ice::util;

//______________________________________________________________________________
CreamJob::CreamJob( ) :
    m_status( api::job_statuses::UNKNOWN ),
    m_num_logged_status_changes( 0 ),
    m_last_seen( time(0) ),
    m_last_empty_notification( time(0) ),
    m_end_lease( 0 ), 
    m_statusPollRetryCount( 0 ),
    m_exit_code( 0 ),
    m_is_killed_by_ice( false )
{

}

//______________________________________________________________________________
CreamJob::CreamJob( const std::string& ad ) throw ( ClassadSyntax_ex& ) 
{
    unserialize( ad );
}

//______________________________________________________________________________
/*CreamJob::CreamJob( const CreamJob& aJob ) throw()
{
  m_cream_jobid = aJob.m_cream_jobid;
  m_grid_jobid = aJob.m_grid_jobid;
  m_jdl = aJob.m_jdl;
  m_ceid = aJob.m_ceid;
  m_endpoint = aJob.m_endpoint;
  m_cream_address = aJob.m_cream_address;
  m_cream_deleg_address = aJob.m_cream_deleg_address;
  m_user_proxyfile = aJob.m_user_proxyfile;
  m_user_dn = aJob.m_user_dn;
  m_sequence_code = aJob.m_sequence_code;
  m_delegation_id   = aJob.   m_delegation_id;
  m_wn_sequence_code  = aJob.m_wn_sequence_code;
  m_status = aJob.m_status;
  m_num_logged_status_changes  = aJob.m_num_logged_status_changes;
  m_last_seen = aJob.m_last_seen;
  m_end_lease  = aJob.m_end_lease;
  m_proxyCertTimestamp  = aJob.m_proxyCertTimestamp;
  m_statusPollRetryCount  = aJob.m_statusPollRetryCount;
  m_exit_code = aJob.m_exit_code;
  m_failure_reason = aJob.m_failure_reason;
  m_worker_node = aJob.m_worker_node;
  m_is_killed_by_ice = aJob.m_is_killed_by_ice;

  CREAM_SAFE_LOG(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->debugStream()
		 << "CreamJob::COPYCTOR() - ALVISE-DEBUG CALLED FOR JOB ["
		 << aJob.describe() << "]"
		 << log4cpp::CategoryStream::ENDLINE);
}*/

//______________________________________________________________________________
string CreamJob::serialize( void ) const
{
    string res;

    classad::ClassAd ad;
    ad.InsertAttr( "cream_jobid", m_cream_jobid );
    ad.InsertAttr( "status", m_status );
    ad.InsertAttr( "exit_code", m_exit_code );
    ad.InsertAttr( "failure_reason", m_failure_reason );
    ad.InsertAttr( "delegation_id", m_delegation_id );
    ad.InsertAttr( "wn_sequence_code", m_wn_sequence_code );
    ad.InsertAttr( "num_logged_status_changes", m_num_logged_status_changes );
    ad.InsertAttr( "worker_node", m_worker_node );
    ad.InsertAttr( "is_killed_by_ice", m_is_killed_by_ice );
    ad.InsertAttr( "user_dn", m_user_dn);

    classad::ClassAdParser parser;
    classad::ClassAd* jdlAd = parser.ParseClassAd( m_jdl );
    if(!jdlAd) {
      CREAM_SAFE_LOG(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->fatalStream()
		       << "CreamJob::serialize() - ClassAdParser::ParseClassAd() returned a NULL pointer. STOP!"
		       << log4cpp::CategoryStream::ENDLINE);
      abort();
    }
    
    //boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( jdlAd );
    
    // Updates sequence code
    jdlAd->InsertAttr( "LB_sequence_code", m_sequence_code );
    ad.Insert( "jdl", jdlAd );

    try {    
        ad.InsertAttr( "last_seen", boost::lexical_cast< string >(m_last_seen) );
        ad.InsertAttr( "end_lease" , boost::lexical_cast< string >(m_end_lease) );
	ad.InsertAttr( "lastmodiftime_proxycert", boost::lexical_cast< string >( m_proxyCertTimestamp ) );
    } catch( boost::bad_lexical_cast& ) {
        // Should never happen... FIXME
    }

    classad::ClassAdUnParser unparser;
    unparser.Unparse( res, &ad );
    return res;
}

//______________________________________________________________________________
void CreamJob::unserialize( const std::string& buf ) throw( ClassadSyntax_ex& )
{
    classad::ClassAdParser parser;

    classad::ClassAd *ad;
    classad::ClassAd *jdlAd; // no need to free it
    int st_number;
    string tstamp; // last status change
    string elease; // end lease
    string lseen; // last seen
    string lemptynotif; // last empty notification
    string lastmtime_proxy; // proxyCertTimestamp

    ad = parser.ParseClassAd( buf );

    if(!ad)
        throw ClassadSyntax_ex(string("ClassAd parser returned a NULL pointer parsing entire classad ")+buf);
  
    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( ad );
  
    if ( ! classad_safe_ptr->EvaluateAttrString( "cream_jobid", m_cream_jobid ) ||
         ! classad_safe_ptr->EvaluateAttrNumber( "status", st_number ) ||
         ! classad_safe_ptr->EvaluateAttrNumber( "exit_code", m_exit_code ) || 
         ! classad_safe_ptr->EvaluateAttrClassAd( "jdl", jdlAd ) ||
         ! classad_safe_ptr->EvaluateAttrNumber( "num_logged_status_changes", m_num_logged_status_changes ) ||
         ! classad_safe_ptr->EvaluateAttrString( "last_seen", lseen ) ||
         ! classad_safe_ptr->EvaluateAttrString( "end_lease", elease ) ||
	 ! classad_safe_ptr->EvaluateAttrString( "lastmodiftime_proxycert", lastmtime_proxy) ||
         ! classad_safe_ptr->EvaluateAttrString( "delegation_id", m_delegation_id ) ||
         ! classad_safe_ptr->EvaluateAttrString( "wn_sequence_code", m_wn_sequence_code ) ||
         ! classad_safe_ptr->EvaluateAttrString( "failure_reason", m_failure_reason ) ||
         ! classad_safe_ptr->EvaluateAttrString( "worker_node", m_worker_node ) ||
         ! classad_safe_ptr->EvaluateAttrBool( "is_killed_by_ice", m_is_killed_by_ice ) ||
	 ! classad_safe_ptr->EvaluateAttrString( "user_dn", m_user_dn) ||
         ! classad_safe_ptr->EvaluateAttrString( "last_empty_notification", lemptynotif )) {

        throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for one of the following attributes: grid_jobid, status, exit_code, jdl, num_logged_status_changes, last_seen, end_lease, lastmodiftime_proxycert, delegation_id, wn_sequence_code, failure_reason, worker_node, is_killed_by_ice, user_dn, last_empty_notifications" );

    }
    m_status = (api::job_statuses::job_status)st_number;
    boost::trim_if( tstamp, boost::is_any_of("\"" ) );
    boost::trim_if( elease, boost::is_any_of("\"" ) );
    boost::trim_if( lseen, boost::is_any_of("\"" ) );
    boost::trim_if( lemptynotif, boost::is_any_of("\"" ) );
    boost::trim_if( lastmtime_proxy, boost::is_any_of("\"" ) );
    boost::trim_if( m_delegation_id, boost::is_any_of("\"") );
    boost::trim_if( m_wn_sequence_code, boost::is_any_of("\"") );
    boost::trim_if( m_failure_reason, boost::is_any_of("\"") );
    boost::trim_if( m_worker_node, boost::is_any_of("\"") );
    boost::trim_if( m_user_dn, boost::is_any_of("\"") );

    try {
        m_last_empty_notification = boost::lexical_cast< time_t >( lemptynotif );
        m_end_lease = boost::lexical_cast< time_t >( elease );
        m_last_seen = boost::lexical_cast< time_t >( lseen );
	m_proxyCertTimestamp = boost::lexical_cast< time_t >( lastmtime_proxy );
    } catch( boost::bad_lexical_cast& ) {
        throw ClassadSyntax_ex( "CreamJob::unserialize() is unable to cast [" + tstamp + "] or [" +elease+"] or [" +lseen + "] or [" +lastmtime_proxy + "] or ["+lemptynotif +"] to time_t" );
    }
    boost::trim_if(m_cream_jobid, boost::is_any_of("\""));

    classad::ClassAdUnParser unparser;
    string jdl_string;
    unparser.Unparse( jdl_string, jdlAd ); // FIXME: Unparsing & Parsing is not good...

    setJdl( jdl_string );
}

//______________________________________________________________________________
void CreamJob::setJdl( const string& j ) throw( ClassadSyntax_ex& )
{
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

    /**
     * No need to lock the mutex because getInstance already does that
     */
    //iceConfManager* conf = iceConfManager::getInstance();

    m_cream_address = iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_prefix() 
	+ m_endpoint + iceConfManager::getInstance()->getConfiguration()->ice()->cream_url_postfix();
    m_cream_deleg_address = iceConfManager::getInstance()->getConfiguration()->ice()->creamdelegation_url_prefix() 
	+ m_endpoint + iceConfManager::getInstance()->getConfiguration()->ice()->creamdelegation_url_postfix();
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

bool CreamJob::can_be_purged( void ) const
{
    return ( ( m_status == api::job_statuses::DONE_OK ) ||
             ( m_status == api::job_statuses::CANCELLED ) ||
             ( m_status == api::job_statuses::DONE_FAILED ) ||
             ( m_status == api::job_statuses::ABORTED ) );
}


bool CreamJob::can_be_resubmitted( void ) const
{ 
    return ( ( m_status == api::job_statuses::DONE_FAILED ) ||
             ( m_status == api::job_statuses::ABORTED ) );
}

string CreamJob::describe( void ) const
{
    string result;
    result.append( "gridJobID=\"" );
    result.append( getGridJobID() );
    result.append( "\" CREAMJobID=\"" );
    result.append( getCreamJobID() );
    result.append( "\"" );
    return result;
}

void CreamJob::setSequenceCode( const std::string& seq )
{
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
    jdl_ad->InsertAttr( "LB_sequence_code", m_sequence_code );

    classad::ClassAdUnParser unparser;
    m_jdl.clear(); // This is necessary because apparently unparser.Unparse *appends* the serialization of jdl_ad to m_jdl
    unparser.Unparse( m_jdl, jdl_ad );

    // CREAM_SAFE_LOG( api_util::creamApiLogger::instance()->getLogger()->infoStream()
    //                     << "CreamJob::setSequenceCode() - "
    //                     << "old jdl=[" << old_jdl << "] new jdl=["
    //                     << m_jdl << "]"
    //                     << log4cpp::CategoryStream::ENDLINE);
}

size_t CreamJob::size( void ) const 
{
  size_t size = 0;
  size  = sizeof(glite::ce::cream_client_api::job_statuses::job_status);
  size += sizeof(m_num_logged_status_changes);
  size += sizeof(m_last_seen);
  size += sizeof(m_end_lease);
  size += sizeof(m_proxyCertTimestamp);
  size += sizeof(m_statusPollRetryCount);
  size += sizeof(m_exit_code);
  size += sizeof(m_is_killed_by_ice);
  size =+ sizeof(m_last_empty_notification);
  
  size += m_failure_reason.capacity() + m_worker_node.capacity() + m_wn_sequence_code.capacity();
  size += m_delegation_id.capacity() + m_sequence_code.capacity() + m_user_dn.capacity();
  size += m_user_proxyfile.capacity() + m_cream_deleg_address.capacity() + m_cream_address.capacity();
  size += m_endpoint.capacity() + m_ceid.capacity() + m_jdl.capacity() + m_grid_jobid.capacity();
  size += m_cream_jobid.capacity();

  size += 14*sizeof(string);
  
  //cout << "creamJob size="<<size<<endl;
  return size;
}

string CreamJob::getBetterProxy( void ) const
{
    return DNProxyManager::getInstance()->getBetterProxyByDN( m_user_dn );
}
