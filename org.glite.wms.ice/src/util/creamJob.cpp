#include "creamJob.h"
#include "iceConfManager.h"

#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "classad_distribution.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace std;
namespace iceUtil = glite::wms::ice::util;
namespace fs = boost::filesystem;

//______________________________________________________________________________
CreamJob::CreamJob( ) :
    status( UNKNOWN ),
    last_status_change( time(NULL) ),
    last_seen( last_status_change ),
    end_lease( last_status_change + 60*30 ) // FIXME: remove hardcoded default
{

}

//______________________________________________________________________________
CreamJob::CreamJob( const std::string& ad ) throw ( ClassadSyntax_ex& ) 
{
    unserialize( ad );
}

//______________________________________________________________________________
string CreamJob::serialize( void ) const
{
    string res;

    classad::ClassAd ad;
    ad.InsertAttr( "cream_jobid", cream_jobid );
    ad.InsertAttr( "status", status );
    classad::ClassAdParser parser;
    classad::ClassAd* jdlAd = parser.ParseClassAd( jdl );
    // Updates sequence code
    jdlAd->InsertAttr( "LB_sequence_code", sequence_code );
    ad.Insert( "jdl", jdlAd );

    try {    
        ad.InsertAttr( "last_status_change", boost::lexical_cast< string >(last_status_change) ); 
        ad.InsertAttr( "last_seen", boost::lexical_cast< string >(last_seen) );
        ad.InsertAttr( "end_lease" , boost::lexical_cast< string >(end_lease) );
    } catch( boost::bad_lexical_cast& ) {
        // Should never happen...
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
    classad::ClassAd *jdlAd;
    int st_number;
    string tstamp; // last status change
    string elease; // end lease
    string lseen; // last seen

    ad = parser.ParseClassAd( buf );
  
    if(!ad)
        throw ClassadSyntax_ex(string("ClassAd parser returned a NULL pointer parsing entire classad ")+buf);
  
    if ( ! ad->EvaluateAttrString( "cream_jobid", cream_jobid ) ||
         ! ad->EvaluateAttrNumber( "status", st_number ) ||
         ! ad->EvaluateAttrClassAd( "jdl", jdlAd ) ||
         ! ad->EvaluateAttrString( "last_status_change", tstamp ) ||
         ! ad->EvaluateAttrString( "last_seen", lseen ) ||
         ! ad->EvaluateAttrString( "end_lease", elease ) ) {
        throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for 'grid_jobid' or 'status' or 'jdl' or 'last_update' or 'end_lease' attributes");
    }
    status = (glite::ce::cream_client_api::job_statuses::job_status)st_number;
    boost::trim_if( tstamp, boost::is_any_of("\"" ) );
    boost::trim_if( elease, boost::is_any_of("\"" ) );
    boost::trim_if( lseen, boost::is_any_of("\"" ) );
    
    try {
        last_status_change = boost::lexical_cast< time_t >( tstamp );
        end_lease = boost::lexical_cast< time_t >( elease );
        last_seen = boost::lexical_cast< time_t >( lseen );
    } catch( boost::bad_lexical_cast& ) {
        throw ClassadSyntax_ex( "CreamJob::unserialize() is unable to cast [" + tstamp + "] or [" +elease+"] or [" +lseen + "] to time_t" );
    }
    boost::trim_if(cream_jobid, boost::is_any_of("\""));

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

    jdl = j;

    // Look for the "ce_id" attribute
    if ( !jdlAd->EvaluateAttrString( "ce_id", ceid ) ) {
        throw ClassadSyntax_ex("CreamJob::setJdl: ce_id attribute not found, or is not a string");
    }
    boost::trim_if(ceid, boost::is_any_of("\"") );
    
    // Look for the "X509UserProxy" attribute
    if ( !jdlAd->EvaluateAttrString( "X509UserProxy", user_proxyfile ) ) {
        throw ClassadSyntax_ex("CreamJob::setJdl: X509UserProxy attribute not found, or is not a string");
    }
    boost::trim_if(user_proxyfile, boost::is_any_of("\""));

    // Look for the "LBSequenceCode" attribute (if this attribute is not in the classad, the sequence code is set to the empty string
    if ( jdlAd->EvaluateAttrString( "LB_sequence_code", sequence_code ) ) {
        boost::trim_if(sequence_code, boost::is_any_of("\""));
    }
    
    // Look for the "edg_jobid" attribute
    if ( !jdlAd->EvaluateAttrString( "edg_jobid", grid_jobid ) ) {
        throw ClassadSyntax_ex( "CreamJob::setJdl: edg_jobid attribute not found, or is not a string" );
    }
    boost::trim_if(grid_jobid, boost::is_any_of("\"") );
  
    vector<string> pieces;
    try{
        glite::ce::cream_client_api::util::CEUrl::parseCEID(ceid, pieces);
    } catch(glite::ce::cream_client_api::util::CEUrl::ceid_syntax_ex& ex) {
        throw ClassadSyntax_ex(ex.what());
    }
    endpoint = pieces[0] + ":" + pieces[1];

    /**
     * No need to lock the mutex because getInstance already does that
     */
    iceUtil::iceConfManager* conf = iceUtil::iceConfManager::getInstance();

    {
      boost::recursive_mutex::scoped_lock M( iceUtil::iceConfManager::mutex );
      cream_address = conf->getCreamUrlPrefix() + endpoint + conf->getCreamUrlPostfix();
      cream_deleg_address = conf->getCreamUrlDelegationPrefix() + endpoint + conf->getCreamUrlDelegationPostfix();
    }

}

//______________________________________________________________________________
bool CreamJob::is_active( void ) const
{
    return ( ( status == REGISTERED ) ||
             ( status == PENDING ) ||
             ( status == IDLE ) ||
             ( status == RUNNING ) ||
             ( status == HELD ) );
}
