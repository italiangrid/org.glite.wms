#ifndef EDG_WORKLOAD_COMMON_CONFIGURATION_NSCONFIGURATION_H
#define EDG_WORKLOAD_COMMON_CONFIGURATION_NSCONFIGURATION_H

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

#endif /* EDG_WORKLOAD_COMMON_CONFIGURATION_NSCONFIGURATION_H */

// Local Variables:
// mode: c++
// End:
