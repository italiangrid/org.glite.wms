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
		*	Reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /*
		*	Performs the operations to submit the job(s)
		*/
		void submission ( ) ;
	private
                :/*
                 *	Contacts the endpoint configurated in the context
                 *	in order to retrieve the http(s) destionationURI of the job
                 *	identified by the jobid
                 *	@param jobid the identifier of the job
		 *	@param zipURI this parameter returns the DestinationURI associated to the default protocol for ZIP files (according to the value of Options::DESTURI_ZIP_PROTO)
                 *	@return a pointer to the string with the http(s) destinationURI (or NULL in case of error)
                */
                std::string* getInputSBDestURI(const std::string &jobid,const std::string &child, std::string &zipURI) ;

                /*
                *	Checks if the UserFreeQuota is set for the user on the EndPoint; if it is, checks that
                *	the size of the InputSB file to be transferred doesn't exceed this quota
                *	@param files the set of files to be transferred
                *	@param limit this parameter returns the value of the user free quota
                *	@return true if the user freequota is enough or no free quota has been set on the server (false, otherwise)
                */
                //bool checkFreeQuota ( std::vector<std::pair<std::string,std::string> > files, long &limit ) ;
void jobISBFiles(vector<std::string> &paths);
void collectionISBFiles(vector<std::string> &paths);
void dagISBFiles(vector<std::string> &paths, const bool &children=true);
int getInputSandboxSize(const wmsJobType &jobtype);
void checkInputSandboxSize (const wmsJobType &jobtype) ;
std::string JobSubmit::getDagISBURI (const std::string &node="");
		/*
                * Performs either registration or submission of the job
		*@param submit perform submission (true) or registration (false)
                */
		std::string jobRegOrSub(bool submit);

		/*
		* Performs job starting
		* @param jobid the identifier of the job to be started
		*/
		void jobStarter(const std::string &jobid);



		/*
                *	Reads the list of input pathnames for the job ( identified by the input jobid) and matches them with the local files, resolving evantually wildcards,
                *	 and the remote destinationURI's; gets back a list of pair: local file, remote destinationURI's where to transfer the local file.
                *	@param jobid the identifier of the job
                *	@param paths input pathnames to be checked
                *	@param to_bcopied at the end of the execution it returns a vector containing the resulting list of pairs (it is empty if no file has to be transferred)
		*	@return the pointer to the destination URI string to be used for the file transferring
                */
                std::string* toBCopiedFileList(const std::string &jobid,const std::string &child,const std::string &isb_uri, const std::vector <std::string> &paths, std::vector <std::pair<std::string, std::string> > &to_bcopied);
		/*
		* Archives a group of files into a tar file and creates and compresses the archive.
		* These files represent the InputSandbox of the job to be copied to the WMProxy server.
		* They are archived creating a filesystem tree based on the destinationURI path of the job provided as input.
		* @param to_bcopied the list of files to be archived that will be copied to the WMProxy server
		* @param destURI the destinationURI of the job where the gzip file has to be transferred
		*
		 */

void createZipFile (std::vector <std::pair<std::string, std::string> > &to_bcopied, const std::string &destURI) ;
		                /*
		* 	Performs the transferring a set of local files to one(more) remote machine(s) by gsiftp
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
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

