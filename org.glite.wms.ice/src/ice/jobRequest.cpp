
#include "jobRequest.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"

using namespace glite::wms::ice;
//using namespace glite::ce::cream_client_api::util;
using namespace std;

jobRequest::jobRequest(const string& request) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&)
{
  this->unparse(request);
}

void jobRequest::unparse(const string& request) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&)
{
  classad::ClassAd *ad = NULL;
  classad::ExprTree *tree = NULL;
  string jobExpr(""), _command(""), _jdl("");
  

  grid_jobid = "";
  jdl = "";
  certfile = "";

  ad = parser.ParseClassAd(request);
  
  if(!ad)
    throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

  if((tree=ad->Lookup("command"))==NULL)
    throw util::JobRequest_ex("attribute 'command' not found");
  else {
    unp.Unparse(_command, tree);
    //cout << "jobRequest::unparse - req=["<<_command<<"]"<<endl;
    glite::ce::cream_client_api::util::string_manipulation::trim(_command, "\"");
    command = unknown;
    if(_command == "jobsubmit")
      command = jobsubmit;
    if(_command == "jobcancel")
      command = jobcancel;
  }
    
  if((tree=ad->Lookup("version"))==NULL)
    throw util::JobRequest_ex("attribute 'version' not found");

  if((tree=ad->Lookup("arguments"))==NULL)
    throw util::JobRequest_ex("attribute 'arguments' not found");

  
  unp.Unparse(_jdl, tree);

  //cerr << "_jdl = <"<<_jdl<<">"<<endl;

  ad = parser.ParseClassAd(_jdl);
  if(!ad)
    throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing request's 'ad' tree");
  
  if((tree=ad->Lookup("ad"))==NULL)
    throw util::JobRequest_ex("attribute 'ad' not found");
  else {
    unp.Unparse(jdl, tree);
  }
  
  ad = parser.ParseClassAd(jdl);
  if(!ad)
    throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing JDL");

  if((tree=ad->Lookup("id"))==NULL)
    throw util::JobRequest_ex("attribute 'id' not found");
  else {
    unp.Unparse(grid_jobid, tree);
  }
  
  ad = parser.ParseClassAd(jdl);
  
  if((tree=ad->Lookup("X509UserProxy"))==NULL)
    throw util::JobRequest_ex("attribute 'X509UserProxy' not found in JDL");
  else {
    unp.Unparse(certfile, tree);
    glite::ce::cream_client_api::util::string_manipulation::trim(certfile, "\"");
  }
}
