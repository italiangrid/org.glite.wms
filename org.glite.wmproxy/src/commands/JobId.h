/*
 * File: JobId.h
 *
 */

// $Id$

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_JOBID_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_JOBID_H_

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {    
namespace jobid {

  inline std::string to_filename(glite::wmsutils::jobid::JobId j, 
				 int level = 0,
				 bool extended_path = true) {
    std::string path( get_reduced_part(j, level) );
    if ( extended_path ) path.append(std::string("/") + glite::wmsutils::jobid::to_filename(j));
    return path;
  }
		     
}
}
}
}
}

#endif
