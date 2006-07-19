#ifndef  ORG_GLITE_WMSUI_WRAPY_SDWRAPPER_H
#define ORG_GLITE_WMSUI_WRAPY_SDWRAPPER_H
/*
 * SdWrapper.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */


#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/logger/common.h"
#include "glite/lb/JobStatus.h"
#include <vector>

/*********************
* VOMS includes:
*********************/
#include "glite/security/voms/voms_api.h"

/**
 * Provide a wrapper that manages user cerificate general information
 * it allows to check the proxy cerfticate's parameters such us validity, retrieve issuer etc etc.
 * it also allows to manipulate VirtualOrganisation certificate properties, retreving, if present, VO default names and groups
 *
 * @brief Service Discovery wrapper class
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
class  ServiceDiscovery {
	public:
		/** Constructor */
		ServiceDiscovery ( const std::string& vo) ;
		/** Service Discovery default Destructor
		*/
		~ServiceDiscovery();
};

#endif

