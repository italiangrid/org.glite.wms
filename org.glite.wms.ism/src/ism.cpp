// $Id$

#include <iostream>
#include <fstream>
#include <cstdio>

#include <boost/lexical_cast.hpp>

#include <classad_distribution.h>

#include "glite/wms/ism/ism.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/logger/logger_utils.h"

namespace configuration = glite::wms::common::configuration;

namespace {
  int s_active_side[2];
  int s_matching_threads[2];
}

namespace glite {
namespace wms {
namespace ism {

void switch_active_side()
{
  ism_mutex_type::scoped_lock l(get_ism_mutex(ism::ce));
  s_active_side[ism::ce] = (s_active_side[ism::ce] + 1) % 2;
  if (s_matching_threads[s_active_side[ism::ce]] != 0) {
    assert(true);
  }
}

int active_side()
{
  ism_mutex_type::scoped_lock l(get_ism_mutex(ism::ce));
  ++s_matching_threads[s_active_side[ism::ce]];
  return s_active_side[ism::ce];
}

ism_type::value_type make_ism_entry(
  std::string const& id, // resource identifier
  int update_time, // update time
  ad_ptr const& ad, // resource descritpion
  update_function_type const& uf, // update function
  int expiry_time // expiry time with default 5*60
)
{
  boost::shared_ptr<boost::mutex> mt(new boost::mutex);
  return std::make_pair(
    id,
    boost::make_tuple(update_time, expiry_time, ad, uf, mt)
  );
}

namespace {

ism_type* the_ism1[2];
ism_type* the_ism2[2];
ism_mutex_type* the_ism_mutex[2];

}

void set_ism(
  ism_type* ism1,
  ism_type* ism2,
  ism_mutex_type* ism_mutex,
  size_t the_ism_index
)
{
  the_ism1[the_ism_index] = ism1 + the_ism_index;
  the_ism2[the_ism_index] = ism2 + the_ism_index;
  the_ism_mutex[the_ism_index] = ism_mutex + the_ism_index;
}

ism_mutex_type& get_ism_mutex(size_t the_ism_index)
{
  return *the_ism_mutex[the_ism_index];
}

ism_type& get_ism(size_t the_ism_index, int face)
{
  if (face) {
    return *the_ism1[the_ism_index];
  } else {
    return *the_ism2[the_ism_index];
  }
}

ism_type& get_ism(size_t the_ism_index)
{
  if (s_active_side) {
    return *the_ism2[the_ism_index];
  } else {
    return *the_ism1[the_ism_index];
  }
}

int matching_threads(int side)
{
  return s_matching_threads[side];
}

void matched_thread(int side)
{
  --s_matching_threads[side];
  if (side != s_active_side[ism::ce] && s_matching_threads[side] == 0) {
    get_ism(ism::ce, side).clear(); // updater thread not needed anymore
  }
}

std::ostream&
operator<<(std::ostream& os, ism_type::value_type const& value)
{
  return os << '[' << value.first << "]\n"
            << boost::tuples::get<update_time_entry>(value.second) << '\n'
	    << boost::tuples::get<expiry_time_entry>(value.second) << '\n'
	    << *boost::tuples::get<ad_ptr_entry>(value.second) << '\n'
	    << "[END]";
}

void call_update_ism_entries::operator()()
{
  _(ce);
  _(se);
}

void call_update_ism_entries::_(size_t the_ism_index)
{
  ism_mutex_type::scoped_lock l(get_ism_mutex(the_ism_index));
  time_t current_time = std::time(0);

  ism_type::iterator pos=get_ism(the_ism_index).begin();
  ism_type::iterator const e=get_ism(the_ism_index).end();

  for ( ; pos!=e; ) {

    bool inc_done = false;
    // Check the state of the ClassAd information
    if (boost::tuples::get<ad_ptr_entry>(pos->second) != NULL) {
      // If the ClassAd information is not NULL, go on with the updating
      int diff = current_time - boost::tuples::get<update_time_entry>(pos->second);
      // Check if .. is greater than expiry time
      if (diff > boost::tuples::get<expiry_time_entry>(pos->second)) {
        // Check if function object wrapper is not empty
        if (!boost::tuples::get<update_function_entry>(pos->second).empty()) {
          bool is_updated = update_ism_entry()(pos->second);
          if (!is_updated) {
            // if the update function returns false we remove the entry
            // only if it has been previously marked as invalid i.e. the entry's 
            // update time is less than 0
  	    if (boost::tuples::get<update_time_entry>(pos->second)<0) {
              get_ism(the_ism_index).erase(pos++);
              inc_done = true;
            } else {
              boost::tuples::get<update_time_entry>(pos->second) = -1;
            }
	  }
          else {
            boost::tuples::get<update_time_entry>(pos->second) = current_time;
          }
        }
        else {
          // If the function object wrapper is empty, remove the entry
          get_ism(the_ism_index).erase(pos++);
          inc_done = true;
        }
      }
    } else {
      // If the ClassAd information is NULL, remove the entry by ism
      get_ism(the_ism_index).erase(pos++);
      inc_done = true;
    }
    if (!inc_done) ++pos;
  }
}

bool update_ism_entry::operator()(ism_entry_type entry) 
{
  return boost::tuples::get<update_function_entry>(entry)(
  	boost::tuples::get<expiry_time_entry>(entry), 
	boost::tuples::get<ad_ptr_entry>(entry));
}

// Returns whether the entry has expired, or not
bool is_expired_ism_entry(const ism_entry_type& entry)
{
  boost::xtime ct;
  boost::xtime_get(&ct, boost::TIME_UTC);
  int diff = ct.sec - boost::tuples::get<update_time_entry>(entry);
  return (diff > boost::tuples::get<expiry_time_entry>(entry));
}

bool is_void_ism_entry(const ism_entry_type& entry)
{
  return (boost::tuples::get<expiry_time_entry>(entry)<=0);
}

//get IsmDump file
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
  std::string const dump(get_ism_dump());
  std::string const tmp_dump(dump + ".tmp");

  _(ce, std::iostream::ios_base::trunc, tmp_dump);
  _(se, std::iostream::ios_base::app, tmp_dump);

  int res = std::rename(tmp_dump.c_str(), dump.c_str());
  if (res) {
    Warning("Cannot rename ISM dump file (error "
      + boost::lexical_cast<std::string>(res) + ')'
    );
  }
}

void call_dump_ism_entries::_(
  size_t the_ism_index,
  std::iostream::ios_base::openmode open_mode,
  std::string const& filename)
{
  std::ofstream outf(filename.c_str(), open_mode);
  ism_mutex_type::scoped_lock l(get_ism_mutex(the_ism_index));
  for (ism_type::iterator pos=get_ism(the_ism_index).begin();
       pos!= get_ism(the_ism_index).end(); ++pos) {
    if (boost::tuples::get<2>(pos->second)) {
      classad::ClassAd          ad_ism_dump;

      ad_ism_dump.InsertAttr("id", pos->first);
      ad_ism_dump.InsertAttr("update_time",
        boost::tuples::get<update_time_entry>(pos->second));
      ad_ism_dump.InsertAttr("expiry_time",
        boost::tuples::get<expiry_time_entry>(pos->second));
      ad_ism_dump.Insert("info",
        boost::tuples::get<ad_ptr_entry>(pos->second).get()->Copy());
      outf << ad_ism_dump;
    }
  }
}

}}}
