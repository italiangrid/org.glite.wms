#ifndef GLITE_WMS_COMMON_CONFIGURATION_JCCONFIGURATION_H
#define GLITE_WMS_COMMON_CONFIGURATION_JCCONFIGURATION_H

#include "confbase.h"

COMMON_NAMESPACE_BEGIN {

namespace configuration {

class JCConfiguration : public confbase_c {
public:
  JCConfiguration( const classad::ClassAd *ad );

  ~JCConfiguration( void );

  /* Tmpl: partial include class JCConfiguration */
};

}; // configuration namespace closure

} COMMON_NAMESPACE_END;

#endif /* GLITE_WMS_COMMON_CONFIGURATION_JCCONFIGURATION_H */

// Local Variables:
// mode: c++
// End:
