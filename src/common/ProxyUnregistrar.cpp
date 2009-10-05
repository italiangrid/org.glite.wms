#include <string>

#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/security/proxyrenewal/renewal.h"
#include "jobcontrol_namespace.h"

#include "ProxyUnregistrar.h"

USING_COMMON_NAMESPACE;
using namespace std;

namespace ts = glite::wms::common::logger::threadsafe;

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {

ProxyUnregistrar::ProxyUnregistrar( const string &id ) : pu_id( id ) {}

ProxyUnregistrar::~ProxyUnregistrar( void ) {}

void ProxyUnregistrar::unregister( void )
{
  int      err = 0;
  logger::StatePusher      pusher( ts::edglog, "ProxyUnregistrar::unregister()" );

  ts::edglog << logger::setlevel( logger::verylow )
	     << "Unregistering user proxy..." << endl;

  err = glite_renewal_UnregisterProxy( this->pu_id.c_str(), NULL );

  if( err && (err != EDG_WLPR_PROXY_NOT_REGISTERED) )
    ts::edglog << logger::setlevel( logger::null ) << "I cannot unregister the job proxy." << endl
	       << "Reason: \"" << edg_wlpr_GetErrorText(err) << "\"." << endl;
  else if( err == EDG_WLPR_PROXY_NOT_REGISTERED )
    ts::edglog << logger::setlevel( logger::null ) << "Job proxy not registered. Going ahead." << endl;

  return;
}

} // Namespace jccommon

} JOBCONTROL_NAMESPACE_END
