#include "JCConfiguration.h"

using namespace std;

COMMON_NAMESPACE_BEGIN {

namespace configuration {

JCConfiguration::JCConfiguration( const classad::ClassAd *ad ) : confbase_c( ad )
{}

JCConfiguration::~JCConfiguration( void )
{}

}; // configuration namespace closure

} COMMON_NAMESPACE_END;
