/***************************************************************************
 *  filename  : InteractiveJobWrapper.cpp
 *  authors   : Elisabetta Ronchieri elisabetta.ronchieri@cnaf.infn.it
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <algorithm>
#include <cassert>

#include "JobWrapper.h"
#include "InteractiveJobWrapper.h"

using namespace std;

namespace url = glite::wms::helper::jobadapter::url;

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {

InteractiveJobWrapper::InteractiveJobWrapper(const string& job)
  : JobWrapper(job)
{  
}

InteractiveJobWrapper::~InteractiveJobWrapper(void)
{
}

ostream& 
InteractiveJobWrapper::make_bypass_transfer(ostream& os,
					    const string& prefix) const
{
  vector<string> infiles;
  infiles.push_back("glite-wms-pipe-input");
  infiles.push_back("glite-wms-pipe-output");
  infiles.push_back("glite-wms-job-agent");
  
  os << "for f in ";
  for (vector<string>:: const_iterator it = infiles.begin();
       it != infiles.end(); ++it) {
    os << " \"" << *it << "\"";
  }
  os << " ; do" << endl;
      
  os << "   globus-url-copy " << prefix << "opt/glite/bin/${f} file://${workdir}/${f}" << endl
     << "   chmod +x ${workdir}/${f}" << endl
     << "done" << endl	  
     << endl;

  string libfile("/opt/glite/lib/libglite-wms-grid-console-agent.so.0");
  
  os << "globus-url-copy " << prefix << libfile << " file://${workdir}/libglite-wms-grid-console-agent.so.0" << endl
     << endl;
  
  return os << endl;  
}

ostream&
InteractiveJobWrapper::execute_job(ostream& os,
		                   const string& arguments,
		                   const string& job,
		                   const string& stdi,
		                   const string& stdo,
		                   const string& stde,
		                   int           node) const
{
  os << "./glite-wms-job-agent ${BYPASS_SHADOW_HOST} ${BYPASS_SHADOW_PORT}"; 
	  
  if (arguments != "") {
    os << " \"" << job << " " << arguments << " $*\"";
  }
  else {
    os << " \"" << job << " $*\"";
  }

  return os << endl
            << endl;
}

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace gltie
