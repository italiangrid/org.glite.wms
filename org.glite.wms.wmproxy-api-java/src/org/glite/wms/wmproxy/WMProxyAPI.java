/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy;


import java.net.URL;
import java.io.File ;
import java.io.FileNotFoundException ;

/*
import javax.net.ssl.HttpsURLConnection;
import org.glite.security.trustmanager.ContextWrapper ;
*/

import org.glite.security.delegation.DelegationHandler ;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 *
 *
 *
 * @ingroup
 * @brief
 * @version
 * @date
 * @author Marco Sottilaro <marco.sottilaro@datamat.it>
 *
 */


public class WMProxyAPI{

	/**
	*
	*  @param url WMProxy service URL
	*  @param proxyFile the path location of the user proxy file
	*  @throws  .ServiceException If any error in calling the service
	*  @throws MalformedURLException malformed service URL specified as input
	*  @throws  FileNotFoundException Unable to find the user proxy file
	*
	*/

	public WMProxyAPI (String url, String proxyFile) throws javax.xml.rpc.ServiceException,
					java.net.MalformedURLException, java.io.FileNotFoundException {

		logger = Logger.getLogger(WMProxyAPI.class);
		logger.debug ("url=[" + url + "] - proxyFile = [" + proxyFile + "]");

		// service URL
		this.serviceURL = new URL (url);

		// proxyFile
		this.proxyFile = proxyFile;

		// Sets up the connection
		this.setUpService ( );

	}

	/**
	*
	*  @param url WMProxy service URL
	*  @param proxyFile the path location of the user proxy file
	*  @param logPropFille the path location of the log4j properties file
	*  @throws ServiceException If any error occurs in calling the service
	*  @throws MalformedURLException malformed service URL specified as input
	*  @throws FileNotFoundException Unable to find the user proxy file
	*
	*/

	public WMProxyAPI (String url, String proxyFile, String logPropFile) throws javax.xml.rpc.ServiceException,
					java.net.MalformedURLException, java.io.FileNotFoundException {

		// logger
		if ( logPropFile != null)
			PropertyConfigurator.configure(logPropFile);
	/*TBR !!!!!!!!!!!*/
		else
			PropertyConfigurator.configure("/home/msotty/Develop/org.glite.wms.wmproxy-api-java/config/log4j.properties");
	/*TBR !!!!!!!!!!!*/

		logger = Logger.getLogger(WMProxyAPI.class);
		logger.debug ("INPUT: url=[" + url + "] - proxyFile = [" + proxyFile + "]");

		// service URL
		this.serviceURL = new URL (url);

		// proxyFile
		this.proxyFile = proxyFile;

		// Sets up the connection
		this.setUpService ( );

	}

	/**
	*
	*  @param delegationID the id to identify the delegation
	*  @return a certReq string (Service certificate request)
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*
	*/

	public java.lang.String getProxyReq (java.lang.String delegationId) throws java.rmi.RemoteException {

			logger.debug ("INPUT: delegationId=[" +delegationId + "]" );
			return this.serviceStub.getProxyReq(delegationId) ;

	}

	/**
	*
	*  @param certReq Service certificate request (@see #getProxyReq)
	*  @param delegationID the id used to identify the delegation (@see #getProxyReq)
	*  @param propFile  the path location of the properties file (????)
	*  @return the generated proxy certificate
	*  @throws Exception If any error occurs during the creation of the proxy
	*
	*/

	public String createProxyfromCertReq(java.lang.String certReq, java.lang.String delegationId, java.lang.String propFile) throws  java.lang.Exception {

		logger.debug ("INPUT: certReq=[" + certReq + "]");
		logger.debug ("INPUT: delegationId=[" +delegationId + "] - propFile " + propFile + "]" );

		// Generating a proxy certificate
		DelegationHandler dlgHandler = new DelegationHandler(certReq,delegationId, propFile);

		// Gets the certificate
		return dlgHandler.getPEMProxyCertificate();

	}

	/**
	*
	*  @param delegationId the id used to identify the delegation (@see #getProxyReq)
	*  @param proxy the user  proxy
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*
	*/
	public void putProxy(java.lang.String delegationId, java.lang.String proxy) throws java.rmi.RemoteException {

		logger.debug ("INPUT: proxy=[" + proxy + "]");
		logger.debug ("INPUT: delegationId=[" +delegationId + "]");

		this.serviceStub.putProxy(delegationId, proxy) ;

	}

	/**
	*
	* @return the version numbers
	* @throws RemoteException If any error occurs during the execution of the remote method call
	* @throws GenericFaultType
	* @throws AuthenticationFaultType
	*
	*/

	public java.lang.String getVersion()
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType {

		return this.serviceStub.getVersion() ;

	}

	/**
	*
	*  @param jdl the job description
	*  @param delegationId the id used to identify the delegation (@see #getProxyReq)
	*  @return the structure containing the id of the registered job
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthenticationFaultType
	*  @throws InvalidArgumentFaultType
	*
	*/
	public org.glite.wms.wmproxy.JobIdStructType jobRegister(java.lang.String jdl, java.lang.String delegationId)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType {

		logger.debug ("INPUT: JDL=[" + jdl + "]");
		logger.debug ("INPUT: delegationId=[" +delegationId + "]");

		return this.serviceStub.jobRegister (jdl, delegationId) ;
	}

	/**
	*
	*  @param jobId the id of the job
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthenticationFaultType
	*  @throws OperationNotAllowedFaultType
	*  @throws InvalidArgumentFaultType
	*  @throws JobUnknownFaultType
	*
	*/
	public void jobStart(java.lang.String jobId)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType,
				org.glite.wms.wmproxy.OperationNotAllowedFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType,
				org.glite.wms.wmproxy.JobUnknownFaultType {

		logger.debug ("INPUT: jobid=[" + jobId + "]");
		this.serviceStub.jobStart ( jobId ) ;
	}

	/**
	*
	*  @param jobId the id of the job
	*  @param return the id of the job  (@see #org.glite.wms.wmproxy.JobIdStructType)
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws OperationNotAllowedFaultType
	*  @throws InvalidArgumentFaultType
	*  @throws JobUnknownFaultType
	*
	*/
	public org.glite.wms.wmproxy.JobIdStructType jobSubmit(java.lang.String jdl, java.lang.String delegationId)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType {

			logger.debug ("INPUT: JDL=[" + jdl + "]\n");
			logger.debug ("INPUT: delegationId=[" +delegationId + "]");

			return this.serviceStub.jobSubmit(jdl, delegationId) ;
	}

	/**
	*
	*  @param jobId the id of the job
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws OperationNotAllowedFaultType
	*  @throws InvalidArgumentFaultType
	*  @throws JobUnknownFaultType
	*
	*/
	public void jobCancel(java.lang.String jobId)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType,
				org.glite.wms.wmproxy.OperationNotAllowedFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType,
				org.glite.wms.wmproxy.JobUnknownFaultType {

		logger.debug ("INPUT: jobid=[" + jobId + "]");
		this.serviceStub.jobCancel(jobId);

	}

	/**
	*
	*  @return the size of the InputSandbox
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthenticationFaultType
	*
	*/
	public double getMaxInputSandboxSize()
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType {

		return this.serviceStub.getMaxInputSandboxSize();

		//return -3;  TBR !!!!
	}

	/**
	*
	*  @param jobId the id of the job
	*  @return the Sandbox destionation URI
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws OperationNotAllowedFaultType
	*  @throws InvalidArgumentFaultType
	*  @throws JobUnknownFaultType
	*
	*/
	public java.lang.String getSandboxDestURI(java.lang.String jobId)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType,
				org.glite.wms.wmproxy.OperationNotAllowedFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType,
				org.glite.wms.wmproxy.JobUnknownFaultType {

		logger.debug ("INPUT: jobid=[" + jobId + "]");

		return this.serviceStub.getSandboxDestURI(jobId);
	}

	/**
	*
	*  @param softLimit
	*  @param hardLimit
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GetQuotaManagementFaultType
	*  @throws GenericFaultType
	*  @throws AuthenticationFaultType
	*
	*/
	public void getTotalQuota (javax.xml.rpc.holders.LongHolder softLimit, javax.xml.rpc.holders.LongHolder hardLimit)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.GetQuotaManagementFaultType,
				 org.glite.wms.wmproxy.GenericFaultType,
				 org.glite.wms.wmproxy.AuthenticationFaultType {

		//softLimit.value = 0;
		//hardLimit.value = 0;
		this.serviceStub.getTotalQuota(softLimit, hardLimit);

	}

