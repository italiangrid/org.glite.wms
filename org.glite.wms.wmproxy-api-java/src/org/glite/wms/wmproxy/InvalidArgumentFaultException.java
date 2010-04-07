/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */
/*
 * Copyright (c) 2005 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 * 
 * Authors: Marco Sottilaro (marco.sottilaro@datamat.it)
 */


package org.glite.wms.wmproxy;

/**
 * This exception is thrown to indicate that an input argument of the requested operation is not valid
 *  (i.e. the JobId string, the JDL expression, a single JDL attribute expresion, etc..)
 */

public class InvalidArgumentFaultException extends BaseException {
	/**
	* Constructs an Exception with no specified detail message.
	*/
	public InvalidArgumentFaultException () {
		super();
	}
	/**
	* Constructs an Exception with the specified detail message.
	*/
	public InvalidArgumentFaultException(String message) {
		super(message);
	}
}
