// File: DispatcherFromFileList.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILELIST_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILELIST_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif
#ifndef BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H
#include "DispatcherImpl.h"
#endif
#ifndef GLITE_WMS_COMMON_UTILITIES_FLEXTRACTOR_H
#include "glite/wms/common/utilities/FLExtractor.h"
#endif

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace task = glite::wms::common::task;
namespace utilities = glite::wms::common::utilities;

class DispatcherFromFileList: public DispatcherImpl
{
public:
  typedef utilities::FLExtractor<std::string> extractor_type;

private:
  boost::shared_ptr<extractor_type> m_extractor;

public:
  DispatcherFromFileList(boost::shared_ptr<extractor_type> extractor);

  void run(task::PipeWriteEnd<pipe_value_type>& write_end);
};

} // server
} // manager
} // wms
} // glite 

#endif

// Local Variables:
// mode: c++
// End:
