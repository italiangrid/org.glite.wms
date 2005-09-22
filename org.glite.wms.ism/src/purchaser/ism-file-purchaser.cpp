// File: ism-file-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/regex.hpp>
#include <fstream>
#include "glite/wms/ism/purchaser/ism-file-purchaser.h"

using namespace std;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

namespace {
ii::create_entry_update_fn_t      *f_ii_purchaser_entry_update_fn;
ii_gris::create_entry_update_fn_t *f_ii_gris_purchaser_entry_update_fn;
cemon::create_entry_update_fn_t   *f_cemon_purchaser_entry_update_fn;
rgma::create_entry_update_fn_t   *f_rgma_purchaser_entry_update_fn;
}

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

          Debug("Loading ISM entry info: " << id << "\n");

	  boost::shared_ptr<classad::ClassAd> info(static_cast<classad::ClassAd*>(i->Copy()));
          info->SetParentScope(0);

          insert_aux_requirements(info);
          expand_glueceid_info(info);

          // Check the type of puchaser which has generated the info
	  string purchased_by;
          info->EvaluateAttrString("PurchasedBy",purchased_by);
          // For compatibility reason with previous version info related to ii purchaser
	  // will be identified by evaluating the information_service_url...
	  boost::tuple<std::string, int, std::string> isinfo;
	  if (split_information_service_url(*info, isinfo)) {
            // At this point we are sure that the info has been purchased either from
            // the ism-ii-purchaser or ism-ii-gris-purchaser. For the former we should use
            // the just extract info from GlueInformationServiceURL, whereas for the latter
            if (purchased_by==string("ism_ii_gris_purchaser")) {    
	        get_ism().insert(make_ism_entry(id, ut, info,
                f_ii_gris_purchaser_entry_update_fn(id,
                  boost::tuples::get<0>(isinfo),
	          boost::tuples::get<1>(isinfo),
                  boost::tuples::get<2>(isinfo), 30)));
            } 
            else {
              get_ism().insert(make_ism_entry(id, ut, info,
                f_ii_purchaser_entry_update_fn()));
            }
	  }
	  else if (purchased_by==string("ism_cemon_purchaser")) {
            get_ism().insert(make_ism_entry(id, ut, info,
              f_cemon_purchaser_entry_update_fn()));
          }
          else if (purchased_by==string("ism_rgma_purchaser")) {
            get_ism().insert(make_ism_entry(id, ut, info,
              f_rgma_purchaser_entry_update_fn()));
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

extern "C" void set_purchaser_entry_update_fns(
  ii::create_entry_update_fn_t* ii, 
  ii_gris::create_entry_update_fn_t* ii_gris, 
  cemon::create_entry_update_fn_t* cemon,
  rgma::create_entry_update_fn_t* rgma
)
{
  f_ii_purchaser_entry_update_fn = ii;
  f_ii_gris_purchaser_entry_update_fn = ii_gris;
  f_cemon_purchaser_entry_update_fn = cemon;
  f_rgma_purchaser_entry_update_fn = rgma;
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
