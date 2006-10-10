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

extern int errno;

using namespace std;

namespace iceUtil = glite::wms::ice::util;
namespace api = glite::ce::cream_client_api;
namespace fs = boost::filesystem;

//______________________________________________________________________________
iceUtil::CreamJob::CreamJob( ) :
    m_status( api::job_statuses::UNKNOWN ),
    m_num_logged_status_changes( 0 ),
    m_last_seen( time(0) ),
    m_end_lease( m_last_seen + 60*30 ), // FIXME: remove hardcoded default
    m_statusPollRetryCount( 0 ),
    m_exit_code( 0 ),
    m_is_killed_by_ice( false )
{

}

//______________________________________________________________________________
iceUtil::CreamJob::CreamJob( const std::string& ad ) throw ( ClassadSyntax_ex& ) 
{
    unserialize( ad );
}

//______________________________________________________________________________
string iceUtil::CreamJob::serialize( void ) const
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
void iceUtil::CreamJob::unserialize( const std::string& buf ) throw( ClassadSyntax_ex& )
{
    classad::ClassAdParser parser;

    classad::ClassAd *ad;
    classad::ClassAd *jdlAd; // no need to free it
    int st_number;
    string tstamp; // last status change
    string elease; // end lease
    string lseen; // last seen
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
         ! classad_safe_ptr->EvaluateAttrBool( "is_killed_by_ice", m_is_killed_by_ice ) ) {

        throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for one of the following attributes: grid_jobid, status, exit_code, jdl, num_logged_status_changes, last_seen, end_lease, lastmodiftime_proxycert, delegation_id, wn_sequence_code, failure_reason, worker_node, is_killed_by_ice" );

    }
    m_status = (api::job_statuses::job_status)st_number;
    boost::trim_if( tstamp, boost::is_any_of("\"" ) );
    boost::trim_if( elease, boost::is_any_of("\"" ) );
    boost::trim_if( lseen, boost::is_any_of("\"" ) );
    boost::trim_if( lastmtime_proxy, boost::is_any_of("\"" ) );
    boost::trim_if( m_delegation_id, boost::is_any_of("\"") );
    boost::trim_if( m_wn_sequence_code, boost::is_any_of("\"") );
    boost::trim_if( m_failure_reason, boost::is_any_of("\"") );
    boost::trim_if( m_worker_node, boost::is_any_of("\"") );

    try {
        m_end_lease = boost::lexical_cast< time_t >( elease );
        m_last_seen = boost::lexical_cast< time_t >( lseen );
	m_proxyCertTimestamp = boost::lexical_cast< time_t >( lastmtime_proxy );
    } catch( boost::bad_lexical_cast& ) {
        throw ClassadSyntax_ex( "CreamJob::unserialize() is unable to cast [" + tstamp + "] or [" +elease+"] or [" +lseen + "] or [" +lastmtime_proxy + "] to time_t" );
    }
    boost::trim_if(m_cream_jobid, boost::is_any_of("\""));

    classad::ClassAdUnParser unparser;
    string jdl_string;
    unparser.Unparse( jdl_string, jdlAd ); // FIXME: Unparsing & Parsing is not good...

    setJdl( jdl_string );
}

//______________________________________________________________________________
void iceUtil::CreamJob::setJdl( const string& j ) throw( ClassadSyntax_ex& )
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
    iceUtil::iceConfManager* conf = iceUtil::iceConfManager::getInstance();

    m_cream_address = conf->getCreamUrlPrefix() 
	+ m_endpoint + conf->getCreamUrlPostfix();
    m_cream_deleg_address = conf->getCreamUrlDelegationPrefix() 
	+ m_endpoint + conf->getCreamUrlDelegationPostfix();
}

//______________________________________________________________________________
bool iceUtil::CreamJob::is_active( void ) const
{
    return ( ( m_status == api::job_statuses::REGISTERED ) ||
             ( m_status == api::job_statuses::PENDING ) ||
             ( m_status == api::job_statuses::IDLE ) ||
             ( m_status == api::job_statuses::RUNNING ) ||
             ( m_status == api::job_statuses::HELD ) ||
	     ( m_status == api::job_statuses::REALLY_RUNNING) );
}

bool iceUtil::CreamJob::can_be_purged( void ) const
{
    return ( ( m_status == api::job_statuses::DONE_OK ) ||
             ( m_status == api::job_statuses::CANCELLED ) ||
             ( m_status == api::job_statuses::DONE_FAILED ) ||
             ( m_status == api::job_statuses::ABORTED ) );
}


bool iceUtil::CreamJob::can_be_resubmitted( void ) const
{ 
    return ( ( m_status == api::job_statuses::DONE_FAILED ) ||
             ( m_status == api::job_statuses::ABORTED ) );
}
