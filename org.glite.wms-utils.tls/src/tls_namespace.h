#ifndef __TLS_NAMESPACE_H_LOADED
#define __TLS_NAMESPACE_H_LOADED

#define TLS_NAMESPACE_BEGIN namespace glite { namespace wms { namespace tls

#define TLS_NAMESPACE_END }}

#define USING_TLS_NAMESPACE using namespace glite::wms::tls
#define USING_TLS_NAMESPACE_ADD( last ) using namespace glite::wms::tls::##last

#define TLS_NAMESPACE_CLASS( Type )                  \
namespace glite { namespace wms { namespace tls { \
  class Type;                                           \
}}}

#define TLS_SUBNAMESPACE_CLASS( Namespace, Type )    \
namespace glite { namespace wms { namespace tls { \
  namespace Namespace {                                 \
    class Type;                                         \
  }                                                     \
}}}

#endif /* __TLS_NAMESPACE_H_LOADED */

// Local Variables:
// mode: c++
// End:
