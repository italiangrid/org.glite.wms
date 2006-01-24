/***************************************************************************
 *  filename  : JobWrapper.h
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *              Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#ifndef GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H
#define GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H

#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <boost/shared_ptr.hpp>
#include "jobadapter/url/URL.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

namespace classad {
class ExprList;
}

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {
	
enum job_type {NORMAL = 0, MPI_LSF = 1, MPI_PBS = 2, INTERACTIVE = 3};

class JobWrapper
{
public:
  /**
   *  Initializes the object passing the parameters to the job wrapper.
   *  \param job
   *  \ingroup jobadapter
   */
  JobWrapper(const std::string& job);

  virtual ~JobWrapper();

  /**
   * Declare the standard input file.
   * @param file file to be used as standard input for the user job
   * \ingroup jobadapter
   */
  void standard_input(const std::string& file);

  /**
   * Declare the standard output file.
   * @param file file to be used as standard output of the user job
   * \ingroup jobadapter
   */
  void standard_output(const std::string& file);

  /**
   * Declare the standard error file.
   * @param file file to be used as standard error of the user job
   * \ingroup jobadapter
   */
  void standard_error(const std::string& file);

  /**
   * Declare which files are in the input sandbox and where they are located.
   * @param base_url location of all the files in the input sandbox
   * @param files    list of files in the input sandbox
   * \ingroup jobadapter
   */
  void input_sandbox(const url::URL& base_url, 
                     const std::vector<std::string>& files);
  
  /**
   * Declare which files are in the output sandbox and where they have to be moved.
   * @param base_url location the files in the output sandbox have to be moved to
   * @param files    list of files in the output sandbox
   * \ingroup jobadapter
   */
  void output_sandbox(const url::URL& base_url,
                      const std::vector<std::string>& files);

  /**
   * Set the value of the EDG_WL_RB_BROKERINFO environment variable.
   * If the function is not called the variable is not set.
   * @param file file containing Broker information in the job working
   *             directory. If file is not specified a default value is used.
   * \ingroup jobadapter
   */
  void brokerinfo(const std::string& file = s_brokerinfo_default);

  /**
   * Declare that the job should run in a subdirectory.
   * \ingroup jobadapter
   */
  void create_subdir(void);

  /**
  * Set the value of the EDG_WL_JOBID environment variable.
  * If the function is not called the variable is not set.
  * @param jobid
  * \ingroup jobadapter
  */
  void job_Id(const std::string& jobid);
  
  /**
   * Set the value of the workdir.
   * @param jobid
   * \ingroup jobadapter
   */
  void job_id_to_filename(const std::string& jobid_to_filename);
  
  /**
  * Declare arguments
  * @param args
  * \ingroup jobadapter
  */
  void arguments(const std::string& args);

  /*
  * Declare maradonaprotocol
  * @param maradonaprotocol
  * \ingroup maradonaprotocol
  */
  void maradonaprotocol(const std::string& protocol, 
		        const std::string& filename);

  /**
   * Declare gatekeeper_hostname
   * @param gatekeeper
   * \ingroup jobadapter
   */
  void gatekeeper_hostname(const std::string& scode);

  /**
   * Declare globus_resource_contact_string
   * @param globus_resource_contact_string
   * \ingroup jobadapter
   */
  void globus_resource_contact_string(
                  const std::string& globus_resource_contact_string);

  /**
   * Declare which environment variables must be exported.
   * @param env    list of the environment variables
   * \ingroup jobadapter
   */
  void environment(const std::vector<std::string>& env);

  void nodes(int node);

   /**
    * Field the outputdata attribute.
    * @param output_data    list of classads
    * \ingroup jobadapter
    */
   void outputdata(const classad::ExprList* output_data);

   /**
    * Declare virtual organization
    * @param vo 
    * \ingroup jobadapter
    */ 
   void vo(const std::string& vo);

   /**
    * Set the name of the DSUpload output file.
    * @param jobid
    * \ingroup jobadapter
    */
   void dsupload(const url::URL& id);

   /**
    * Set the support of Input/Output Sandboxes in WMProxy.
    * @param wm
    * \ingroup jobadapter
    */
   void wmp_support(void);

   /**
   * Declare which files are in the input sandbox and where they are located.
   * @param base_url location of all the files in the input sandbox
   * @param files    list of files in the input sandbox
   * \ingroup jobadapter
   */
  void wmp_input_sandbox_support(const url::URL& base_url,
                                  const std::vector<std::string>& input_base_files);

  /**
   * Declare which files are in the output sandbox and where they have to be moved.
   * @param base_url location the files in the output sandbox have to be moved to
   * @param files    list of files in the output sandbox
   * \ingroup jobadapter
   */
  void wmp_output_sandbox_support(const std::vector<std::string>& output_files,
				 const std::vector<std::string>& output_dest_files);   
   
  /**
   * Declare the token file.
   * @param base_url
   * @param file
   * \ingroup token
   */
  void token(std::string const& token_file);

  /**
    * Set the support of Perusal.
    * \ingroup jobadapter
    */

  void perusal_support(void);
  void perusal_timeinterval(int perusaltimeinterval);
  void perusal_filesdesturi(std::string const& filesdesturi);
  void perusal_listfileuri(std::string const& listfileuri);

  void set_job_type(int);
  void set_osb_wildcards_support(bool);

private:

  virtual std::ostream& print(std::ostream& os) const;  
  bool fill_out_script(std::string const&, std::ostream&) const;
  bool dump_vars(std::ostream&) const;

  std::string              m_job;
  std::string              m_standard_input;
  std::string              m_standard_output;
  std::string              m_standard_error;
  std::string              m_arguments;
  std::string              m_maradonaprotocol;

  url::URL                 m_input_base_url;
  std::vector<std::string> m_input_files;

  url::URL                 m_output_base_url;
  std::vector<std::string> m_output_files;

  std::string              m_brokerinfo;
  bool                     m_create_subdir;
  std::string              m_jobid;
  std::string              m_jobid_to_filename;
  std::string              m_gatekeeper_hostname;
  std::string              m_globus_resource_contact_string;
  std::vector<std::string> m_environment;
  int                      m_nodes; // nodes for parallel jobs
  boost::shared_ptr<classad::ExprList> m_outputdata;
  std::string              m_vo;
  std::string              m_dsupload;

  bool	                   m_wmp_support;
  std::vector<std::string> m_wmp_input_files;
  std::vector<std::string> m_wmp_input_base_files;
  std::vector<std::string> m_wmp_output_files;
  std::vector<std::string> m_wmp_output_dest_files;
  
  std::string              m_token_file;
  bool                     m_token_support;

  bool                     m_perusal_support;
  int                      m_perusal_timeinterval;
  std::string              m_perusal_filesdesturi;
  std::string              m_perusal_listfileuri;
  
  int                      m_job_type;
  bool                     m_osb_wildcards_support;

  static const std::string s_brokerinfo_default;

  friend std::ostream& operator<<(std::ostream& os, const JobWrapper& jw);
};

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H
