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

/***************************************************************************
* filename : BrokerInfo.h
* authors : Livio Salconi <livio.salconi@pi.infn.it>
* copyright : (C) 2001 by INFN
***************************************************************************/

// $Id: BrokerInfo.h,v 1.3.8.1 2010/04/07 12:53:50 mcecchi Exp $

#ifndef __BROKERINFO_H__
#define __BROKERINFO_H__

/*! \Set this to TRUE if BrokerInfo file contains only ClassAd syntax
 *  \@ or set it to FALSE if contains some more informations
 */
const bool PARSE_RULE = true;

#include <string>
#include <vector>
#include <strstream>
#include <fstream>

#ifdef WANT_NAMESPACES
#define CLASSAD( x )  classad::x
#else
#define CLASSAD( x )  x
#endif

#include "classad_distribution.h"

#include "bi_result.h"

class BrokerInfoEx {
};

class BrokerInfo {
  public:

  /*! Default Destructor.
   */
  ~BrokerInfo(void);

  /*! \Returns the handle to the instantiated object.
   */
  static BrokerInfo* instance(void);

  /*! \Returns the BrokerInfo file to parse.
   */
  std::string getBIFileName(void);
  
  /*! \returns CE (Computing Element) Info.
   *  \param CE  The ResourceID of the Computing Element to be returned.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getCE(std::string& CE);

  /*! \returns Data Access Protocol list
   *  \param DAPs Vector of Data Access Protocols specified by the user.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getDataAccessProtocol(std::vector<std::string>& DAPs);

  /*! \returns LFN to SFN mapping
   *  \param LFN String of LFN specified.
   *  \param SFNs Vector of corresponding SFNs.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getLFN2SFN(std::string LFN, std::vector<std::string>& SFNs);

  /*! \returns SE (Storage Element) list
   *  \param SEs Vector of SEs serving PFNs.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getSEs(std::vector<std::string>& SEs);

  /*! \returns SE Protocols list ordered following the order of SEs returned by getSEs
   *  \param SEProtcs Vector of protocols supported by SEs.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getSEProtocols(std::string SE, std::vector<std::string>& SEProtos);

  /*! \returns SE Ports list ordered following the order of SEs returned by getSEs
   *  \param SEPorts Vector of ports used by SEs.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getSEPort(std::string SE, std::string SEProtocol, std::string& SEPort);

  /*! \returns SE's protocol port number
   *  \param SE Vector of SEs close to the given CE.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getCloseSEs(std::vector<std::string>& SEs);

  /*! \returns SE MountPoint for the specific CloseSE returned by the getCloseSEs method.
   *  \param SEMount string of SE mount point.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getSEMountPoint(std::string CloseSE, std::string& SEMount);

  /*! \returns SE FreeSpace for the specific CloseSE returned by the getCloseSEs method.
   * \param SEFreeSpace string of SE free space.
   * \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
    BI_Result getSEFreeSpace(std::string CloseSE, std::string& SEFreeSpace);

  /*! \returns InputData list
   *  \param LFN/GUID/LFC vector.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getInputData(std::vector<std::string>& LFNs);

  /*! \returns Virtual Organization string
   *  \param VO string.
   *  \return BI_SUCCESS, or BI_ERROR if the operation failed
   */
  BI_Result getVirtualOrganization(std::string& VO);

  /*! \Parsing utilities
   */
  int parser(std::string& outbuffer);

  private:

  /*! \Default constructor. It reads BrokerInfo file (.BrokerInfo) in
   *  the default location (user working space).
   *  It fails if the BrokerInfo file cannot be found or is unreadable
   */
  BrokerInfo(void);

	/*! \Parsing utilities
   */
  BI_Result searchAD(std::string attrName, std::string& attrExpr, CLASSAD(ClassAd*) clAd);
	void prettyStrList(std::string buffer, std::vector<std::string>& outList);
	void prettyCAdList(std::string buffer, std::vector<std::string>& outList);
  void prettyString(std::string& outStr);
	CLASSAD(ClassAd*) parserAD(std::string buffer);

  std::string BrokerInfoFile_;
  std::ifstream fbrokerinfo_;
  std::strstream mbrokerinfo_;
  CLASSAD(ClassAd*) ad_;

  static BrokerInfo* instance_;
};

/*! \class BrokerInfo
 *  \brief parses .BrokerInfo file containing job submission parameters
 *  \param BrokerInfoFile_      holds the fullpath of the brokerinfo file
 *  \param fbrokerinfo_         handler to the brokerinfo file
 *  \param mbrokerinfo_         handler to the brokerinfo memory info
 *  \param instance_            it points to the current instance.
 * this class is intended so to be instantiated only once (singleton).
 */

#endif // __BROKERINFO_H__
