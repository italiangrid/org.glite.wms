#ifndef GLITE_WMS_WMPROXY_WMPEXPDAGAD_H
#define GLITE_WMS_WMPROXY_WMPEXPDAGAD_H
#include "glite/wms/jdl/ExpDagAd.h"
class WMPExpDagAd:glite::wms::jdl::ExpDagAd{
	public:
		setReserved( const std::string attr_name , cons std::string attr_value );
};

#endif // GLITE_WMS_WMPROXY_WMPEXPDAGAD_H
