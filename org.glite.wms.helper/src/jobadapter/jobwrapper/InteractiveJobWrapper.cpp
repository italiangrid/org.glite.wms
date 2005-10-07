/***************************************************************************
 *  filename  : InteractiveJobWrapper.cpp
 *  authors   : Elisabetta Ronchieri elisabetta.ronchieri@cnaf.infn.it
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <algorithm>
#include <cassert>

#include "InteractiveJobWrapper.h"

using namespace std;

namespace url = glite::wms::helper::jobadapter::url;
namespace configuration = glite::wms::common::configuration;

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
InteractiveJobWrapper::print(std::ostream& os) const
{
  const configuration::WMConfiguration* wm_config
    = configuration::Configuration::instance()->wm();

  if (!fill_out_script( 
                       wm_config->job_wrapper_template_dir() 
                       + 
                       "/template.interactive.sh" ,os) ) {
    os << "echo \"Generic error occurred while writing template\"\n";
  }

  return os;
}

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace gltie
