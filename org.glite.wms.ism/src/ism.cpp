// $Id$

#include "glite/wms/ism/ism.h"
#include <iostream>
#include <classad_distribution.h>

namespace glite {
namespace wms {
namespace ism {

ism_type::value_type make_ism_entry(
  std::string const& ce_id,
  timestamp_type const& xt,
  ce_ad_ptr const& ce_ad
)
{
  return std::make_pair(ce_id, boost::make_tuple(xt, ce_ad));
}

boost::mutex& get_ism_mutex(void)
{
  static boost::mutex ism_mutex;

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
            << boost::tuples::get<0>(value.second).sec << '\n'
	    << boost::tuples::get<0>(value.second).nsec << '\n'
	    << *boost::tuples::get<1>(value.second) << '\n'
	    << "[END]";
}

}
}
}
