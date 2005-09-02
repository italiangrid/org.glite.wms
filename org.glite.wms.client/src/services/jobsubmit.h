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
		 *	@param
                 *	@return a pointer to the string with the http(s) destinationURI (or NULL in case of error)
                */
                std::string* getInputSBDestURI(const std::string &jobid, const std::string &child) ;

                /*
                *	Checks if the UserFreeQuota is set for the user on the EndPoint; if it is, checks that
                *	the size of the InputSB file to be transferred doesn't exceed this quota
                *	@param files the set of files to be transferred
                *	@param limit this parameter returns the value of the user free quota
                *	@return true if the user freequota is enough or no free quota has been set on the server (false, otherwise)
                */
                bool checkFreeQuota ( std::vector<std::pair<std::string,std::string> > files, long &limit ) ;
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
                *	reads the list of input pathnames for the job ( identified by the input jobid) and matches them with the local files, resolving evantually wildcards,
                *	 and the remote destinationURI's; gets back a list of pair: local file, remote destinationURI's where to transfer the local file.
                *	@param jobid the identifier of the job
                *	@param paths input pathnames to be checked
                *	@param to_bcopied the vector containing the resulting list of pairs (it is empty if no file has to be transferred)
                */
                void toBCopiedFileList(const std::string &jobid, const std::string &child,const std::string &isb_uri, const std::vector <std::string> &paths, std::vector <std::pair<std::string, std::string> > &to_bcopied);
		/*
		*	Gets the list of the InputSandbox files to be transferred to the DestinationURI's
		*	@param paths the list of files that still need to be transferred
		*	@return the string message
		*/
		std::string JobSubmit::transferFilesList(std::vector<std::pair<std::string,std::string> > &paths, const std::string& jobid) ;
                /*
                *	looks for the node name in the list (nodename; vector of list of files) and returns the associated list of files
                *	@param node the node index (the name of the node to be looked for in the list)
                *	@param list vector (index=nodename; vector of files); the vector of files contains a list of pathnames
                *	@return the list of files of the input node
                */
                std::vector<std::string> getNodeFileList (const std::string &node, std::vector< std::pair<std::string ,std::vector<std::string > > > &list);
                /*
                * 	reads the JobRegister results for a normal job and performs the transferring of the
                *  	local InputSandbox files (called by the main method: submission)
                */
                std::string normalJob( );
                /*
                * 	reads the JobRegister results for a DAG  job and performs the transferring of the
                *  	local InputSandbox files (called by the main method: submission)
                */
                std::string dagJob( );
                /*
                * 	reads the JobRegister results for a collection of jobs  and performs the transferring of the
                *  	local InputSandbox files (called by the main method: submission)
                */
                std::string collectionJob( );
		/*
                * reads and checks the JDL file of which pathname is  specified in the jdlFile attribute of this class
                *@param jobtype the code of the JobType (see wmsJobType enum)
		*@param filestoBtransferred whether the ad has any file to be transferred (true) or not(false)
                */
                void JobSubmit::checkAd(bool &filestoBtransferred, wmsJobType &jobtype);

		/*
                *	string input arguments
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
		/*
                *	boolean input arguments
                */
		bool nomsgOpt ;
		bool nolistenOpt ;

                bool registerOnly;
		bool startJob;
		std::string transferMsg;

		/*
		* Time attributes
		*/
		long expireTime ;
                /*
                * JobId's
                */
                glite::wms::wmproxyapi::JobIdApi jobIds ;
		/*
                *	Ad-objects
		*/
                glite::wms::jdl::Ad *jobAd ;
		glite::wms::jdl::ExpDagAd *dagAd  ;
        	glite::wms::jdl::CollectionAd *collectAd ;
		/*
                * jobShadow for interactive jobs
                */
                Shadow *jobShadow ;
		/*
		*	path to the JDL file
		*/
		std::string *jdlFile ;
		/*
		*	string of the user JDL
		*/
		std::string *jdlString ;
		/**
		* List of Destination URI's
		*/
		std::vector< std::pair<std::string ,std::vector<std::string > > > dsURIs ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_JOBSUBMIT_H

