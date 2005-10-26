#include "iceCommandSubmit.hh"
#include "glite/ce/cream-client-api-c/string_manipulation.h"

using namespace glite::wms::ice;
using namespace std;
using namespace glite::ce::cream_client_api;

iceCommandSubmit::iceCommandSubmit( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) :
    iceAbsCommand( )
{
    classad::ClassAdParser parser;
    classad::ClassAdUnParser unp;
    classad::ClassAd *ad = parser.ParseClassAd( request );
    classad::ExprTree *tree;
    string jobExpr, _command, _jdl;

    if (!ad)
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");

    if ( (tree=ad->Lookup("command") )==NULL ) {
        throw util::JobRequest_ex("attribute 'command' not found");
    }

    unp.Unparse(_command, tree);
    //cout << "jobRequest::unparse - req=["<<_command<<"]"<<endl;
    glite::ce::cream_client_api::util::string_manipulation::trim(_command, "\"");

    if ( 0 != _command.compare( "jobsubmit" ) ) {
        throw util::JobRequest_ex("wrong command ["+_command+"] parsed by iceCommandSubmit" );
    }

    if ( (tree=ad->Lookup("version") )==NULL )
        throw util::JobRequest_ex("attribute 'version' not found");

    if ( (tree=ad->Lookup("arguments") )==NULL )
        throw util::JobRequest_ex("attribute 'arguments' not found");

    unp.Unparse(_jdl, tree);
    ad = parser.ParseClassAd(_jdl);

    if (!ad) {
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing user's JDL");
    }

    if ( ( tree=ad->Lookup("ad") )==NULL ) {
        throw util::JobRequest_ex("attribute 'ad' not found");
    }

    unp.Unparse(jdl, tree);
    
    ad = parser.ParseClassAd(jdl);
    
    if ( (tree=ad->Lookup("X509UserProxy") )==NULL ) {
        throw util::JobRequest_ex("attribute 'X509UserProxy' not found in JDL");
    }

    unp.Unparse(certfile, tree);
    glite::ce::cream_client_api::util::string_manipulation::trim(certfile, "\"");

}

void iceCommandSubmit::execute( soap_proxy::CreamProxy* c )
{

}
