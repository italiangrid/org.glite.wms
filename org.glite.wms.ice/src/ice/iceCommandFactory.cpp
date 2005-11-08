#include "iceCommandFactory.h"
#include "iceAbsCommand.h"
#include "iceCommandSubmit.h"
#include "iceCommandCancel.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
#include "classad_distribution.h"

using namespace glite::wms::ice;
using namespace std;

iceAbsCommand* iceCommandFactory::mkCommand( const std::string& request ) throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) 
{
    iceAbsCommand* result = 0;
    classad::ClassAdParser parser;
    classad::ClassAd *_rootAd = parser.ParseClassAd( request );

    if ( !_rootAd ) {
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");
    }

    string _commandStr;
    if ( !_rootAd->EvaluateAttrString( "command", _commandStr ) ) {
        throw util::JobRequest_ex("attribute 'command' not found or is not a string");
    }
    glite::ce::cream_client_api::util::string_manipulation::trim(_commandStr, "\"");

    if ( 0 == _commandStr.compare( "jobsubmit" ) ) {
        result = new iceCommandSubmit( request );
    } else if ( 0 == _commandStr.compare( "jobcancel" ) ) {
        result = new iceCommandCancel( request );
    } else {
        throw util::JobRequest_ex( "Unknown command " + _commandStr + " in request classad" );
    }
    return result;
}
