// $Id$

#include <iostream>
#include <fstream>
#include <classad_distribution.h>

#include "glite/wms/ism/ism.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

namespace configuration = glite::wms::common::configuration;

namespace glite {
namespace wms {
namespace ism {

ism_type::value_type make_ism_entry(
  std::string const& id,    // resource identifier
  int const& ut,            // update time
  ad_ptr const& ad,         // resource descritpion
  update_uf_type const& uf, // update function
  int const& et             // expiry time with defualt 5*60
)
{
  return std::make_pair(id, boost::make_tuple(ut, et, ad, uf));
}

boost::recursive_mutex& get_ism_mutex(void)
{
  static boost::recursive_mutex ism_mutex;

  return ism_mutex;
}

ism_type& get_ism(void)
{
  static ism_type ism;

  return ism;
}

std::ostream&
operator<<(std::ostream& os, ism_type::value_type const& value)
{
  return os << '[' << value.first << "]\n"
            << boost::tuples::get<0>(value.second) << '\n'
	    << boost::tuples::get<1>(value.second) << '\n'
	    << *boost::tuples::get<2>(value.second) << '\n'
	    << "[END]";
}

void call_update_ism_entries::operator()()
{
  bool value = true;
  boost::recursive_mutex::scoped_lock l(get_ism_mutex());
  boost::xtime ct;
  boost::xtime_get(&ct, boost::TIME_UTC);

  for (ism_type::iterator pos=get_ism().begin(); 
       pos!= get_ism().end(); ++pos) {
    
    // Check the state of the ClassAd information
    if (boost::tuples::get<2>(pos->second) != NULL) {
      
      // If the ClassAd information is not NULL, go on with the updating
      int diff = ct.sec - boost::tuples::get<0>(pos->second);
      
      // Check if .. is greater than expiry time
      if (diff > boost::tuples::get<1>(pos->second)) {
        // Check if function object wrapper is not empty
        if (!boost::tuples::get<3>(pos->second).empty()) {
          value = update_ism_entry()(pos->second);
          if (value == false) {
  	    // Reset ClassAd
            // (boost::tuples::get<2>(pos->second)).reset();
            get_ism().erase(pos);
          } else {
	    // Update ism entry
            boost::xtime_get(&ct, boost::TIME_UTC);
            pos->second = make_tuple(ct.sec,
                                     boost::tuples::get<1>(pos->second),
                                     boost::tuples::get<2>(pos->second),
                                     boost::tuples::get<3>(pos->second));
	  }
        }
        else {
          // If the function object wrapper is empty, remove the entry
          get_ism().erase(pos);
        } 
      }
    } else {
      // If the ClassAd information is NULL, remove the entry by ism
      get_ism().erase(pos);
    }

  }
}

bool update_ism_entry::operator()(ism_entry_type entry) 
{
  return boost::tuples::get<3>(entry)(
  	boost::tuples::get<1>(entry), 
	boost::tuples::get<2>(entry));
}

// Returns whether the entry has expired, or not
bool is_expired_ism_entry(const ism_entry_type& entry)
{
  boost::xtime ct;
  boost::xtime_get(&ct, boost::TIME_UTC);
  int diff = ct.sec - boost::tuples::get<0>(entry);
  return (diff > boost::tuples::get<1>(entry));
}

bool is_void_ism_entry(const ism_entry_type& entry)
{
  return (boost::tuples::get<1>(entry)<=0);
}

// get IsmDump file
std::string get_ism_dump(void)
{
  configuration::Configuration const* const config = configuration::Configuration::instance();
  assert(config);

  configuration::WMConfiguration const* const wm_config = config->wm();
  assert(wm_config);

  return wm_config->ism_dump();
}

void call_dump_ism_entries::operator()()
{
  boost::recursive_mutex::scoped_lock l(get_ism_mutex());
  std::ofstream             outf(get_ism_dump().c_str());
  
  for (ism_type::iterator pos=get_ism().begin();
       pos!= get_ism().end(); ++pos) {
    classad::ClassAd          ad_ism_dump;

    ad_ism_dump.InsertAttr("id", pos->first);
    ad_ism_dump.InsertAttr("update_time", boost::tuples::get<0>(pos->second));
    ad_ism_dump.InsertAttr("expiry_time", boost::tuples::get<1>(pos->second));
    ad_ism_dump.Insert("info", boost::tuples::get<2>(pos->second).get()->Copy());

    outf << ad_ism_dump;
  }
}

}
}
}
