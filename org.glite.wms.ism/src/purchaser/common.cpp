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

  std::string const requirements_str(
    "["
    "  CloseOutputSECheck = IsUndefined(other.OutputSE)"
    "    ||   member(other.OutputSE,GlueCESEBindGroupSEUniqueID);"
    "  AuthorizationCheck = member(other.CertificateSubject, GlueCEAccessControlBaseRule)"
    "    ||   member(strcat(\"VO:\",other.VirtualOrganisation), GlueCEAccessControlBaseRule);"
    "  requirements = AuthorizationCheck && CloseOutputSECheck;"
    "]"
    );

   std::string const gangmatch_storage_ad_str(
    "["
    "  storage =  ["
    "     CEid = parent.GlueCEUniqueID;"
    "     VO = parent.other.VirtualOrganisation;"
    "     additionalSESAInfo = listAttrRegEx(\"^GlueS[EA].*\","
    "       parent.other.requirements);"
    "     CloseSEs = retrieveCloseSEsInfo( CEid, VO, additionalSESAInfo );"
    "  ];"
    "]"
  );

  boost::scoped_ptr<classad::ClassAd> requirements_ad;
  boost::scoped_ptr<classad::ClassAd> gangmatch_storage_ad;
}

bool expand_information_service_info(gluece_info_type& gluece_info)
{
  string isURL;
  bool result = false;
  try {

    isURL = utils::evaluate_attribute(*gluece_info, "GlueInformationServiceURL");
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
  } catch (utils::InvalidValue& e) {
    Error("Cannot evaluate GlueInformationServiceURL...");
    result = false;
  }
  return result;
}

bool insert_gangmatch_storage_ad(gluece_info_type& gluece_info)
{
 try {
    if(!gangmatch_storage_ad) {
      gangmatch_storage_ad.reset( 
        utils::parse_classad(gangmatch_storage_ad_str)
      ); 
    }
    gluece_info->Update(
      *gangmatch_storage_ad
    );
  }
  catch(...) {
    assert(false);
  }
  return true;
}

bool insert_aux_requirements(gluece_info_type& gluece_info)
{
    try {
    if(!requirements_ad) {
      requirements_ad.reset(
        utils::parse_classad(requirements_str)
      );
    }
    gluece_info->Update(
      *requirements_ad
    );
  }
  catch(...) {
    assert(false);
  }
  return true;
}

bool expand_glueceid_info(gluece_info_type& gluece_info)
{
  string ce_str;
  ce_str.assign(utils::evaluate_attribute(*gluece_info, "GlueCEUniqueID"));
  static boost::regex  expression_ceid("(.+/[^\\-]+-([^\\-]+))-(.+)");
  boost::smatch  pieces_ceid;
  string gcrs, type, name;
  
  if (boost::regex_match(ce_str, pieces_ceid, expression_ceid)) {
    
    gcrs.assign(pieces_ceid[1].first, pieces_ceid[1].second);
    try {
      type.assign(utils::evaluate_attribute(*gluece_info, "GlueCEInfoLRMSType"));
    } 
    catch(utils::InvalidValue& e) {
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

bool split_information_service_url(classad::ClassAd const& ad, boost::tuple<std::string, int, std::string>& i)
{
 try {
  std::string ldap_dn;
  std::string ldap_host;
  std::string ldap_url;

  ldap_url.assign( utils::evaluate_attribute(ad, "GlueInformationServiceURL") );
  static boost::regex expression_gisu( "\\S.*://(.*):([0-9]+)/(.*)" );
  boost::smatch pieces_gisu;
  std::string port;

  if (boost::regex_match(ldap_url, pieces_gisu, expression_gisu)) {

    ldap_host.assign (pieces_gisu[1].first, pieces_gisu[1].second);
    port.assign      (pieces_gisu[2].first, pieces_gisu[2].second);
    ldap_dn.assign   (pieces_gisu[3].first, pieces_gisu[3].second);

    i = boost::make_tuple(ldap_host, std::atoi(port.c_str()), ldap_dn);
  }
  else {
  return false;
  }
 }
 catch (utils::InvalidValue& e) {
   return false;
 }
 return true;
}

boost::xtime get_current_time(void)
{
  boost::xtime current_time;

  boost::xtime_get(&current_time, boost::TIME_UTC);
  
  return current_time;
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
