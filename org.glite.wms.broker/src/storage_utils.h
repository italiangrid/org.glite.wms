#ifndef _GLITE_WMS_BROKER_STORAGE_UTILS_H_
#define _GLITE_WMS_BROKER_STORAGE_UTILS_H_

#include "glite/wms/rls/catalog_access_utils.h"
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <set>
#include <numeric>
#include <algorithm>

namespace classad
{
  class ClassAd;
}

namespace glite {
namespace wms {
namespace broker {

typedef glite::wms::rls::filemapping filemapping;

typedef
std::map<
  std::string, // se
  boost::tuple<
    std::vector<std::pair<std::string,int> >,         // protocols and ports
    std::vector<filemapping::const_iterator>,         // link to supplied data files
    std::vector<std::pair<std::string, std::string> > // close computing elements and mount points
  >
> storagemapping;
namespace tag
{
  enum { storage_info, filemapping_link, ce_bind_info };
}
boost::shared_ptr<storagemapping>
resolve_storagemapping_info(
  boost::shared_ptr<filemapping> fm
);

boost::shared_ptr<filemapping>
resolve_filemapping_info(
  const classad::ClassAd& requestAd
);

std::vector<storagemapping::const_iterator>
select_compatible_storage(
  storagemapping const& sm,
  std::vector<std::string>const& dap
);
 
size_t
count_unique_logical_files(
  std::vector<storagemapping::const_iterator>::const_iterator first,
  std::vector<storagemapping::const_iterator>::const_iterator last
);

struct extract_computing_elements_id : std::binary_function<
  std::set<std::string>*,
  storagemapping::const_iterator,
  std::set<std::string>*
>
{
  std::set<std::string>*
  operator()(
    std::set<std::string>* s,
    storagemapping::const_iterator si
  );
};

struct is_storage_close_to
{
  is_storage_close_to(std::string const& id);
  bool operator()(storagemapping::const_iterator const& v);
  std::string m_ceid;
};

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif
