
#include "creamJob.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "classad_distribution.h"

using namespace glite::wms::ice::util;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::job_statuses;
using namespace std;

classad::ClassAdParser cj_parser;
classad::ClassAdUnParser cj_unp;

//______________________________________________________________________________
CreamJob::CreamJob(const string& _jdl, 
		   const string& _jobid, 
		   const string& gid, 
		   const job_status& _status) 
  throw(ClassadSyntax_ex&) : jobid(_jobid), edg_jobid(gid), 
			     jdl(_jdl), status(_status)
{
  classad::ClassAd *ad;
  classad::ExprTree *jdltree;
  
  ad = cj_parser.ParseClassAd(_jdl);
  
  if(!ad)
    throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire JDL");
  
  if((jdltree=ad->Lookup("ce_id"))!=NULL)
    {
      cj_unp.Unparse(ceid, jdltree);
      //classad::ClassAdUnParser::Unparse(ceid, jdltree);
    } else {
    throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for 'ce_id' attributes");
  }

  if((jdltree=ad->Lookup("X509UserProxy"))!=NULL)
    {
      cj_unp.Unparse(user_proxyfile, jdltree);
      //classad::ClassAdUnParser::Unparse(user_proxyfile, jdltree);
    } else {
    throw ClassadSyntax_ex("ClassAd parser returned a NULL pointer looking for 'X509UserProxy' attributes");
  }

  glite::ce::cream_client_api::util::string_manipulation::trim(ceid, "\"");
  glite::ce::cream_client_api::util::string_manipulation::trim(user_proxyfile, "\"");

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
