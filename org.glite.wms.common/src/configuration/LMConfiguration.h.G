#ifndef GLITE_WMS_COMMON_CONFIGURATION_LMCONFIGURATION_H
#define GLITE_WMS_COMMON_CONFIGURATION_LMCONFIGURATION_H

#include "confbase.h"

COMMON_NAMESPACE_BEGIN {

namespace configuration {

class LMConfiguration : public confbase_c {
public:
  LMConfiguration( const classad::ClassAd *ad );

  ~LMConfiguration( void );

  /* Tmpl: partial include class LMConfiguration */
};

}; // configuration namespace closure

} COMMON_NAMESPACE_END;

#endif /* GLITE_WMS_COMMON_CONFIGURATION_LMCONFIGURATION_H */

// Local Variables:
// mode: c++
// End:
