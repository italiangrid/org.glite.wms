#ifndef __USERINTERFACE_NAMESPACE_H_LOADED
#define __USERINTERFACE_NAMESPACE_H_LOADED
#include <string>
#define USERINTERFACE_NAMESPACE_BEGIN namespace edg { namespace workload { namespace userinterface {
#define USERINTERFACE_NAMESPACE_END }}
#define USING_USERINTERFACE_NAMESPACE using namespace edg::workload::userinterface
#define USING_USERINTERFACE_NAMESPACE_ADD( last ) using namespace edg::workload::userinterface::##last

#endif /* __USERINTERFACE_NAMESPACE_H_LOADED */

// Local Variables:
// mode: c++
// End:
