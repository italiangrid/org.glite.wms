/*
 * File: listmatch.cpp
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Author: Cinzia Di Giusto <Cinzia.DiGiusto@cnaf.infn.it>
 */

#include <iostream>
#include <fstream>

#include "listmatch.h"
#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/helper/Helper.h"
#include "glite/wms/helper/exceptions.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

bool match(classad::ClassAd const& jdl, std::string const& result_file)
{
  std::auto_ptr<classad::ClassAd> result;

  try {
    result.reset(glite::wms::helper::Helper("MatcherHelper").resolve(&jdl)); 
  }
  catch (glite::wms::helper::HelperError const& e) {
    // We need to chime into the communication pipe in this case.
    result.reset(new classad::ClassAd);
    result->InsertAttr("reason", std::string("error accessing broker helper"));
    result->InsertAttr("match_result", new classad::ExprList);
  }
  
  if (result_file.empty() || result_file[0] != '/') {
    return false;
  }
  
  std::ofstream out(result_file.c_str());
  
  out << *result << '\n';
      
  return true;
}

} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite
