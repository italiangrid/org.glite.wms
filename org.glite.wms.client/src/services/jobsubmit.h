#ifndef GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H
#define GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

// inheritance
#include "job.h"
// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
#include "utilities/logman.h"
//listener for interactive jobs
#include "listener.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// Ad's
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/collectionad.h"

namespace glite {
namespace wms{
namespace client {
namespace services {

/**
* Type of Jobs
*/
enum wmsJobType {
	WMS_JOB, //normal
        WMS_DAG,
        WMS_COLLECTION,
        WMS_PARAMETRIC
};

class JobSubmit : public Job {
	public :
		/*
		*	Default constructor
		*/
		JobSubmit ( );
                /*
		*	Default destructor
		*/
		~JobSubmit ( );
		/*
		*	Processes the command-line user arguments
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	Performs the operations to submit the job(s)
		*/
		void submission ( ) ;
	private:
		/**
                 *	Contacts the server in order to retrieve the list of all destionationURI's of the job (with the available protocols).
		 *  	In case of compound jobs (DAG, collections etc..), it also retrieves the URIs of the nodes.
		 * 	The request is done using the JobId. Compound jobs are identified by the parent's JobId
                 *	@param jobid the string with the JobId
		 *	@param zipURI this parameter returns the DestinationURI associated to the default protocol for ZIP files (according to the value of Options::DESTURI_ZIP_PROTO)
                 *	@return a pointer to the string with the destinationURI having the protocol that will be used for any file transferring
		 *	(or NULL in case the destinationURI with the chosen protocol is not available )
                */
                std::string* getBulkDestURI(const std::string &jobid, const std::string &child, std::string &zipURI) ;


std::string* getSbDestURI(const std::string &jobid, const std::string &child, std::string &zipURI);
		/**
		* Retrieves from the user all the local files of the job input sandbox that have been referenced in the JDL describing
		* a normal job.
		* @param paths returns the list of local files found in the JDL InputSandbox
		*/
		void jobISBFiles(vector<std::string> &paths);
		/**
		* Retrieves from the user all the local files of the job input sandbox that have been referenced in the JDL describing
		* a collection.
		* @param paths returns the list of local files of the job input sandbox that have been referenced in the JDL
		*/
		void collectionISBFiles(vector<std::string> &paths);
		/**
		* Retrieves from the user all the local files of the job input sandbox that have been referenced in the JDL.
		* This method is executed when jobs are represented as DAG
		* @param paths returns the list of local files of the job input sandbox that have been referenced in the JDL
		*/
		void dagISBFiles(vector<std::string> &paths, const bool &children=true);
		/**
		* Retrieves the list of local files that have been referenced in the JDL calling one of the xxxxxISBFiles( ) methods
		* (according to the type of the job) and gets back the total size (in bytes) of these files.
		* @param jobtype the type of job described in the JDL (according to the tags defined in the wmsJobType enum type)
		*/
		int getInputSandboxSize(const wmsJobType &jobtype);
		/**
		* Checks if the total size of the local files, that have been referenced in the JDL,  is compatible with the limitation that
		* can be set on the server :
		* 	- the first limitation is the available User Free Quota (the remaining free part of available user disk quota)
		* 	- the second one is the max InputSanbox size
		* The max InputSanbox size is checked only if theUser Free Quota has been set. The submission operations go on
		* if eitheir the total size of the file doesn't exceed the fixed limitation or no limitation has been set on the server.
		* This method takes care of contacting the server to retrieve the information on these limitations.
		* (according to the type of the job) and gets back the total size (in bytes) of these files.
		* @param jobtype the type of job described in the JDL (according to the tags defined in the wmsJobType enum type)
		*/
		void checkInputSandboxSize (const wmsJobType &jobtype) ;
		/**
		* Returns a string with the InputSandbox URI information eventually set in JDL by user for
		* one of the nodes in a DAG. Empty string is returned if no InputSandbox URI has been referenced in the JDL for the specified node.
		* Children nodes is indicated by its name specified in the input string parameter. Providing no input parameter, the
		* method looks for the InputSandbox URI of the parent node
		* @param the string with the name of the child node (default value is "empty string"= parent node)
		* @return the string representing the URI (empty string if no URI is defined for the specified node)
		*/
		std::string JobSubmit::getDagISBURI (const std::string &node="");
		/*
                * Performs either registration or submission.
		* The WMProxy submission service, that performs both registration and start of the job, is invoked
		* if no local files are referenced in the JDL InputSandbox. If there are files that needed
		* to be transferred to the server, only the jobRegistration is performed.
		* After the file transferring, the jobStarter method will be called.
		* @param submit perform submission (true) or registration (false)
		* @return a string contained the identifier with which the job has been registered
                */
		std::string jobRegOrSub(const bool &submit);

