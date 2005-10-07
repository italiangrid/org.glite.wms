/***************************************************************************
 *  filename  : MpiPbsJobWrapper.cpp
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *              Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <algorithm>
#include <cassert>

#include "MpiPbsJobWrapper.h"

using namespace std;

namespace url = glite::wms::helper::jobadapter::url;
namespace configuration = glite::wms::common::configuration;


namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {

MpiPbsJobWrapper::MpiPbsJobWrapper(const string& job)
  : JobWrapper(job)
{ 
}

MpiPbsJobWrapper::~MpiPbsJobWrapper(void)
{
}

ostream&
MpiPbsJobWrapper::print(std::ostream& os) const
{
  const configuration::WMConfiguration* wm_config
    = configuration::Configuration::instance()->wm();

  if (!fill_out_script( 
                       wm_config->job_wrapper_template_dir() 
                       + 
                       "/template.mpi.pbs.sh" ,os) ) {
    os << "echo \"Generic error occurred while writing template\"\n";
  }

  return os;
}



} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite
