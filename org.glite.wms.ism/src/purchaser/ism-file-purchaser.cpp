// File: ism-file-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/regex.hpp>
#include <fstream>
#include "glite/wms/ism/purchaser/ism-file-purchaser.h"
#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"

using namespace std;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

ism_file_purchaser::ism_file_purchaser(
  std::string const& filename,
  exec_mode_t mode,
  size_t interval,
  exit_predicate_type exit_predicate,
  skip_predicate_type skip_predicate
)
  : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
  m_filename(filename)
{
}

void ism_file_purchaser::operator()()
{
  do_purchase();
}

void ism_file_purchaser::do_purchase()
{
  do {
    
    gluece_info_container_type gluece_info_container;
    
    std::ifstream src(m_filename.c_str());
    if (!src.good()) {
	Warning("Unable to load ISM status from dump file: " << m_filename << "\n");
        return;
    }	
    
    Debug("Loading ISM status from dump file: " << m_filename << "\n");
   
    boost::mutex::scoped_lock l(get_ism_mutex());
    while(!src.eof()) {
      try {
	boost::scoped_ptr<classad::ClassAd> ad(utilities::parse_classad(src));
	string id(utilities::evaluate_attribute(*ad,"id"));
        if (m_skip_predicate.empty() || !m_skip_predicate(id)) {

	  int    ut(utilities::evaluate_attribute(*ad,"update_time"));
	  int    et(utilities::evaluate_attribute(*ad,"expiry_time"));
	  const classad::ClassAd *i=utilities::evaluate_attribute(*ad,"info");

          Debug("Entry info: " << *i << "\n");

boost::shared_ptr<classad::ClassAd> info(static_cast<classad::ClassAd*>(i->Copy()));
info->SetParentScope(0);

          insert_aux_requirements(info);
          expand_glueceid_info(info);
	  boost::tuple<std::string, int, std::string> isinfo;
	  if (split_information_service_url(*info, isinfo)) {
	    get_ism().insert(make_ism_entry(id, ut, info,
              ism_ii_purchaser_entry_update(id,
                boost::tuples::get<0>(isinfo),
	        boost::tuples::get<1>(isinfo),
                boost::tuples::get<2>(isinfo), 30)));
	  }
	  else {
	    get_ism().insert(make_ism_entry(id, ut, info));
	  }
       }
      } 
      catch (utilities::CannotParseClassAd&) {
	Warning("Error parsing info from ISM dump file\n");
      }
      catch (utilities::InvalidValue& e) {
      }	
    }
    if (m_mode == loop) {
      sleep(m_interval);
    }
  } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));
}

// the class factories

extern "C" ism_file_purchaser* create_file_purchaser(
    std::string const& filename,
    exec_mode_t mode,
    size_t interval,
    exit_predicate_type exit_predicate,
    skip_predicate_type skip_predicate
) {
    return new ism_file_purchaser(filename, mode, interval, exit_predicate, skip_predicate);
}

extern "C" void destroy_file_purchaser(ism_file_purchaser* p) {
    delete p;
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
