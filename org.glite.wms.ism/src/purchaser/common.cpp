// File: common.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "glite/wms/ism/purchaser/common.h"

using namespace std;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

namespace {
  boost::scoped_ptr<classad::ClassAd> requirements_ad;
}

bool expand_information_service_info(gluece_info_type& gluece_info)
{
  string isURL;
  bool result = false;
  try {

    isURL = utilities::evaluate_attribute(*gluece_info, "GlueInformationServiceURL");
    static boost::regex  expression_gisu("\\S.*://(.*):([0-9]+)/(.*)");
    boost::smatch        pieces_gisu;

    if (boost::regex_match(isURL, pieces_gisu, expression_gisu)) {

      string ishost(pieces_gisu[1].first, pieces_gisu[1].second);
      string isport(pieces_gisu[2].first, pieces_gisu[2].second);
      string isbasedn(pieces_gisu[3].first, pieces_gisu[3].second);

      gluece_info->InsertAttr("InformationServiceDN", isbasedn);
      gluece_info->InsertAttr("InformationServiceHost", ishost);
      gluece_info->InsertAttr("InformationServicePort",
                              boost::lexical_cast<int>(isport));
      result = true;
    }
  } catch (utilities::InvalidValue& e) {
    Error("Cannot evaluate GlueInformationServiceURL...");
    result = false;
  }
  return result;
}

void insert_aux_requirements(gluece_info_type& gluece_info)
{
  if (!requirements_ad) {
    
    std::string requirements_str(
      "[ \
        CloseOutputSECheck = IsUndefined(other.OutputSE) \
          ||   member(other.OutputSE, GlueCESEBindGroupSEUniqueID); \
        AuthorizationCheck = member(other.CertificateSubject, GlueCEAccessControlBaseRule) \
          ||   member(strcat(\"VO:\",other.VirtualOrganisation), GlueCEAccessControlBaseRule); \
      ]"
    );

    try {
      requirements_ad.reset(utilities::parse_classad(requirements_str));
    }
    catch(...) {
      std::cout << "Ops!" << std::endl;
      exit( -1 );
    }
  }
  gluece_info->Update(*requirements_ad);
}

bool expand_glueceid_info(gluece_info_type& gluece_info)
{
  string ce_str;
  ce_str.assign(utilities::evaluate_attribute(*gluece_info, "GlueCEUniqueID"));
  static boost::regex  expression_ceid("(.+/[^\\-]+-(.+))-(.+)");
  boost::smatch  pieces_ceid;
  string gcrs, type, name;
  
  if (boost::regex_match(ce_str, pieces_ceid, expression_ceid)) {
    
    gcrs.assign(pieces_ceid[1].first, pieces_ceid[1].second);
    try {
      type.assign(utilities::evaluate_attribute(*gluece_info, "GlueCEInfoLRMSType"));
    } 
    catch(utilities::InvalidValue& e) {
      // Try to fall softly in case the attribute is missing...
      type.assign(pieces_ceid[2].first, pieces_ceid[2].second);
      Warning("Cannot evaluate GlueCEInfoLRMSType using value from contact string: " << type);
    }
    // ... or in case the attribute is empty.
    if (type.length() == 0) type.assign(pieces_ceid[2].first, pieces_ceid[2].second);
    name.assign(pieces_ceid[3].first, pieces_ceid[3].second);
  }
  else { 
    Warning("Cannot parse CEid=" << ce_str);
    return false;
  }
  gluece_info -> InsertAttr("GlobusResourceContactString", gcrs);
  gluece_info -> InsertAttr("LRMSType", type);
  gluece_info -> InsertAttr("QueueName", name);
  gluece_info -> InsertAttr("CEid", ce_str);
  return true;
}

timestamp_type get_current_time(void)
{
  timestamp_type current_time;

  boost::xtime_get(&current_time, boost::TIME_UTC);
  
  return current_time;
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
