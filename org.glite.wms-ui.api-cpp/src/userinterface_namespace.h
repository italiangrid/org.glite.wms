#ifndef __USERINTERFACE_NAMESPACE_H_LOADED
#define __USERINTERFACE_NAMESPACE_H_LOADED
#include <string>
#define USERINTERFACE_NAMESPACE_BEGIN namespace glite { namespace wms { namespace userinterface {
#define USERINTERFACE_NAMESPACE_END }}
#define USING_USERINTERFACE_NAMESPACE using namespace glite::wms::userinterface
#define USING_USERINTERFACE_NAMESPACE_ADD( last ) using namespace glite::wms::userinterface::##last

#endif /* __USERINTERFACE_NAMESPACE_H_LOADED */

// Local Variables:
// mode: c++
// End:
