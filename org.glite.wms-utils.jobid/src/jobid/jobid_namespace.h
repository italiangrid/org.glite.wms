#ifndef __COMMON_NAMESPACE_H_LOADED
#define __COMMON_NAMESPACE_H_LOADED

#define COMMON_NAMESPACE_BEGIN namespace glite { namespace wms { namespace common

#define COMMON_NAMESPACE_END }}

#define USING_COMMON_NAMESPACE using namespace glite::wms::common
#define USING_COMMON_NAMESPACE_ADD( last ) using namespace glite::wms::common::##last

#define COMMON_NAMESPACE_CLASS( Type )                  \
namespace glite { namespace wms { namespace common { \
  class Type;                                           \
}}}

#define COMMON_SUBNAMESPACE_CLASS( Namespace, Type )    \
namespace glite { namespace wms { namespace common { \
  namespace Namespace {                                 \
    class Type;                                         \
  }                                                     \
}}}

#endif /* __COMMON_NAMESPACE_H_LOADED */

// Local Variables:
// mode: c++
// End:
