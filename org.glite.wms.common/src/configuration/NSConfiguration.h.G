#ifndef GLITE_WMS_COMMON_CONFIGURATION_NSCONFIGURATION_H
#define GLITE_WMS_COMMON_CONFIGURATION_NSCONFIGURATION_H

#include "confbase.h"

COMMON_NAMESPACE_BEGIN {

namespace configuration {

class NSConfiguration : public confbase_c {
public:
  NSConfiguration( const classad::ClassAd *ad );

  ~NSConfiguration( void );

  /* Tmpl: partial include class NSConfiguration */
};

}; // configuration namespace closure

} COMMON_NAMESPACE_END;

#endif /* GLITE_WMS_COMMON_CONFIGURATION_NSCONFIGURATION_H */

// Local Variables:
// mode: c++
// End:
