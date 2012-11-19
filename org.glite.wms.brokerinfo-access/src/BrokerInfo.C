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
* filename : BrokerInfo.C
* authors : Livio Salconi <livio.salconi@pi.infn.it>
* copyright : (C) 2001 by INFN
***************************************************************************/

// $Id: BrokerInfo.C,v 1.5.2.3 2010/04/07 12:53:50 mcecchi Exp $

#include "classad_distribution.h"
#include <fstream>
#include <cassert>
#include <iostream>
#include <ctype.h>

#include "BrokerInfo.h"

#ifdef WANT_NAMESPACES
#define CLASSAD( x )  classad::x
#else
#define CLASSAD( x )  x
#endif

BrokerInfo* BrokerInfo::instance_ = NULL;

BrokerInfo::BrokerInfo(void) {

  char *c_BrokerInfoFile_ = getenv("GLITE_WMS_RB_BROKERINFO");
  if (c_BrokerInfoFile_ != 0) {
    BrokerInfoFile_ = c_BrokerInfoFile_;
  }else if((c_BrokerInfoFile_ = getenv("EDG_WL_RB_BROKERINFO"))!=0){
    BrokerInfoFile_ = c_BrokerInfoFile_;
  } else {
    BrokerInfoFile_ = ".BrokerInfo";
  }

  fbrokerinfo_.open(BrokerInfoFile_.c_str());
  if (! fbrokerinfo_.is_open()) {
    std::cerr << "Error:" << std::endl;
    std::cerr << " - environment GLITE_WMS_RB_BROKERINFO or EDG_WL_RB_BROKERINFO not defined" << std::endl;
    std::cerr << " - ./BrokerInfo file is not found" << std::endl;
    throw BrokerInfoEx();
  } else {
    while(! fbrokerinfo_.eof()) {
      std::string line;
      fbrokerinfo_ >> line;
      mbrokerinfo_ << line;
    }
    fbrokerinfo_.close();
    mbrokerinfo_ << std::ends;
  }
	ad_ = parserAD(mbrokerinfo_.str());
}

BrokerInfo* BrokerInfo::instance(void)
{
  if (instance_ == 0) {
    instance_ = new BrokerInfo();
  }
  return instance_;
}

CLASSAD(ClassAd*) BrokerInfo::parserAD(std::string buffer) {

  CLASSAD(ClassAdParser) parser;
	CLASSAD(ClassAd*) clAd;

	clAd = parser.ParseClassAd(buffer, PARSE_RULE);
  if(clAd == NULL) {
    std::cout << "Error: not ClassAd compliant file" << std::endl;
    throw BrokerInfoEx();
  }
	return clAd;
}

BI_Result BrokerInfo::searchAD(std::string attrName, std::string &attrExpr, CLASSAD(ClassAd*) clAd) {

  CLASSAD(ClassAdUnParser) unp;
  CLASSAD(ClassAdIterator) itr;
  CLASSAD(ExprTree*) bufferExpr;
  CLASSAD(Value) v;
  BI_Result foundName = BI_ERROR;
  std::string bufferName;

  attrExpr = "";
  itr.Initialize(*clAd);

  bufferExpr = clAd->Lookup(attrName);
  if (bufferExpr != NULL) {
        foundName = BI_SUCCESS;
        clAd->EvaluateExpr(bufferExpr,v);
        unp.Unparse(attrExpr, v);
  }
 return foundName;
}

void BrokerInfo::prettyString(std::string& outStr) {

  int begin = outStr.find("\"", 0);
	int end = outStr.find("\"", begin + 1);
	if ((begin != -1) && (end != -1)) {
	  outStr = outStr.substr(begin + 1, end - begin - 1);
	}
}

void BrokerInfo::prettyStrList(std::string buffer, std::vector<std::string>& outList) {

  // clear return data field
  outList.clear();

  int sb, se, sp0, sp1;
  std::string strparse;

	sb = buffer.find("{", 0);
	se = buffer.find("}", 0);
	sp0 = sb;

  while(sp0 < se) {
    sp1 = buffer.find_first_of(",", sp0 + 1);
		if (sp1 == -1) {
		  sp1 = se;
		}
		strparse = buffer.substr(sp0 + 1, sp1 - sp0 - 1);
    int begin = strparse.find("\"", 0);
		if (begin == -1) {
		  outList.push_back("");
		} else {
      int end = strparse.find_first_of("\"", begin + 1);
      outList.push_back(strparse.substr(begin + 1, end - begin - 1));
		}
    sp0 = sp1;
  }
}

void BrokerInfo::prettyCAdList(std::string buffer, std::vector<std::string>& outList) {

  int send, sp0, sp0x, sp1;

	// clear return data field
  outList.clear();

	sp0 = 0;
	sp1 = 0;
	sp0x = 0;
  send = buffer.length() - 1;

  while(sp0 < send) {
	  sp0 = buffer.find_first_of("[", sp0x);
		if(sp0 == -1) {
		  sp0 = send;
		}
		sp1 = buffer.find_first_of("]", sp0x);
		if(sp1 == -1) {
		  sp1 = send;
		}
		sp0x = buffer.find_first_of("[", sp0 + 1);
		if(sp0x == -1) {
		  sp0x = send;
		}
    while(sp0x < sp1) {
		  sp1 = buffer.find_first_of("]", sp1 + 1);
		  if(sp1 == -1) {
		    sp1 = send;
		  }
		  sp0x = buffer.find_first_of("[", sp0x + 1);
		  if(sp0x == -1) {
		    sp0x = send;
		  }
		}
		if ((sp0 < send) && (sp1 < send)) {
      outList.push_back(buffer.substr(sp0, sp1 - sp0 + 1));
		}
	}
}

std::string BrokerInfo::getBIFileName(void) {

  return BrokerInfoFile_;
}

BI_Result BrokerInfo::getCE(std::string& CE) {

  BI_Result retVal;
	std::string  buffer;

	retVal = searchAD("ComputingElement", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  retVal = searchAD("name", CE, parserAD(buffer));
	}
  prettyString(CE);

	return retVal;
}

BI_Result BrokerInfo::getDataAccessProtocol(std::vector<std::string>& DAPs) {

  BI_Result retVal;
	std::string buffer;

	retVal = searchAD("DataAccessProtocol", buffer, ad_);
	if(retVal == BI_SUCCESS) {

          std::string::size_type bs = buffer.find("{");
          std::string::size_type be = buffer.rfind("}");

	  if( bs != std::string::npos && be != std::string::npos) {
            prettyStrList(buffer, DAPs);
          }
          else {
            prettyString(buffer);
            DAPs.push_back(buffer);
          }
	}
  return retVal;
}

BI_Result BrokerInfo::getLFN2SFN(std::string LFN, std::vector<std::string>& SFNs) {

  BI_Result retVal;
	std::string buffer, lfnName;
  std::vector<std::string> strVect;

	retVal = searchAD("InputFNs", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  prettyCAdList(buffer, strVect);
		int count = 0;
		int found = 0;
		while((found == 0) && (count < strVect.size())) {
      retVal = searchAD("name", lfnName, parserAD(strVect[count]));
			prettyString(lfnName);
      if(LFN.compare(lfnName) == 0) {
        found = 1;
        retVal = searchAD("SFNs", buffer, parserAD(strVect[count]));
	      prettyStrList(buffer, SFNs);
			}
			count++;
		}
	}
  return retVal;
}

BI_Result BrokerInfo::getSEs(std::vector<std::string>& SEs) {

  BI_Result retVal;
	std::string buffer, seName;
  std::vector<std::string> strVect;

	retVal = searchAD("StorageElements", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  prettyCAdList(buffer, strVect);
		int count = 0;
		while(count < strVect.size()) {
      retVal = searchAD("name", seName, parserAD(strVect[count]));
      prettyString(seName);
			SEs.push_back(seName);
			count++;
		}
	}
  return retVal;
}

BI_Result BrokerInfo::getSEProtocols(std::string SE, std::vector<std::string>& SEProtos) {

  BI_Result retVal;
	std::string buffer, seName, prName;
  std::vector<std::string> strVect;

	retVal = searchAD("StorageElements", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  prettyCAdList(buffer, strVect);
		int count = 0;
		int found = 0;
		while((found == 0) && (count < strVect.size())) {
      retVal = searchAD("name", seName, parserAD(strVect[count]));
			prettyString(seName);
      if(SE.compare(seName) == 0) {
        found = 1;
        retVal = searchAD("protocols", buffer, parserAD(strVect[count]));
        prettyCAdList(buffer, strVect);
				int counter = 0;
				while(counter < strVect.size()) {
          retVal = searchAD("name", prName, parserAD(strVect[counter]));
          prettyString(prName);
					SEProtos.push_back(prName);
					counter++;
				}
			}
			count++;
		}
	}
  return retVal;
}

