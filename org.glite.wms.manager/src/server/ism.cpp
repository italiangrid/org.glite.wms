#include <iostream>
#include <boost/utility.hpp>
#include "ism.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include <classad_distribution.h>

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

using namespace std;
namespace requestad = glite::wms::jdl;
namespace logger    = glite::wms::common::logger;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {
  ism_type ism;

  boost::mutex ism_mutex;
  timestamp_type ism_time;
}
	
ism_type::value_type make_ism_entry(std::string ce_id,
	       	timestamp_type xt,
	       	ce_ad_ptr ce_ad)
{
  return std::make_pair(ce_id, boost::make_tuple(xt, ce_ad));
}

boost::mutex& get_ism_mutex(void)
{
  return ism_mutex;
}

ism_type& get_ism(void)
{
  return ism;
}

timestamp_type& get_current_time(void)
{
  boost::xtime_get(&ism_time, boost::TIME_UTC);
  
  return ism_time;
}

ce_id_type get_ce_id(ce_ad_ptr ad)
{
try {
  std::string ce_id(requestad::get_ce_id(*ad));
  
  return ce_id;
} catch (requestad::ManipulationException const& e) {
  edglog(warning) << e.what() << endl;
  return ce_id_type(); 
} catch (std::exception const& e) {
  edglog(warning) << e.what() << endl;
  return ce_id_type();
} catch (...) {
  edglog( warning ) << "Caught exception while getting ce_id information" << endl;
  return ce_id_type();
}
}

std::ostream&
operator<<(std::ostream& os, const ism_type::value_type& value)
{    	
  return os << "[" << value.first << "]" << endl 
            << boost::tuples::get<0>(value.second).sec << endl
	    << boost::tuples::get<0>(value.second).nsec << endl
	    << *boost::tuples::get<1>(value.second) << endl 
	    << "[END]" << endl << endl;
}

}
}
}
}
