#ifndef __COMMON_NAMESPACE_H_LOADED
#define __COMMON_NAMESPACE_H_LOADED

#define COMMON_NAMESPACE_BEGIN namespace edg { namespace workload { namespace common

#define COMMON_NAMESPACE_END }}

#define USING_COMMON_NAMESPACE using namespace edg::workload::common
#define USING_COMMON_NAMESPACE_ADD( last ) using namespace edg::workload::common::##last

#define COMMON_NAMESPACE_CLASS( Type )                  \
namespace edg { namespace workload { namespace common { \
  class Type;                                           \
}}}

#define COMMON_SUBNAMESPACE_CLASS( Namespace, Type )    \
namespace edg { namespace workload { namespace common { \
  namespace Namespace {                                 \
    class Type;                                         \
  }                                                     \
}}}

#endif /* __COMMON_NAMESPACE_H_LOADED */

// Local Variables:
// mode: c++
// End:
