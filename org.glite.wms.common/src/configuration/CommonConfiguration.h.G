#ifndef EDG_WORKLOAD_COMMON_CONFIGURATION_COMMONCONFIGURATION_H
#define EDG_WORKLOAD_COMMON_CONFIGURATION_COMMONCONFIGURATION_H

#include "confbase.h"

COMMON_NAMESPACE_BEGIN {

namespace configuration {

class CommonConfiguration : public confbase_c {
public:
  CommonConfiguration( const classad::ClassAd *ad );

  ~CommonConfiguration( void );

  /* Tmpl: partial include class CommonConfiguration */
};

}; // Namespace configuration

} COMMON_NAMESPACE_END;

#endif /* EDG_WORKLOAD_COMMON_CONFIGURATION_COMMONCONFIGURATION_H */

// Local Variables:
// mode: c++
// End:
