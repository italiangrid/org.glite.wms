#include "WMPExpDagAd.h"
WMPExpDagAd::setReserved (const std::string attr_name , cons std::string attr_value ){
	classad::Value v;
	v.SetStringValue(attr_value);
	dagad->set_generic( attr_name, classad::Literal::MakeLiteral(v));
}