	/**
	*
	*  @param softLimit
	*  @param hardLimit
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GetQuotaManagementFaultType
	*  @throws GenericFaultType
	*  @throws AuthenticationFaultType
	*
	*/
	public void getFreeQuota (javax.xml.rpc.holders.LongHolder softLimit, javax.xml.rpc.holders.LongHolder hardLimit)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.GetQuotaManagementFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType {

		//softLimit.value = 0;  TBR !!!!!!!!!
		//hardLimit.value = 0; TBR !!!!!!!!!

		this.serviceStub. getFreeQuota(softLimit, hardLimit);

	}

	/**
	*
	*  @param jobId the id of the job
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws OperationNotAllowedFaultType
	*  @throws InvalidArgumentFaultType
	*  @throws JobUnknownFaultType
	*
	*/
	public void jobPurge(java.lang.String jobId)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				 org.glite.wms.wmproxy.GenericFaultType,
				 org.glite.wms.wmproxy.AuthenticationFaultType,
				 org.glite.wms.wmproxy.OperationNotAllowedFaultType,
				 org.glite.wms.wmproxy.InvalidArgumentFaultType,
				 org.glite.wms.wmproxy.JobUnknownFaultType {

		logger.debug ("INPUT: jobid=[" + jobId + "]");

		this.serviceStub.jobPurge(jobId);

	}

	/**
	*
	*  @param jobId the id of the job
	*  @return the list of the job output files with their size (@see #org.glite.wms.wmproxy.StringAndLongList)
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws OperationNotAllowedFaultType
	*  @throws InvalidArgumentFaultType
	*  @throws JobUnknownFaultType
	*
	*/
	public org.glite.wms.wmproxy.StringAndLongList getOutputFileList (java.lang.String jobId)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType,
				org.glite.wms.wmproxy.OperationNotAllowedFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType,
				org.glite.wms.wmproxy.JobUnknownFaultType {

		logger.debug ("INPUT: jobid=[" + jobId + "]");

		return this.serviceStub.getOutputFileList (jobId);

	}

	/**
	*
	*  @param jdl the job description
	*  @return the list of the found resources and their rank values (@see #org.glite.wms.wmproxy.StringAndLongList)
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws OperationNotAllowedFaultType
	*  @throws InvalidArgumentFaultType
	*
	*/
	public org.glite.wms.wmproxy.StringAndLongList jobListMatch (java.lang.String jdl)
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType,
				org.glite.wms.wmproxy.NoSuitableResourcesFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType {

		logger.debug ("INPUT: JDL=[" + jdl + "]");

		return this.serviceStub.jobListMatch( jdl);
	}

	/**
	*
	*  @param jobType the job type description(@see #org.glite.wms.wmproxy.JobTypeList,  #org.glite.wms.wmproxy.JobType)
	*  @param executable the exacutable filename
	*  @param arguments the arguments of the executable
	*  @param requirements  a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template of the job
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws InvalidArgumentFaultType
	*
	*/
	public java.lang.String getJobTemplate (org.glite.wms.wmproxy.JobTypeList jobType,
										java.lang.String executable,
										java.lang.String arguments,
										java.lang.String requirements,
										java.lang.String rank)
				throws java.rmi.RemoteException,
					org.glite.wms.wmproxy.AuthorizationFaultType,
					org.glite.wms.wmproxy.GenericFaultType,
					org.glite.wms.wmproxy.AuthenticationFaultType,
					org.glite.wms.wmproxy.InvalidArgumentFaultType {

		logger.debug ("INPUT: executable=[" + executable + "] - arguments=[" + arguments + "]");
		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");
		return this.serviceStub.getJobTemplate(jobType, executable, arguments, requirements, rank);

	}

	/**
	*
	*  @param dependencies the description of the DAG  (@see #org.glite.wms.wmproxy.GraphStructType)
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) for all the nodes of the dag
	*  @return the jdl template of the DAG
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws InvalidArgumentFaultType
	*
	*/
	public java.lang.String getDAGTemplate (org.glite.wms.wmproxy.GraphStructType dependencies,
										java.lang.String requirements,
										java.lang.String rank)
				throws java.rmi.RemoteException,
					org.glite.wms.wmproxy.AuthorizationFaultType,
					org.glite.wms.wmproxy.GenericFaultType,
					org.glite.wms.wmproxy.AuthenticationFaultType,
					org.glite.wms.wmproxy.InvalidArgumentFaultType {

		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");
		return this.serviceStub.getDAGTemplate(dependencies, requirements, rank);

	}

