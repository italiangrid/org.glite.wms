/***************************************************************************
 *  filename  : JobWrapper.h
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *              Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#ifndef GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H
#define GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif

#ifndef GLTIE_WMS_X_VECTOR
#define GLTIE_WMS_X_VECTOR
#include <vector>
#endif

#ifndef GLITE_WMS_X_IOSTREAM
#define GLITE_WMS_X_IOSTREAM
#include <iostream>
#endif

#ifndef GLITE_WMS_X_UTILITY
#define GLITE_WMS_X_UTILITY
#include <utility>
#endif

#ifndef GLITE_WMS_X_BOOST_SHARED_PTR_HPP
#define GLITE_WMS_X_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif

#ifndef GLITE_WMS_HELPER_JOBADAPTER_URL_URL_H
#include "jobadapter/url/URL.h"
#endif

namespace classad {
class ExprList;
}

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {
	
class JobWrapper
{
  friend std::ostream& operator<<(std::ostream& os, const JobWrapper& jw);

public:
  /**
   *  Default constructor.
   *  Initializes the object passing the parameters to the job wrapper.
   *  \param job
   *  \ingroup jobadapter
   */
  JobWrapper(const std::string& job);

  /**
   *  Default destructor.
   *  \ingroup jobadapter
   */
  virtual ~JobWrapper(void);

public:
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
  void wmp_input_sandbox_support(const std::vector<std::string>& input_base_files);

  /**
   * Declare which files are in the output sandbox and where they have to be moved.
   * @param base_url location the files in the output sandbox have to be moved to
   * @param files    list of files in the output sandbox
   * \ingroup jobadapter
   */
  void wmp_output_sandbox_support(const std::vector<std::string>& output_files,
				  const std::vector<std::string>& output_dest_files);   
   
protected:
  virtual std::ostream& print(std::ostream& os) const;
  
  virtual std::ostream& set_shell(std::ostream& os) const;
  
  virtual std::ostream& set_subdir(std::ostream&      os,
		                   const std::string& dir) const;
  virtual std::ostream& clean_subdir(std::ostream& os) const;
  
  virtual std::ostream& check_workdir(std::ostream& os) const;
  virtual std::ostream& set_workdir(std::ostream& os) const;
  
  virtual std::ostream& set_environment(std::ostream&                   os,
		  		        const std::vector<std::string>& env) const;
  virtual std::ostream& set_environment(std::ostream&      os,
			                const std::string& variable,
					const std::string& value) const;
  virtual std::ostream& check_set_environment(std::ostream& os,
                                        const std::string&  variable,
                                        const std::string&  value) const;

  virtual std::ostream& set_umask(std::ostream& os) const;
  
  virtual std::ostream& check_file_mod(std::ostream&      os,
		                       const std::string& file) const;
  
  virtual std::ostream& set_lb_sequence_code(std::ostream&      os,
  	                                     const std::string& event,
					     const std::string& reason,
					     const std::string& scode,
					     const std::string& ecode) const;
  virtual std::ostream& check_globus(std::ostream& os) const;

  virtual std::ostream& prepare_transfer(std::ostream&                   os,
			                 const std::vector<std::string>& file) const;
  virtual std::ostream& make_transfer(std::ostream&   os,
		  		      const url::URL& prefix,
				      const bool&     input) const;
  virtual std::ostream& make_transfer_wmp_support(std::ostream&   os,
		                                  const bool&     input) const;

  virtual std::ostream& execute_job(std::ostream&      os,
			            const std::string& arguments,
			            const std::string& job,
			            const std::string& stdi,
			            const std::string& stdo,
			            const std::string& stde,
				    int                node) const;

  virtual std::ostream& make_bypass_transfer(std::ostream& os,
		  			     const std::string& prefix) const;

  virtual std::ostream& create_maradona_file(std::ostream& os,
		  			     const std::string& jobid_to_filename) const;

  virtual std::ostream& send_dgas_gianduia_lsf_files(std::ostream& os,
				  const std::string& globus_resource_contact_string,
                                  const std::string& gatekeeper) const;

  virtual std::ostream& send_dgas_gianduia_pbs_files(std::ostream& os,
  			          const std::string& globus_resource_contact_string,
                                  const std::string& gatekeeper) const;

  virtual std::ostream& doExit(std::ostream& os,
		  	       const std::string& maradonaprotocol,
			       bool create_subdir) const;

  virtual std::ostream& handle_output_data(std::ostream& os) const;

  virtual std::ostream& doDSUploadTmp(std::ostream& os) const;
  virtual std::ostream& doDSUpload(std::ostream& os) const;
  virtual std::ostream& doCheckReplicaFile(std::ostream& os) const;
  virtual std::ostream& doReplicaFile(std::ostream& os) const;
  virtual std::ostream& doReplicaFilewithLFN(std::ostream& os) const;
  virtual std::ostream& doReplicaFilewithSE(std::ostream& os) const;
  virtual std::ostream& doReplicaFilewithLFNAndSE(std::ostream& os) const;
	
protected:
  // executable, stdin, stdout, stderr, and arguments
  std::string              m_job;
  std::string              m_standard_input;
  std::string              m_standard_output;
  std::string              m_standard_error;
  std::string              m_arguments;
  std::string              m_maradonaprotocol;
  
  // input sandbox
  url::URL                 m_input_base_url;
  std::vector<std::string> m_input_files;

  // output sandbox
  url::URL                 m_output_base_url;
  std::vector<std::string> m_output_files;

  // brokerinfo file
  std::string              m_brokerinfo;

  // create subdirectory?
  bool                     m_create_subdir;

  // job id
  std::string              m_jobid;

  // job id file name
  std::string              m_jobid_to_filename;
  
  // gatekeeper hostname
  std::string              m_gatekeeper_hostname;

  // environment variable
  std::vector<std::string> m_environment;

  // node number
  int                      m_nodes;

  // List of OutputData
  boost::shared_ptr<classad::ExprList> m_outputdata;
  
  std::string              m_vo;

  std::string              m_dsupload;

  bool			   m_wmp_support;
  std::vector<std::string> m_wmp_input_files;
  std::vector<std::string> m_wmp_input_base_files;
  std::vector<std::string> m_wmp_output_files;
  std::vector<std::string> m_wmp_output_dest_files;
  
protected:
  // default value of the brokerinfo file
  static const std::string s_brokerinfo_default;
};

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_JOBADAPTER_JOBWRAPPER_JOB_WRAPPER_H
