#include "CommonConfiguration.h"

using namespace std;

COMMON_NAMESPACE_BEGIN {

namespace configuration {

CommonConfiguration::CommonConfiguration( const classad::ClassAd *ad ) : confbase_c( ad )
{}

CommonConfiguration::~CommonConfiguration( void )
{}

}; // configuration namespace closure

} COMMON_NAMESPACE_END;
