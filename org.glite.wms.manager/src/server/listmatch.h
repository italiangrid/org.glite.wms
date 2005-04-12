/*
 * File: listjobmatch.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Author: Cinzia Di Giusto <Cinzia.DiGiusto@cnaf.infn.it>
 */

#include <string>

namespace classad {
    class ClassAd;
}
namespace glite {
namespace wms {
namespace manager {
namespace server {
            
bool match(
  classad::ClassAd const& jdl, 
  std::string const& result_file,
  int number_of_results, 
  bool include_brokerinfo
);


} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite

