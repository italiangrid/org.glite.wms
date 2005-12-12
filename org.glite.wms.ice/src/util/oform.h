#ifndef __GLITE_WMS_ICE_UTIL_OFORM_H__
#define __GLITE_WMS_ICE_UTIL_OFORM_H__

#include <string>

std::string oform( const char* fmt, ... )
{
    static char buf[ 256 ];

    va_list ap;
    va_start( ap, fmt );
    vsnprintf( buf, 256, fmt, ap );
    va_end( ap );
    return std::string( buf );
} ;

#endif
