#include "NSConfiguration.h"

COMMON_NAMESPACE_BEGIN {

namespace configuration {

NSConfiguration::NSConfiguration( const classad::ClassAd *ad ) : confbase_c( ad )
{}

NSConfiguration::~NSConfiguration( void )
{}

}; // configuration namespace closure

} COMMON_NAMESPACE_END;
