// File: DispatcherFromFileList.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILELIST_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILELIST_H

#include <string>
#include <boost/shared_ptr.hpp>
#include "DispatcherImpl.h"
#include "glite/wms/common/utilities/FLExtractor.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherFromFileList: public DispatcherImpl
{
public:
  typedef glite::wms::common::utilities::FLExtractor<std::string> extractor_type;

private:
  boost::shared_ptr<extractor_type> m_extractor;

public:
  DispatcherFromFileList(boost::shared_ptr<extractor_type> extractor);

  void run(glite::wms::common::task::PipeWriteEnd<RequestPtr>& write_end);
};

}}}} // glite::wms::manager::server

#endif

// Local Variables:
// mode: c++
// End:
