// File: Helper.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <fstream>
#include <stdexcept>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>

#include <classad_distribution.h>

#include "glite/gpbox/Clientcc.h"

#include "glite/wms/broker/RBSimpleISMImpl.h"
#include "glite/wms/broker/RBMaximizeFilesISMImpl.h"

#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

#include "glite/wms/classad_plugin/classad_plugin_loader.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/helper/HelperFactory.h"
#include "glite/wms/helper/broker/exceptions.h"
#include "glite/wms/helper/exceptions.h"

#include "glite/wms/ism/ism.h"

#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wms/matchmaking/exceptions.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

#include "Helper.h"
#include "gpbox_utils.h"

namespace fs            = boost::filesystem;
namespace jobid         = glite::wmsutils::jobid;
namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;
namespace requestad     = glite::wms::jdl;
namespace utilities     = glite::wms::common::utilities;
namespace matchmaking   = glite::wms::matchmaking;
namespace gpbox_utils   = glite::wms::helper::gpbox_utils;

#define edglog(level)   logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

namespace glite {
namespace wms {
namespace helper {
namespace broker {

namespace {

typedef glite::wms::brokerinfo::BrokerInfo<glite::wms::brokerinfo::brokerinfoGlueImpl> BrokerInfo;

std::string const helper_id("BrokerHelper");

//FIXME: should be moved back to ckassad_plugin if/when only this DL is used
//FIXME: for matchmaking.
glite::wms::classad_plugin::classad_plugin_loader init;

helper::HelperImpl* create_helper()
{
  return new Helper;
}

struct Register
{
  Register()
  {
    helper::HelperFactory::instance()->register_helper(helper_id, create_helper);
  }
  ~Register()
  {
    helper::HelperFactory::instance()->unregister_helper(helper_id);
  }
};

Register const r;

std::string const f_output_file_suffix(".rbh");

std::auto_ptr<classad::ClassAd>
f_resolve_simple(classad::ClassAd const& input_ad, std::string const& ce_id)
{
  std::auto_ptr<classad::ClassAd> result;

  static boost::regex  expression( "(.+/[^\\-]+-([^\\-]+))-(.+)" );
  boost::smatch        pieces;
  std::string gcrs, type, name;
  if (boost::regex_match(ce_id, pieces, expression)) {
    gcrs.assign(pieces[1].first, pieces[1].second);
    type.assign(pieces[2].first, pieces[2].second);
    name.assign(pieces[3].first, pieces[3].second);

    result.reset(new classad::ClassAd(input_ad));
    requestad::set_globus_resource_contact_string(*result, gcrs);
    requestad::set_queue_name(*result, name);
    try {
      std::string junk = requestad::get_lrms_type(*result);
    } catch ( glite::wms::jdl::CannotGetAttribute const& e) {
      requestad::set_lrms_type(*result, type);
    }
   
    requestad::set_ce_id(*result, ce_id);

    // TODO catch requestad::CannotSetAttribute
  } else {
    throw helper::InvalidAttributeValue(requestad::JDL::SUBMIT_TO,
                                        ce_id,
                                        "match (.+/[^\\-]+-([^\\-]+))-(.+)",
                                        helper_id);
  }

  return result;
}

std::auto_ptr<classad::ClassAd>
f_resolve_mm(classad::ClassAd const& input_ad)
try {
  std::auto_ptr<classad::ClassAd> result;
  std::auto_ptr<glite::wms::broker::ResourceBrokerImpl> rb_impl;

  std::string vo(requestad::get_virtual_organisation(input_ad));

  boost::scoped_ptr<BrokerInfo> BI(new BrokerInfo);

  bool input_data_exists = false;
  std::vector<std::string> input_data;
  requestad::get_input_data(input_ad, input_data, input_data_exists);
  if (input_data_exists) {
    rb_impl.reset(new glite::wms::broker::RBMaximizeFilesISMImpl(BI.get()));
  } 
  else {
    rb_impl.reset(new glite::wms::broker::RBSimpleISMImpl());
  }

  glite::wms::broker::ResourceBroker rb(rb_impl.release());

  // If fuzzy_rank is true in the request ad we have
  // to use the stochastic selector...
  bool use_fuzzy_rank = false;
  if (requestad::get_fuzzy_rank(input_ad, use_fuzzy_rank) && use_fuzzy_rank) {
    rb.changeSelector("stochasticRankSelector");
  }

  boost::scoped_ptr<matchmaking::match_table_t> suitable_CEs(rb.findSuitableCEs(&input_ad));
  if (suitable_CEs->empty()) {
    throw NoCompatibleCEs();
  }

  std::string dg_jobid_str(requestad::get_edg_jobid(input_ad));
  jobid::JobId dg_jobid(dg_jobid_str);

  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  Info("Start gpbox interaction");
  boost::timer perf_timer;
  perf_timer.restart();

  const configuration::CommonConfiguration* common_conf = config->common();
  assert(common_conf);

  const std::string broker_subject(
    gpbox_utils::get_proxy_distinguished_name(common_conf->host_proxy_file())
  );

  const configuration::WMConfiguration* WM_conf = config->wm();
  assert(WM_conf);

  std::string Pbox_host_name(WM_conf->pbox_host_name());
  if( !broker_subject.empty() and !Pbox_host_name.empty() ) {
    try {

      Info(Pbox_host_name);
      Info(WM_conf->pbox_port_num());
      Info(WM_conf->pbox_safe_mode());

      Connection PEP_connection(
                                Pbox_host_name,
                                WM_conf->pbox_port_num(),
                                broker_subject,
                                WM_conf->pbox_safe_mode()
                               );

      Info("gpbox: connection open");

      if (!gpbox_utils::filter_gpbox_authorizations(*suitable_CEs, 
                                       PEP_connection, 
                                       gpbox_utils::get_user_x509_proxy(dg_jobid))) {
        //TODO: throws proper exception
      }
    }
    catch (...) { // exception no_conn from API 
                  // PEP_connection not properly propagated
      Info("gpbox: no connection!!!");
      // no connection to the Pbox server, the RB goes on 
      // without screening the list of suitable CEs
    }; //try
  }
  else {
    Info("gpbox: unable to find the broker proxy certificate or gpbox host name not specified");
  }

  Info("End gpbox:");
  Info(perf_timer.elapsed());
  if (suitable_CEs->empty()) {
    Info("Empty CE list after G-Pbox screening");
    throw NoCompatibleCEs();
  }
  // End of G-Pbox interaction

  matchmaking::match_const_iterator ce_it = rb.selectBestCE(*suitable_CEs);
  
  // update the brokerinfo
  BI->retrieveCloseSEsInfo(ce_it->first); // CE id
  BI->retrieveCloseSAsInfo(vo); // Retrieve only GlueSAAvailableVOSpace

  configuration::NSConfiguration const* const ns_config = config->ns();
  assert(ns_config);

  fs::path p(ns_config->sandbox_staging_path(), fs::native);

  p /= jobid::get_reduced_part(dg_jobid);
  p /= jobid::to_filename(dg_jobid);

  fs::path BIfile(
                  (p / "input/.BrokerInfo").native_file_string(), 
                   fs::portable_posix_name
                  );
  std::ofstream BIfilestream(BIfile.native_file_string().c_str());

  if (!BIfilestream) {
    throw CannotCreateBrokerinfo(BIfile);
  }

  boost::scoped_ptr<classad::ClassAd> biAd(BI->asClassAd());
  classad::ExprTree const* DACexpr = input_ad.Lookup("DataAccessProtocol");
  if (DACexpr) {
    biAd->Insert("DataAccessProtocol", DACexpr->Copy());
  }
  BIfilestream << *biAd << std::endl;

  if (!BIfilestream) {
    throw CannotCreateBrokerinfo(BIfile);
  }

  result.reset(new classad::ClassAd(input_ad));

  // Add the .Brokerinfo files to the InputSandbox
  bool input_sandbox_exists = false;
  bool wmpinput_sandbox_base_uri_exists = false;

  std::vector<std::string> ISB;

  requestad::get_input_sandbox(input_ad, ISB, input_sandbox_exists);
  std::string WMPInputSandboxBaseURI(requestad::get_wmpinput_sandbox_base_uri(
                                             input_ad, 
                                             wmpinput_sandbox_base_uri_exists)
                                    );

  if (wmpinput_sandbox_base_uri_exists) 
    ISB.push_back(WMPInputSandboxBaseURI+"/input/.BrokerInfo");
  else 
    ISB.push_back(".BrokerInfo");
  
  requestad::set_input_sandbox(*result, ISB);

  requestad::set_ce_id(*result, ce_it->first);
  matchmaking::match_info const& ce_info = ce_it->second;
  classad::ClassAd const* ce_ad = ce_info.getAd();

  try {

    requestad::set_globus_resource_contact_string(
      *result,
      utilities::evaluate_attribute(*ce_ad, "GlobusResourceContactString")
    );
    requestad::set_queue_name(
      *result,
      utilities::evaluate_attribute(*ce_ad, "QueueName")
    );
    requestad::set_lrms_type(
      *result,
      utilities::evaluate_attribute(*ce_ad, "LRMSType")
    );
    requestad::set_ce_id(
      *result,
      utilities::evaluate_attribute(*ce_ad, "CEid")
    );

  } catch (utilities::InvalidValue const& e) {

    edglog(error) << e.what() << " for CE id " << ce_it->first << std::endl;

    throw helper::HelperError("BrokerHelper");

  }

  return result;

} catch (matchmaking::ISNoResultError const& e) {

  throw NoAvailableCEs(
    "The user is not authorized on any resource currently registered in "
    + e.host()
  );

} catch (matchmaking::InformationServiceError const& e) {

  throw NoAvailableCEs(e.what());
  
} catch (matchmaking::RankingError const& e) {
  
  throw NoAvailableCEs(e.what());
	
} catch (requestad::CannotGetAttribute const& e) {

  throw helper::CannotGetAttribute(e, helper_id);

} catch (requestad::CannotSetAttribute const& e) {

  throw helper::CannotSetAttribute(e, helper_id);

} catch( jobid::JobIdException& jide ) {

  edglog( error ) << jide.what() << std::endl;
  throw helper::InvalidAttributeValue(requestad::JDL::JOBID,
                                      "unknown",
                                      "valid jobid",
                                      helper_id);

} catch( fs::filesystem_error& fse ) {

    edglog( error ) << fse.what() << std::endl;
    throw helper::FileSystemError(helper_id, fse);

}

} // {anonymous}

std::string
Helper::id() const
{
  return helper_id;
}

std::string
Helper::output_file_suffix() const
{
  return f_output_file_suffix;
}

classad::ClassAd*
Helper::resolve(classad::ClassAd const* input_ad) const
{
  bool submit_to_exists = false;
  std::string ce_id = requestad::get_submit_to(*input_ad, submit_to_exists);

  std::auto_ptr<classad::ClassAd> result
    = submit_to_exists ? f_resolve_simple(*input_ad, ce_id) 
                       : f_resolve_mm(*input_ad);

  return result.release();
}

}}}} // edg::workload::planning::broker
