// File: ism-ii-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/progress.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <classad_distribution.h>
#include "ldap-utils.h"
#include "glite/wms/ism/ism.h"
#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"
#include "glite/wms/common/logger/logger_utils.h"

using namespace std;

namespace glite {
namespace wms {

namespace ldif2classad	= common::ldif2classad;

namespace ism {
namespace purchaser {

namespace {

boost::condition f_purchasing_cycle_run_condition;
boost::mutex     f_purchasing_cycle_run_mutex;

bool 
ism_ii_purchaser_entry_update::operator()(
  int a,
  boost::shared_ptr<classad::ClassAd>& ad
)
{
  boost::mutex::scoped_lock l(f_purchasing_cycle_run_mutex);
  f_purchasing_cycle_run_condition.notify_one();
  // At this point we don't know if the update
  // will be successfully performed or not since
  // the task of the update is to notify the purchaser thread
  // waiting on the "run another loop" condition... 
  // If we return false the entry is removed from the ISM
  // and then, eventually, readded on successull completion
  // of the actual update.
  return false;
}

} // {anonymous}

ism_ii_purchaser::ism_ii_purchaser(
  std::string const& hostname,
  int port,
  std::string const& distinguished_name,
  int timeout,
  exec_mode_t mode,
  size_t interval,
  exit_predicate_type exit_predicate,
  skip_predicate_type skip_predicate
) : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
  m_hostname(hostname),
  m_port(port),
  m_dn(distinguished_name),
  m_timeout(timeout)
{
}

void ism_ii_purchaser::operator()()
{
  do_purchase();
}

void ism_ii_purchaser::do_purchase()
{
  do {
    try {
      gluece_info_container_type gluece_info_container;
      vector<gluece_info_iterator> gluece_info_container_updated_entries;

      boost::timer t0;
      fetch_bdii_info(
        m_hostname,
        m_port,
        m_dn,
        m_timeout,
        gluece_info_container
      );
      Info("BDII fetching completed in " << t0.elapsed() << " seconds");
      
      gluece_info_iterator it = gluece_info_container.begin();
      gluece_info_iterator const gluece_info_container_end(
        gluece_info_container.end()
      );

     for ( ;
           it != gluece_info_container_end; ++it) {

       // Skips CE info according to the specified predicate...
       if (m_skip_predicate.empty() || !m_skip_predicate(it->first)) {
           
         if (expand_glueceid_info(it->second) &&
             insert_aux_requirements(it->second) && 
             insert_gangmatch_storage_ad(it->second)) {

           it->second->InsertAttr("PurchasedBy","ism_ii_purchaser");
           gluece_info_container_updated_entries.push_back(it);
         }
       }
     }
     {
       ism_mutex_type::scoped_lock l(get_ism_mutex());	

       vector<gluece_info_iterator>::iterator it(
         gluece_info_container_updated_entries.begin()
       );
       vector<gluece_info_iterator>::iterator const e(
         gluece_info_container_updated_entries.end()
       );
       time_t const current_time = std::time(0);
       for ( ; it != e; ++it ) {
	  ism_type::iterator ism_it = get_ism().find((*it)->first);
          if (ism_it != get_ism().end()) {
            ism_type::data_type& data = ism_it->second;
            boost::tuples::get<0>(data) = current_time;
            boost::tuples::get<2>(data) = (*it)->second;
          } else {
            get_ism().insert( 
              make_ism_entry(
                (*it)->first,
                current_time,
                (*it)->second,
                ism_ii_purchaser_entry_update()
              )
            );
          }
        } 
      } // unlock the mutex
      if (m_mode) {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += m_interval;
        boost::mutex::scoped_lock l(f_purchasing_cycle_run_mutex);
        f_purchasing_cycle_run_condition.timed_wait(l, xt);
      }
    }
    catch (...) { // TODO: Check which exception may arrive here... and remove catch all
      Warning("Failed to purchase info from " << m_hostname << ":" << m_port);
    }
  } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));
}

// the class factories
extern "C" ism_ii_purchaser* create_ii_purchaser(std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout,
    exec_mode_t mode,
    size_t interval,
    exit_predicate_type exit_predicate,
    skip_predicate_type skip_predicate) 
{
    return new ism_ii_purchaser(
      hostname, port, distinguished_name, timeout, 
      mode, interval, exit_predicate, skip_predicate
    );
}

extern "C" 
void destroy_ii_purchaser(ism_ii_purchaser* p) {
    delete p;
}

// the entry update function factory
extern "C" 
boost::function<bool(int&, ad_ptr)> create_ii_entry_update_fn() 
{
  return ism_ii_purchaser_entry_update();
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
