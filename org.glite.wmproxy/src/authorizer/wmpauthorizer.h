/*
        Copyright (c) Members of the EGEE Collaboration. 2004.
        See http://public.eu-egee.org/partners/ for details on the copyright holders.
        For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPAUTHORIZER_H
#define GLITE_WMS_WMPROXY_WMPAUTHORIZER_H

#include <pwd.h>
#include <sys/types.h>

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

/**
 * WMPAuthorizer class provides a set of utility methods for performing
 * Grid Users authorization and mapping to local users. It also relies on 
 * the functions provided by the security lcas/lcmaps components.
 *
 * @brief utility methods for user authZ and local id mapping
 * @version 1.0
 * @date 2005
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
*/
class WMPAuthorizer {
public:
        WMPAuthorizer();
        virtual ~WMPAuthorizer() throw();

        /**
         * Calls LCAS to check if the user is authorized to submit requests to WMProxy. 
         * Check is done on the basis of user's credential.
         * @return true if the user is authorized, false otherwise
         */
        void checkUserAuthZ(FILE * fp);

        /**
         * Calls LCMAPS to map the grid user to a local user.
         * Mapping is done on the basis of user's credential.
         * @return the local user id.
         */
        uid_t getUserId(FILE * fp);
};

} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_WMPAUTHORIZER_H
