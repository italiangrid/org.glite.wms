
#include "creamJob.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "classad_distribution.h"
#include "boost/algorithm/string.hpp"


using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace std;

//classad::ClassAdParser cj_parser;
//classad::ClassAdUnParser cj_unp;

//______________________________________________________________________________
CreamJob::CreamJob(const string& _jdl, 
		   const string& _cream_jobid, 
		   const job_status& _status,
		   time_t tstamp, const string& subID) 
  throw(ClassadSyntax_ex&) : cream_jobid( _cream_jobid), 
			     grid_jobid( ), 
			     jdl(_jdl), 
			     status(_status), 
			     lastUpdate(tstamp),
			     subscriptionID(subID)
{
    classad::ClassAdParser parser;
    classad::ClassAd *ad = parser.ParseClassAd( _jdl );;
  
    if (!ad)
        throw ClassadSyntax_ex("The JDL is not valid");
    
    // Look for the "ce_id" attribute
    if ( !ad->EvaluateAttrString( "ce_id", ceid ) ) {
        throw ClassadSyntax_ex("ce_id attribute not found, or is not a string");
    }
    boost::trim_if(ceid, boost::is_any_of("\"") );
    
    // Look for the "X509UserProxy" attribute
    if ( !ad->EvaluateAttrString( "X509UserProxy", user_proxyfile ) ) {
        throw ClassadSyntax_ex("X509UserProxy attribute not found, or is not a string");
    }
    boost::trim_if(user_proxyfile, boost::is_any_of("\""));
    
    // Look for the "edg_jobid" attribute
    if ( !ad->EvaluateAttrString( "edg_jobid", grid_jobid ) ) {
        throw ClassadSyntax_ex( "edg_jobid attribute not found, or is not a string" );
    }
    boost::trim_if(grid_jobid, boost::is_any_of("\"") );
  
    vector<string> pieces;
    try{
        glite::ce::cream_client_api::util::CEUrl::parseCEID(ceid, pieces);
    } catch(glite::ce::cream_client_api::util::CEUrl::ceid_syntax_ex& ex) {
        throw ClassadSyntax_ex(ex.what());
    }
    endpoint = pieces[0] + ":" + pieces[1];
    cream_address = string("https://") + endpoint + "/ce-cream/services/CREAM";
    cream_deleg_address = cream_address + "Delegation";
}
