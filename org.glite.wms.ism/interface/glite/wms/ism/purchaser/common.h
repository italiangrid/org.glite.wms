// File: common.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef _GLITE_WMS_ISM_PURCHASER_COMMON_
#define _GLITE_WMS_ISM_PURCHASER_COMMON_

#include <vector>
#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/ism/ism.h"

namespace exception = glite::wmsutils::exception;

namespace glite {
namespace wms {

namespace utilities     = common::utilities;
namespace logger        = common::logger;

namespace ism {
namespace purchaser {

  typedef boost::shared_ptr<classad::ClassAd>      gluece_info_type;
  typedef std::map<std::string, gluece_info_type>  gluece_info_container_type;
  typedef gluece_info_container_type::const_iterator gluece_info_const_iterator;
  typedef gluece_info_container_type::iterator       gluece_info_iterator;
  
 bool expand_information_service_info(gluece_info_type& gluece_info);
 void insert_aux_requirements(gluece_info_type& gluece_info);
 bool expand_glueceid_info(gluece_info_type& gluece_info);
 timestamp_type get_current_time(void);

 enum exec_mode_t {
   once,
   loop
 };

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
