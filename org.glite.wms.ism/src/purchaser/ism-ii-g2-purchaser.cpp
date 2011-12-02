// File: ism-ii-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>

// Copyright (c) Members of the EGEE Collaboration. 2009. 
// See http://www.eu-egee.org/partners/ for details on the copyright holders.  

// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//     http://www.apache.org/licenses/LICENSE-2.0 
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.

// $Id$

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <classad_distribution.h>

#include "ldap-utils-g2.h"
#include "glite/wms/ism/ism.h"
#include "glite/wms/ism/purchaser/ism-ii-g2-purchaser.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

using namespace std;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

namespace {

boost::condition f_purchasing_cycle_run_condition;
boost::mutex     f_purchasing_cycle_run_mutex;

} // {anonymous}

bool 
ism_ii_g2_purchaser_entry_update::operator()(
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


ism_ii_g2_purchaser::ism_ii_g2_purchaser(
  std::string const& hostname,
  int port,
  std::string const& distinguished_name,
  int timeout,
  std::string const& ldap_ce_filter_ext,
  bool ldap_search_async,
  exec_mode_t mode,
  size_t interval,
  exit_predicate_type exit_predicate,
  skip_predicate_type skip_predicate
) : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
  m_hostname(hostname),
  m_port(port),
  m_dn(distinguished_name),
  m_timeout(timeout),
  m_ldap_ce_filter_ext(ldap_ce_filter_ext),
  m_ldap_search_async(ldap_search_async)
{
}

namespace {

void apply_skip_predicate(
  gluece_info_container_type& gluece_info_container,
  vector<gluece_info_iterator>& gluece_info_container_updated_entries,
  skip_predicate_type skip
)
{
  gluece_info_iterator it = gluece_info_container.begin();
  gluece_info_iterator const gluece_info_container_end(
    gluece_info_container.end()
  );

  for ( ; it != gluece_info_container_end; ++it) {
    if (!skip(it->first)) {
      it->second->InsertAttr("PurchasedBy","ism_ii_g2_purchaser");
      gluece_info_container_updated_entries.push_back(it);
    }
    else {
      Debug("Skipping " << it->first << " due to skip predicate settings");
    }
  }
}

void populate_ism(
  vector<gluece_info_iterator>& gluece_info_container_updated_entries,
  size_t the_ism_index)
{      
  static glite::wms::common::configuration::Configuration const& config(
    *glite::wms::common::configuration::Configuration::instance()
  ); 
  static const time_t expiry_time( 
    config.wm()->ism_ii_purchasing_rate() + config.ns()->ii_timeout()
  );

  vector<gluece_info_iterator>::const_iterator it(
    gluece_info_container_updated_entries.begin()
  );
  vector<gluece_info_iterator>::const_iterator const e(
    gluece_info_container_updated_entries.end()
  );

  // no locking is needed here (before the switch)
  int dark_side = ism::dark_side();

  bool insert = false;
  ism_type::iterator ism_entry;

  for ( ; it != e; ++it ) {
    boost::tie(ism_entry, insert) = 
      get_ism(the_ism_index, dark_side).insert( 
        make_ism_entry(
          (*it)->first,
          std::time(0),
          (*it)->second,
          ism_ii_g2_purchaser_entry_update(),
          expiry_time
        )
      );
    if(!insert) { // glue13 entry already inserted merging info
      ism_entry->second.get<2>()->Update(*(*it)->second);
    }
    Debug((*it)->first << " added to ISM ");
  } 
}

}

void ism_ii_g2_purchaser::operator()()
{
  do {

    // do not populate until the existing ism has threads still matching against it
    while (ism::matching_threads(ism::dark_side()) > 0) {

     Debug(
       "waiting for " <<
       ism::matching_threads(ism::active_side()) <<
       " threads to match before switching the ISM side "
     );

     ::sleep(1); // TODO
    }

    // free this memory _before_ another huge allocation made by the purchaser
    ism::get_ism(ism::ce, ism::dark_side()).clear();
    ism::get_ism(ism::se, ism::dark_side()).clear();

    try {

     gluece_info_container_type gluece_info_container;
     vector<gluece_info_iterator> gluece_info_container_updated_entries;

     gluese_info_container_type gluese_info_container;
     vector<gluese_info_iterator> gluese_info_container_updated_entries;

     time_t const t0 = std::time(0);
       fetch_bdii_info_g2(
         m_hostname,
         m_port,
         m_dn,
         m_timeout,
         m_ldap_ce_filter_ext,
         gluece_info_container,
         gluese_info_container
       );
     Debug("BDII fetching completed in " << std::time(0) - t0 << " seconds");

     apply_skip_predicate(
       gluece_info_container,
       gluece_info_container_updated_entries,
       m_skip_predicate
     );

     apply_skip_predicate(
       gluese_info_container,
       gluese_info_container_updated_entries,
       m_skip_predicate
     );

     // incoming requests asking for MM will be assigned the current active
     // side so we can operate without locking here, now that older threads
     // against the current dark side have all flushed
     populate_ism(gluece_info_container_updated_entries, ism::ce);
     populate_ism(gluese_info_container_updated_entries, ism::se);

     ism::switch_active_side();
    } catch (LDAPException& e) {

      Error(
        "Failed to purchase info from "
        << m_hostname << ":" << m_port << " (" << e.what() << ")"
      );
    } catch (...) {

      // TODO: Check which exception may arrive here... and remove catch all
      Warning("Failed to purchase info from " << m_hostname << ":" << m_port);
    }

    if (m_mode) {
      boost::xtime xt;
      boost::xtime_get(&xt, boost::TIME_UTC);
      xt.sec += m_interval;
      boost::mutex::scoped_lock l(f_purchasing_cycle_run_mutex);
      f_purchasing_cycle_run_condition.timed_wait(l, xt);
    }
  } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));
}

// the class factories
extern "C" ism_ii_g2_purchaser* create_ii_g2_purchaser(std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout,
    std::string const& ldap_ce_filter_ext,
    bool ldap_search_async,
    exec_mode_t mode,
    size_t interval,
    exit_predicate_type exit_predicate,
    skip_predicate_type skip_predicate) 
{
    return new ism_ii_g2_purchaser(
      hostname, port, distinguished_name, timeout, 
      ldap_ce_filter_ext, ldap_search_async,
      mode, interval, exit_predicate, skip_predicate
    );
}

extern "C" 
void destroy_ii_g2_purchaser(ism_ii_g2_purchaser* p) {
    delete p;
}

// the entry update function factory
extern "C" 
boost::function<bool(int&, ad_ptr)> create_ii_g2_entry_update_fn() 
{
  return ism_ii_g2_purchaser_entry_update();
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
