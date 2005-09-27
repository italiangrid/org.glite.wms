/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmpexpdagad.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {
	
void WMPExpDagAd::setReserved(const std::string attr_name,
	const std::string attr_value)
{
	classad::Value v;
	v.SetStringValue(attr_value);
	dagad->set_generic(attr_name, classad::Literal::MakeLiteral(v));
}

} // eventlogger
} // wmproxy
} // wms
} // glite
