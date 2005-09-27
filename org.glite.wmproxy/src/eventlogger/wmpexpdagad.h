/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPEXPDAGAD_H
#define GLITE_WMS_WMPROXY_WMPEXPDAGAD_H

#include "glite/wms/jdl/ExpDagAd.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {


class WMPExpDagAd:public glite::wms::jdl::ExpDagAd
{
	public:
		WMPExpDagAd(const std::string &jdl):ExpDagAd(jdl) {};
		WMPExpDagAd(std::ifstream &jdl_in):ExpDagAd(jdl_in) {};
		WMPExpDagAd(const ExpDagAd &dag):ExpDagAd(dag) {};
		
		void setReserved(const std::string attr_name, const std::string attr_value);
};

} // eventlogger
} // wmproxy
} // wms
} // glite

#endif // GLITE_WMS_WMPROXY_WMPEXPDAGAD_H
