#include "LMConfiguration.h"

using namespace std;

COMMON_NAMESPACE_BEGIN {

namespace configuration {

LMConfiguration::LMConfiguration( const classad::ClassAd *ad ) : confbase_c( ad )
{}

LMConfiguration::~LMConfiguration( void )
{}

}; // configuration namespace closure

} COMMON_NAMESPACE_END;
