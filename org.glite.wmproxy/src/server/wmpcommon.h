/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpcommon.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "glite/wms/jdl/ExpDagAd.h"


// Possible values for jdl type attribute
enum type {
	TYPE_JOB,
	TYPE_DAG,
	TYPE_COLLECTION,
};

// Common methods used in both operations and coreoperations
void setGlobalSandboxDir();
int logRemoteHostInfo();
int getType(std::string jdl, glite::wms::jdl::Ad * ad = NULL);