		/*
		* Performs job starting
		* @param jobid the identifier of the job to be started
		*/
		void jobStarter(const std::string &jobid);
		/*
                *	Reads the list of input pathnames  and matches them with the local files, resolving evantually wildcards,
                *	 and the remote destinationURI's; gets back a list of pair: local file, remote destinationURI's where to transfer the local file.
		*  	For compound jobs providing the JobId of a node ("child" input parameter), the InputSandbox of this node in the JDL is processed (if it is specified).
                *	@param jobid a string with the JobId (the parent's jobid in case of compound jobs)
		*	@param child a string with the JobId of the child node (default value is "empty string")
                *	@param paths input pathnames to be checked
                *	@param to_bcopied at the end of the execution it returns a vector containing the resulting list of pairs (it is empty if no file has to be transferred)
		*	@return the pointer to the destination URI string to be used for the file transferring
                */
                std::string* toBCopiedFileList(const std::string &jobid, const std::string &child, const std::string &isb_uri, const std::vector <std::string> &paths, std::vector <std::pair<std::string, std::string> > &to_bcopied);

		std::string* getInputSbDestinationURI(const std::string &jobid, const std::string &child, std::string &zipURI ) ;
		/*
		* Collects a group of files into one or more tar files which are gzip compressed.
		* The number of archives depends on the size limit of tar archives.
		* @param to_bcopied the list of files to be archived that will be copied to the WMProxy server
		* @param destURI the destinationURI of the job where the gzip file has to be transferred
		 */
		void createZipFile (std::vector <std::pair<std::string, std::string> > &to_bcopied, const std::string &destURI) ;
		                /*
		* 	Performs the transferring a set of local files to one(more) remote machine(s) by globus-url-copy (gsiftp protocol)
 		*	@param paths list of files to be transferred (each pair is <source,destination>)
                *	@throw WmsClientException if any error occurs during the operations
                *	(the local file doesn't exists, defective credential, errors on remote machine)
		*/
		void gsiFtpTransfer(std::vector<std::pair<std::string,std::string> > &paths);
                /*
                * 	Performs the transferring a set of local files to one(more) remote machine(s) by curl (https protocol)
                *	@param paths list of files to be transferred (each pair is <source,destination>)
                *	@throw WmsClientException if any error occurs during the operations
                *	(the local file doesn't exists, defective credential, errors on remote machine)
                */
                void curlTransfer (std::vector<std::pair<std::string,std::string> > paths);
		/*
		*	Gets the list of the InputSandbox files to be transferred to the DestinationURI's
		*	@param paths the list of files that still need to be transferred
		*	@param jobid the identifier of the job
		*	@return  the message with the list of the files in the input vector 'paths'
		*/
		std::string JobSubmit::transferFilesList(std::vector<std::pair<std::string,std::string> > &paths, const std::string& jobid) ;
		/**
		* Uploads the local files in the InputSandbox to the WMProxy server. If file compression is allowed, all files are collected
		* in one or more tar file archives which are gzip compressed. The number of archives depends on the size limit of
		* tar archives. If compression is not allowed, each file is separately uploaded in succession.
		* The transferring is performed using either the defined default protocol (Options::TRANSFER_FILES_DEF_PROTO)
		* or the protocol chosen by the --proto option.
		* On the base of this choice, the specific "xxxxxTransfer" function is called.
		* @paths a vector containing the list of local files and for each file the relative destinationURI information
		* (to be used in case of single file transferring or as relative path in the creation of the tar archive)
		* @destURI a string with the destinationURI where to transfer the tar archives
		*/
		 void transferFiles(std::vector<std::pair<std::string,std::string> > &paths, const std::string &destURI, const std::string &jobid);
		/**
		*  Reads the results of the registration operation for a job represented as a DAG  from the JobIdApi structure.
		* This method is only executed if the JDL InputSandbox contains local files that need
		* to be transferred to the WMProxy server.
		* The transferring of the files is performed if this operation has been selected.
		* In case of  it has not been requested (and the operation ends with the only registration),
		* an info message with the list these file is provided.
		*/
                std::string normalJob();
		/**
		*  Reads the results of the registration operation for a job represented as a DAG  from the JobIdApi structure.
		* This method is only executed if the JDL InputSandbox contains local files that need
		* to be transferred to the WMProxy server.
		* The transferring of the files is performed if this operation has been selected.
		* In case of  it has not been requested (and the operation ends with the only registration),
		* an info message with the list these file is provided.
		*/
                std::string dagJob( );
		/**
		*  Reads the results of the registration operation for a collection from the JobIdApi structure.
		* This method is only executed if the JDL InputSandbox contains local files that need
		* to be transferred to the WMProxy server.
		* The transferring of the files is performed if this operation has been selected.
		* In case of  it has not been requested (and the operation ends with the only registration),
		* an info message with the list these file is provided.
		*/
                std::string collectionJob();
		/**
                * Checks the user JDL
		*@param filestoBtransferred whether the ad has any file to be transferred (true) or not(false)
		*@param jobtype returns the code of the JobType (see wmsJobType enum )
                */
                void JobSubmit::checkAd(bool &filestoBtransferred, wmsJobType &jobtype);
		/**
		* Determines whether the files of the InputSandbox can be collected into a tar archive and compressed.
		* This method checks the JDL whether the user has disabled the file archiving and compression setting
		* the "ZIP_ALLOWED" attribute to FALSE.
		* In case this attribute is set to TRUE or not present, it checks the version of the WMProxy server
		* in order to estabilish whether it supports this kind of features.
		*
		*/
		void JobSubmit::checkZipAllowed(const wmsJobType &jobtype) ;
		/**
                *	String input arguments
                */
		std::string* chkptOpt ;
		std::string* collectOpt;
		std::string* dgOpt ;
		std::string* fileProto;
		std::string* lrmsOpt ;
		std::string* toOpt ;
		std::string* inOpt ;
		std::string* resourceOpt ;
		std::string* validOpt ;
		std::string* startOpt ;
		/**
                *	Boolean input arguments
                */
		bool nomsgOpt ;
		bool nolistenOpt ;
                bool registerOnly;
		bool startJob;
		/**
		* Final Warning Message
		*/
		std::string infoMsg;
		/**
		* Time attributes
		*/
		long expireTime ;
                /**
                * JobId's
                */
                glite::wms::wmproxyapi::JobIdApi jobIds ;
		/**
                *	Ad-objects
		*/
                glite::wms::jdl::Ad *jobAd ;
		glite::wms::jdl::ExpDagAd *dagAd  ;
        	glite::wms::jdl::CollectionAd *collectAd ;
		/**
                * JobShadow for interactive jobs
                */
                Shadow *jobShadow ;
		/**
		*	path to the JDL file
		*/
		std::string *jdlFile ;
		/**
		*	string of the user JDL
		*/
		std::string *jdlString ;

int isbSize ;
std::string isbURI;
std::vector< std::pair<std::string ,std::string > > dagISBURIs;

		/**
		* The name of the ISB tar/zip files
		*/
std::vector<std::string> gzFiles;
		/**
		*
		*/
		bool zipAllowed ;
		/**
		* List of Destination URI's
		*/
		std::vector< std::pair<std::string ,std::vector<std::string > > > dsURIs ;
		/*
		* Major Version number of the server
		*/
		int wmpVersion;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

