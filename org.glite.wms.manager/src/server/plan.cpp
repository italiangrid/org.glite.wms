#include "plan.h"

classad::ClassAd* Plan(classad::ClassAd const& ad)
{
  glite::wms::helper::Request request(&ad);

  while (!request.is_resolved()) {
    request.resolve();
  }

  classad::ClassAd const* res_ad = resolved_ad(request);
  return res_ad ? new classad::ClassAd(*res_ad) : 0;
}