	/**
	*
	*  @param jobNumber###TBD
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template of the DAG
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws InvalidArgumentFaultType
	*
	*/
	public java.lang.String getCollectionTemplate (int jobNumber,
											java.lang.String requirements,
											java.lang.String rank)
					throws java.rmi.RemoteException,
						org.glite.wms.wmproxy.AuthorizationFaultType,
						org.glite.wms.wmproxy.GenericFaultType,
						org.glite.wms.wmproxy.AuthenticationFaultType,
						org.glite.wms.wmproxy.InvalidArgumentFaultType {

		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");

		return this.serviceStub.getCollectionTemplate( jobNumber, requirements, rank);

	}

	/**
	*
	*  @param attributes the list of the attributes (@see #org.glite.wms.wmproxy.StringList )
	*  @param param			###TBD
	*  @param paramStart		###TBD
	*  @param paramStep		###TBD
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws InvalidArgumentFaultType
	*
	*/
	public java.lang.String getIntParametricJobTemplate (org.glite.wms.wmproxy.StringList attributes,
													int param,
													int parameterStart,
													int parameterStep,
													java.lang.String requirements,
													java.lang.String rank)
					throws java.rmi.RemoteException,
						org.glite.wms.wmproxy.AuthorizationFaultType,
						org.glite.wms.wmproxy.GenericFaultType,
						org.glite.wms.wmproxy.AuthenticationFaultType,
						org.glite.wms.wmproxy.InvalidArgumentFaultType {


		logger.debug ("INPUT: param=[" + param + "] - parameterStart=[" + parameterStart + "] - parameterStep=[" + parameterStep + "]");
		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");

		return this.serviceStub.getIntParametricJobTemplate(attributes, param, parameterStart, parameterStep, requirements, rank);

	}

	/**
	*
	*  @param attributes the list of the attributes (@see #org.glite.wms.wmproxy.StringList )
	*  @param param			###TBD
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template
	*  @throws RemoteException If any error occurs during the execution of the remote method call
	*  @throws GenericFaultType
	*  @throws AuthorizationFaultType
	*  @throws AuthenticationFaultType
	*  @throws InvalidArgumentFaultType
	*
	*/
	public java.lang.String getStringParametricJobTemplate (org.glite.wms.wmproxy.StringList attributes,
													 org.glite.wms.wmproxy.StringList param,
													 java.lang.String requirements,
													  java.lang.String rank)
					throws java.rmi.RemoteException,
						org.glite.wms.wmproxy.AuthorizationFaultType,
						org.glite.wms.wmproxy.GenericFaultType,
						org.glite.wms.wmproxy.AuthenticationFaultType,
						org.glite.wms.wmproxy.InvalidArgumentFaultType {

		return this.serviceStub.getStringParametricJobTemplate(attributes, param, requirements, rank);
	}
	 /**
	*
	* sets up the service
	*
	*/
	private void setUpService ( ) throws javax.xml.rpc.ServiceException,
					java.net.MalformedURLException,
					java.io.FileNotFoundException {

		String protocol = "";

		// Gets a stub to the service
		this.serviceLocator = new WMProxyLocator( );

		// pointer to Stub object
		this.serviceStub = (WMProxyStub) serviceLocator.getWMProxy_PortType( this.serviceURL ) ;

		// Checks for https protocol
		protocol = serviceURL.getProtocol( ).trim( );
		logger.debug(protocol);

		// Sets security properties for https connections
		if ( protocol.compareTo("https") == 0 ){

			System.setProperty("axis.socketSecureFactory","org.glite.security.trustmanager.axis.AXISSocketFactory");

			File file = new File ( proxyFile) ;
			if ( file.isFile ( ) == true )
				System.setProperty("gridProxyFile", proxyFile);
			else
				throw new FileNotFoundException ("proxy file not found : " + proxyFile);
		}

	}

	/* Service URL */
	private URL serviceURL = null;

	/* Proxy file location */
	private String proxyFile= null;

	/* Service location */
	private WMProxyLocator serviceLocator= null;

	/* Service stub */
	private WMProxyStub serviceStub= null ;

	/* Log manager */
	private Logger logger= null ;

}
