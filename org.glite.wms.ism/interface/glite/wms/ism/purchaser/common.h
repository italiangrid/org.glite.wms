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

 typedef boost::shared_ptr<classad::ClassAd>        gluece_info_type;
 typedef std::map<std::string, gluece_info_type>    gluece_info_container_type;
 typedef gluece_info_container_type::const_iterator gluece_info_const_iterator;
 typedef gluece_info_container_type::iterator       gluece_info_iterator;
 
 typedef boost::shared_ptr<classad::ClassAd>        gluese_info_type;
 typedef std::map<std::string, gluese_info_type>    gluese_info_container_type;
 typedef gluese_info_container_type::const_iterator gluese_info_const_iterator;
 typedef gluese_info_container_type::iterator       gluese_info_iterator;

 bool expand_information_service_info(gluece_info_type& gluece_info);
 bool insert_aux_requirements(gluece_info_type& gluece_info);
 bool insert_gangmatch_storage_ad(gluece_info_type& gluece_info);
 bool expand_glueceid_info(gluece_info_type& gluece_info);
 bool split_information_service_url(classad::ClassAd const&, boost::tuple<std::string, int, std::string>&);
 boost::xtime get_current_time(void);
	
 enum exec_mode_t {
   once,
   loop
 };

 class regex_matches_string {
  std::string m_string;
  public:
  regex_matches_string::regex_matches_string(std::string const& s) : m_string(s) {}
  bool operator()(std::string const& regex) {
    try {
      boost::regex r(regex);
      boost::smatch s;
      return boost::regex_match(m_string, s, r);
    } 
    catch( boost::bad_expression& e ) {
      return false;
    }
  }
 };

 class is_in_black_list {
 std::vector<std::string> m_black_list;
 public:
   is_in_black_list::is_in_black_list(std::vector<std::string> const& black_list) : m_black_list(black_list) {}
   bool operator()(std::string const& entry_id) {
    return find_if(m_black_list.begin(), m_black_list.end(), regex_matches_string(entry_id)) != m_black_list.end();
   }
 };

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