BI_Result BrokerInfo::getSEPort(std::string SE, std::string SEProtocol, std::string& SEPort) {

  BI_Result retVal;
	std::string buffer, seName, prName;
  std::vector<std::string> strVect;

  retVal = searchAD("StorageElements", buffer, ad_);
  if(retVal == BI_SUCCESS) {
    prettyCAdList(buffer, strVect);
    int count = 0;
    int found = 0;
    while((found == 0) && (count < strVect.size())) {
         retVal = searchAD("name", seName, parserAD(strVect[count]));
	 prettyString(seName);
         if(SE.compare(seName) == 0) {
           found = 1;
           retVal = searchAD("protocols", buffer, parserAD(strVect[count]));
           prettyCAdList(buffer, strVect);
	   int counter = 0;
	   while(counter < strVect.size()) {
                retVal = searchAD("name", prName, parserAD(strVect[counter]));
                prettyString(prName);
	        if(SEProtocol.compare(prName) == 0) {
                retVal = searchAD("port", SEPort, parserAD(strVect[counter]));
	        }
	        counter++;
           }
	 }
	 count++;
    }
  }
  return retVal;
}

BI_Result BrokerInfo::getCloseSEs(std::vector<std::string>& SEs) {

  BI_Result retVal;
	std::string buffer, seName;
  std::vector<std::string> strVect;

	retVal = searchAD("ComputingElement", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  retVal = searchAD("CloseStorageElements", buffer, parserAD(buffer));
	}
	if(retVal == BI_SUCCESS) {
	  prettyCAdList(buffer, strVect);
		int count = 0;
		int found = 0;
		while((found == 0) && (count < strVect.size())) {
      retVal = searchAD("name", seName, parserAD(strVect[count]));
      prettyString(seName);
		  SEs.push_back(seName);
		  count++;
		}
	}
  return retVal;
}

BI_Result BrokerInfo::getSEMountPoint(std::string CloseSE, std::string& SEMount) {

  BI_Result retVal;
	std::string buffer, seName;
  std::vector<std::string> strVect;

	retVal = searchAD("ComputingElement", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  retVal = searchAD("CloseStorageElements", buffer, parserAD(buffer));
	}
	if(retVal == BI_SUCCESS) {
	  prettyCAdList(buffer, strVect);
		int count = 0;
		int found = 0;
		while((found == 0) && (count < strVect.size())) {
      retVal = searchAD("name", seName, parserAD(strVect[count]));
      prettyString(seName);
			if(CloseSE.compare(seName) == 0) {
        retVal = searchAD("mount", SEMount, parserAD(strVect[count]));
                                prettyString(SEMount);
			}
		  count++;
		}
	}
  return retVal;
}

BI_Result BrokerInfo::getSEFreeSpace(std::string CloseSE, std::string& SEFreeSpace) {

  BI_Result retVal;
	std::string buffer, seName;
  std::vector<std::string> strVect;

	retVal = searchAD("ComputingElement", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  retVal = searchAD("CloseStorageElements", buffer, parserAD(buffer));
	}
	if(retVal == BI_SUCCESS) {
	  prettyCAdList(buffer, strVect);
		int count = 0;
		int found = 0;
		while((found == 0) && (count < strVect.size())) {
      retVal = searchAD("name", seName, parserAD(strVect[count]));
      prettyString(seName);
			if(CloseSE.compare(seName) == 0) {
        retVal = searchAD("freespace", SEFreeSpace, parserAD(strVect[count]));
                                prettyString(SEFreeSpace);
			}
		  count++;
		}
	}
  return retVal;
}

BI_Result BrokerInfo::getInputData(std::vector<std::string>& LFNs) {

  BI_Result retVal;
	std::string buffer, seName;
  std::vector<std::string> strVect;

	retVal = searchAD("InputFNs", buffer, ad_);
	if(retVal == BI_SUCCESS) {
	  prettyCAdList(buffer, strVect);
		int count = 0;
		while(count < strVect.size()) {
      retVal = searchAD("name", seName, parserAD(strVect[count]));
			prettyString(seName);
			LFNs.push_back(seName);
			count++;
		}
	}
  return retVal;
}

BI_Result BrokerInfo::getVirtualOrganization(std::string& VO) {

  BI_Result retVal;

	retVal = searchAD("VirtualOrganisation", VO, ad_);
	if(retVal == BI_SUCCESS) {
	  prettyString(VO);
	}
  return retVal;
}
