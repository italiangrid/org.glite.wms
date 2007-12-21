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
 * ICE command factory
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceCommandFactory.h"
#include "iceAbsCommand.h"
#include "iceCommandSubmit.h"
#include "iceCommandCancel.h"
#include "classad_distribution.h"
#include "boost/algorithm/string.hpp"
#include "boost/scoped_ptr.hpp"
//x#include "CreamProxyFactory.h"
#include "ice-core.h"
#include "Request.h"

using namespace std;

namespace glite {
namespace wms {
namespace ice {

iceAbsCommand* iceCommandFactory::mkCommand( util::Request* request ) 
  throw(util::ClassadSyntax_ex&, util::JobRequest_ex&) 
{
    iceAbsCommand* result = 0;
    classad::ClassAdParser parser;
    classad::ClassAd *rootAd = parser.ParseClassAd( request->to_string() );

    if ( !rootAd ) {
        throw util::ClassadSyntax_ex("ClassAd parser returned a NULL pointer parsing entire request");
    }

    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( rootAd );

    string commandStr;
    if ( !classad_safe_ptr->EvaluateAttrString( "command", commandStr ) ) {
        throw util::JobRequest_ex("attribute 'command' not found or is not a string");
    }
    boost::trim_if(commandStr, boost::is_any_of("\""));

    if ( boost::algorithm::iequals( commandStr, "submit" ) ) {
      //result = new iceCommandSubmit( util::CreamProxyFactory::makeCreamProxy(false), request );
      result = new iceCommandSubmit( request );
    } else if ( boost::algorithm::iequals( commandStr, "cancel" ) ) {
      //        result = new iceCommandCancel( util::CreamProxyFactory::makeCreamProxy(false), request );
      result = new iceCommandCancel( request );
	
    } else {
        throw util::JobRequest_ex( "Unknown command " + commandStr + " in request classad" );
    }
    return result;
}

} // namespace ice
} // namespace wms
} // namespace glite
