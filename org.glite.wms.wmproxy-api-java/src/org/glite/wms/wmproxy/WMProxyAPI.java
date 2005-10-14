
/*
 * Copyright (c) 2005 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 * 
 * Authors: Marco Sottilaro (marco.sottilaro@datamat.it)
 */


package org.glite.wms.wmproxy;


import java.net.URL;
import java.io.File ;
import java.io.FileNotFoundException ;

import java.io.ByteArrayInputStream ;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.CertificateException;
import java.security.cert.CertificateExpiredException;
import java.util.Date ;

import org.gridsite.www.namespaces.delegation_1.*;


import org.glite.security.delegation.GrDPX509Util;
import org.glite.security.delegation.GrDProxyGenerator;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
* Allows sending requests to the Workload Manager Proxy (WMProxy) server
* <p>
* The class WMProxyAPI represents a client API to access to the WMProxy services provided as Web Services.<BR>
* It provides a set of methods that allow :
* <UL>
* <LI>delegating the credential ;
* <LI>registering and submitting jobs ;
* <LI>cancelling the job during its life-cycle ;
* <LI>retrieving information on the location where the job input sandbox files can be stored ;
* <LI>retrieving the output sandbox files list ;
* <LI>retrieving a list of possible matching Computer Elements ;
* <LI> getting JDL templates ;
* <LI>getting information on the user disk quota on the server .
* </UL>
* </p>
* <p>
* Job requirements are expressed by Job Description Language (JDL).
* The types of jobs supported by the WM service are:
* <UL>
* <LI>Normal - a simple application
* <LI>DAG - a direct acyclic graph of dependent jobs
* <LI>Collection - a set of independent jobs
* <LI>Parametric - jobs with JDL's containing some parameters
* <LI>registering ans submitting jobs ;
* <LI>cancelling the job during its life-cycle ;
* <LI>retrieving information on the location where the job input sandbox files can be stored ;
* <LI>retrieving the output sandbox files list ;
* <LI>retrieving a list of possible matching Computer Elements ;
* <LI> getting JDL templates ;
* <LI>getting information on the user disk quota on the server .
* </UL>
 *
 * @author Marco Sottilaro <marco.sottilaro@datamat.it>
 * @version $Id$
 *
 */

public class WMProxyAPI{

	/**
	*  Constructor
	*  @param url WMProxy service URL
	*  @param proxyFile the path location of the user proxy file
	*  @throws  ServiceException If any error in calling the service
	*  @throws MalformedURLException malformed service URL specified as input
	*  @throws  FileNotFoundException Unable to find the user proxy file
	*
	*/
	public WMProxyAPI (String url, String proxyFile)
			throws 	javax.xml.rpc.ServiceException,
					java.net.MalformedURLException,
					java.io.FileNotFoundException {
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
	*  Constructor that allows setting of the Log4j tool by configuration file
	*  @param url WMProxy service URL
	*  @param proxyFile the path location of the user proxy file
	*  @param logPropFille the path location of the log4j properties file
	*  @throws ServiceException If any error occurs in calling the service
	*  @throws MalformedURLException malformed service URL specified as input
	*  @throws FileNotFoundException Unable to find the user proxy file
	*
	*/
	public WMProxyAPI (String url, String proxyFile, String logPropFile)
				throws 	javax.xml.rpc.ServiceException,
						java.net.MalformedURLException,
						java.io.FileNotFoundException {
		// logger
		if ( logPropFile != null)
			PropertyConfigurator.configure(logPropFile);
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
	*  Gets a proxy identified by the delegationId string.
	*  This method remains to keep compatibility with the version 1.0.0 of WMProxy servers,
	*  but it will be soon deprecated.
	*  @param delegationID the id to identify the delegation
	*  @return a string representing the proxy
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*/
	public java.lang.String getProxyReq (java.lang.String delegationId) throws java.rmi.RemoteException {
			logger.debug ("INPUT: delegationId=[" +delegationId + "]" );
			return this.serviceStub.getProxyReq(delegationId) ;
	}
	/**
	*  Gets a proxy identified by the delegationId string.
	* This method can be only used invoking WMProxy servers with versions greater than or equal to 2.0.0;
	*  the version of the server can be retrieved by calling the getVersion service.
	*  @param delegationID the id to identify the delegation
	*  @return a string representing the proxy
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @since wms.wmproxy-api-java version 1.2.0
	*/
	public java.lang.String grstGetProxyReq (java.lang.String delegationId) throws java.rmi.RemoteException {
			logger.debug ("INPUT: delegationId=[" +delegationId + "]" );
			return this.grstStub.getProxyReq(delegationId) ;
	}


	/**
	* This method allows delegating user credential to the WMProxy server:
	* it creates a proxy from the input certificate (got by a previous call to the createProxyfromCertReq server).
	* The created proxy is sent to the server and identified by a delegationId string.
	* This string can be used to call some services accepting a delegationId string as input parameter
	*  (like  jobRegister, jobSubmit, etc) until its expiration time.
	*  This method remains to keep compatibility with the version 1.0.0 of WMProxy servers,
	*  but it will be soon deprecated; the version of the server can be retrieved by calling the getVersion service.
	*  @param delegationId the id used to identify the delegation
	*  @param cert the input certificate
	*  @throws java.rmi.RemoteException If any error occurs during the execution of the remote method call to the WMProxy server
	* @throws java.io.FileNotFoundException if the local user proxy is not found
	* @throws java.security.cert.CertificateException if any error occurs during the generation of the proxy from the input certificate
	* @throws java.security.cert.CertificateExpiredException if the user proxy has expired
	* @see #getVersion
	*
	*/
	public void putProxy(java.lang.String delegationId, java.lang.String cert)
		throws 	java.rmi.RemoteException,
				java.lang.Exception,
				java.io.FileNotFoundException,
				java.security.cert.CertificateException,
				java.security.cert.CertificateExpiredException {
		logger.debug ("INPUT: cert=[" + cert+ "]");
		logger.debug ("INPUT: delegationId=[" +delegationId + "]");
		logger.debug ("Creating proxy from certificate (CreateProxyfromCertReq)");
		String proxy = this.createProxyfromCertReq(cert);
		logger.debug ("Delegating credential (putProxy)");
		this.serviceStub.putProxy(delegationId, proxy) ;
	}
	/**
	* This method allows delegating user credential to the WMProxy server using the GridSite package:
	* it creates a proxy from the input certificate (got by a previous call to the createProxyfromCertReq server).
	* The created proxy is sent to the server and identified by a delegationId string.
	* This string can be used to call some services accepting a delegationId string as input parameter
	*  (like  jobRegister, jobSubmit, etc) until its expiration time.
	* This method can be only used invoking WMProxy servers with versions greater than or equal to 2.0.0
	*  @param delegationId the id used to identify the delegation
	*  @param cert the input certificate
	*  @throws java.rmi.RemoteException If any error occurs during the execution of the remote method call to the WMProxy server
	* @throws java.io.FileNotFoundException if the local user proxy is not found
	* @throws java.security.cert.CertificateException if any error occurs during the generation of the proxy from the input certificate
	* @throws java.security.cert.CertificateExpiredException if the user proxy has expired
	*  @since wms.wmproxy-api-java version 1.2.0
	* @see #getVersion
	*
	*/
	public void grstPutProxy(java.lang.String delegationId, java.lang.String cert)
		throws 	java.rmi.RemoteException,
				 java.lang.Exception,
				java.io.FileNotFoundException,
				java.security.cert.CertificateException,
				java.security.cert.CertificateExpiredException {
		logger.debug ("INPUT: cert=[" + cert + "]");
		logger.debug ("INPUT: delegationId=[" +delegationId + "]");
		logger.debug ("Creating proxy from certificate (CreateProxyfromCertReq)");
		String proxy = this.createProxyfromCertReq(cert);
		logger.debug ("Delegating credential (putProxy)");
		this.serviceStub.putProxy(delegationId,proxy) ;
	}
	/**
	* Gets the Version of the WMProxy services
	* @return the version numbers
	* @exception RemoteException a problem occurred during the remote call to the WMProxy server
	* @throws AuthenticationFaultType an authentication problem occurred
	* @throws GenericFaultType a generic problem occurred
	*
	*/

	public java.lang.String getVersion()
			throws java.rmi.RemoteException,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType {
		return this.serviceStub.getVersion() ;
	}

	/**
	*   Registers a job for submission.The unique identifier assigned to the job is returned to the client.
	*  This operation only registers the job and assigns it with an identifier.
	*  The processing of the job (matchmaking, scheduling etc.) within the WM is not started.
	* The job is "held" by the system in the "Registered" status until the jobStart operation is called.
	* Between the two calls the client can perform all preparatory actions needed by the job to run
	* (e.g. the registration of input data, the upload of the job input sandbox files etc); especially
	* those actions requiring the specification of the job identifier.
	* The service supports registration (and consequently submission) of simple jobs, parametric jobs, partitionable jobs, DAGs and collections of jobs.
	* The description is always provided through a single JDL description.
	* When a clients requests for registration of a complex object, i.e. parametric and partitionable jobs, DAGs and collections of jobs (all those requests represent in fact a set of jobs),
	*  the operations returns a structure containing the main identifier of the complex object and the identifiers of all related sub jobs.
	*  @param jdl the job jdl representation.
	*  @param delegationId the id used to identify the delegation
	*  @return the structure containing the id of the registered job
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws GenericFaultType a generic problem occurred
	*  @throws AuthenticationFaultType a generic authentication problem occurred
	*  @throws InvalidArgumentFaultType the given JDL expression is not valid
	*
	*/
	public org.glite.wms.wmproxy.JobIdStructType jobRegister(java.lang.String jdl, java.lang.String delegationId)
			throws 	java.rmi.RemoteException,
					org.glite.wms.wmproxy.AuthorizationFaultType,
					org.glite.wms.wmproxy.GenericFaultType,
					org.glite.wms.wmproxy.InvalidArgumentFaultType {
		logger.debug ("INPUT: JDL=[" + jdl + "]");
		logger.debug ("INPUT: delegationId=[" +delegationId + "]");
		return this.serviceStub.jobRegister (jdl, delegationId) ;
	}

	/**
	*  Triggers the submission a previously registered job. It starts the actual processing of the registered job within the Workload Manager.
	* It is assumed that when this operation is called, all the work preparatory to the job
	* (e.g. input sandbox upload, registration of input data to the Data Management service etc.) has been completed by the client.<BR>
	* Here follows an example of the sequence of operations to be performed for submitting a job:<BR>
	* <UL>
	* <LI> jobId = jobRegister(JDL)
	* <LI> destURI = getSandboxDestURI(jobID)
	* <LI> transfer of the job Input Sandbox file to the returned destURI (using GridFTP)
	* <LI> jobStart(jobId).Triggers the submission for a previously registered job
	* </UL>
	*  @param jobId the id of the job
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws GenericFaultType a generic problem occurred
	*  @throws AuthenticationFaultType a generic authentication problem occurred
	*  @throws OperationNotAllowedFaultType the current job status does not allow requested operation.
	*  @throws InvalidArgumentFaultType the given job Id is not valid
	*  @throws JobUnknownFaultType the given job has not been registered to the system.
	*/
	public void jobStart(java.lang.String jobId)
			throws 	java.rmi.RemoteException,
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
	*  Submits a job: performs registration and triggers the submission
	* The JDL description of the job provided by the client is validated by the service,
	* registered to the LB and finally passed to the Workload Manager.
	* The unique identifier assigned to the job is returned to the client.
	* Usage of this operation (instead of jobRegister + jobStart) is indeed recommended when the job identifier is not needed prior to its submission
	* (e.g. jobs without input sandbox or with a sandbox entirely available on a GridFTP server managed by the client or her organisation).
	* The service supports submission of simple jobs, parametric jobs, partitionable jobs, DAGs and collections of jobs;
	* the description is always provided through a single JDL description.
	* When a clients requests for submission of a complex object, i.e. parametric and partitionable jobs, DAGs and collections of jobs
	* (all those requests represent in fact a set of jobs), the operations returns a structure containing the main identifier of the complex object
	*  and the identifiers of all related sub jobs.
	*  @param jobId the id of the job
	*  @param delegationId the id used to identify the delegation
	*  @return the id of the job
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws GenericException a generic problem occurred
	*  @throws AuthorizationFaultType client is not authorized to perform this operation
	*  @throws AuthenticationFaultType a generic authentication problem occurred
	*  @throws InvalidArgumentFaultType the given job JDL expression is not valid
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
	*  This operation cancels a previously submitted job identified by its JobId. If the job is still managed by the WM then it is removed from the WM tasks queue.
	* If the job has been already sent to the CE, the WM simply forwards the request to the CE.
	* For suspending/releasing and sending signals to a submitted job the user has to check that the job has been scheduled to a CE
	* and access directly the corresponding operations made available by the CE service.
	*  @param jobId the id of the job
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws GenericFaultType a generic problem occurred.
	*  @throws AuthorizationFaultType client is not authorized to perform this operation.
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws OperationNotAllowedFaultType current job status does not allow requested operation
	*  @throws InvalidArgumentFaultType the given job Id is not valid.
	*  @throws JobUnknownFaultType the given job has not been registered to the system
	*
	*/
	public void jobCancel(java.lang.String jobId)
			throws 	java.rmi.RemoteException,
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
	* Returns the maximum Input sandbox size (in bytes) a user can count on for a job submission if using the space managed by the WM.
	* This is a static value in the WM configuration (on a job-basis) set by the VO administrator
	*  @return the size of the InputSandbox (in bytes)
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws GenericFaultType a genric problem occurred
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*
	*/
	public double getMaxInputSandboxSize()
			throws 	java.rmi.RemoteException,
					org.glite.wms.wmproxy.GenericFaultType,
					org.glite.wms.wmproxy.AuthenticationFaultType {
		return this.serviceStub.getMaxInputSandboxSize();
	}

	/**
	* Returns a list of destination URI's associated to the job, identified by the jobId provided as input,
	* where the job input sandbox files can be uploaded by the client.
	* The location is created in the storage managed by the WM (if needed) and
	* the corresponding URI is returned to the operation caller if no problems has been arised during creation.
	* Files of the job input sandbox that have been referenced in the JDL as relative or absolute paths
 	* are expected to be found in the returned location when the job lands on the CE.
	* File upload can be performed by the GridFTP/HTTPS server available on the WMS node(using file-transferring tools like curl and globus-url-copy).
	* The user can also specify in the JDL the complete URI of files that are stored on a GridFTP/HTTPS server (e.g. managed by her organisation);
	* those files will be directly downloaded (by the JobWrapper) on the WN where the job will run without transiting on the WMS machine.
	* The same applies to the output sandbox files list, i.e. the user can specify in the JDL the complete URIs for the files of the output sandbox;
	* those files will be directly uploaded (by the JobWrapper) from the WN to the specified GridFTP/HTTPS servers without transiting on the WMS machine.
	*  @param jobId string containing the JobId
	*  @return the list of the Sandbox destionation URI's strings
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws GenericFaultType a generic problem occurred
	*  @throws AuthorizationFaultType client is not authorized to perform this operation.
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws OperationNotAllowedFaultType current job status does not allow requested operation.
	*  @throws InvalidArgumentFaultType the given job Id is not valid.
	*  @throws JobUnknownFaultType the given job has not been registered to the system.
	*
	*/
	public org.glite.wms.wmproxy.StringList getSandboxDestURI(java.lang.String jobId)
			throws 	java.rmi.RemoteException,
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
	* Returns the list of destination URIs associated to a compound job (i.e. a DAG a Collection or a parametric jobs) and all of its sub-jobs in a complex structure containing:*
	* <UL>
	* <LI> the job id
	* <LI> the corresponding list of destination URIs (can be more than one if different transfer protocols are supported, e.g. gsiftp, https etc.)
	* <UL>
	* The structure contains an element (structure above) for the compound job id provided (at first position) and one further element for any sub nodes.
	* It contains only one element if the job id provided as imnput is the identifier of a simple job.
	* The location is created in the storage managed by the WMS and the corresponding URI is returned to the operation caller if no problems has been arised during creation.
	* Files of the job input sandbox that have been referenced in the JDL as relative or absolute paths are expected to be found in the returned location when the job lands on the CE.	* 	* File upload can be performed by the GridFTP/HTTPS server available on the WMS node(using file-transferring tools like curl and globus-url-copy).
	* The user can also specify in the JDL the complete URI of files that are stored on a GridFTP/HTTPS server (e.g. managed by her organisation);
	* those files will be directly downloaded (by the JobWrapper) on the WN where the job will run without transiting on the WMS machine.
	* The same applies to the output sandbox files list, i.e. the user can specify in the JDL the complete URIs for the files of the output sandbox;
	* those files will be directly uploaded (by the JobWrapper) from the WN to the specified GridFTP/HTTPS servers without transiting on the WMS machine.
	*  @param jobId string containing the JobId
	*  @return the list of the Sandbox destionation URI's strings
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws GenericFaultType a generic problem occurred
	*  @throws AuthorizationFaultType client is not authorized to perform this operation.
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws OperationNotAllowedFaultType current job status does not allow requested operation.
	*  @throws InvalidArgumentFaultType the given job Id is not valid.
	*  @throws JobUnknownFaultType the given job has not been registered to the system.
	*/
	public DestURIsStructType getSandboxBulkDestURI(java.lang.String jobId)
			throws 	java.rmi.RemoteException,
					org.glite.wms.wmproxy.AuthorizationFaultType,
					org.glite.wms.wmproxy.GenericFaultType,
					org.glite.wms.wmproxy.AuthenticationFaultType,
					org.glite.wms.wmproxy.OperationNotAllowedFaultType,
					org.glite.wms.wmproxy.InvalidArgumentFaultType,
					org.glite.wms.wmproxy.JobUnknownFaultType {
		logger.debug ("INPUT: jobid=[" + jobId + "]");
		return this.serviceStub.getSandboxBulkDestURI(jobId);
	}
	/**
	* Returns the total space quota assigned to the user on the storage managed by the WM
	* The fault GetQuotaManagementFault is returned if the quota management is not active on the WM.
	*  @param softLimit the returned value of the soft limit free quota i.e. the difference between quota soft limit and user's disk usage.
	*  @param hardLimit  the returned value of the hard limit quota (in bytes) i.e. the real quota limit that cannot be exceeded.
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws GetQuotaManagementFaultType quota management is not active on the WM.
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public void getTotalQuota (javax.xml.rpc.holders.LongHolder softLimit, javax.xml.rpc.holders.LongHolder hardLimit)
			throws 	java.rmi.RemoteException,
					org.glite.wms.wmproxy.GetQuotaManagementFaultType,
				 	org.glite.wms.wmproxy.GenericFaultType,
					 org.glite.wms.wmproxy.AuthenticationFaultType {
		this.serviceStub.getTotalQuota(softLimit, hardLimit);
	}

	/**
	* Returns the remaining free part of available user disk quota (in bytes).
	* The fault GetQuotaManagementFault is returned if the quota management is not active.
	*  @param softLimit the returned value of the soft limit free quota i.e. the difference between quota soft limit and user's disk usage.
	*  @param hardLimit  the returned value of the hard limit quota (in bytes) i.e. the real quota limit that cannot be exceeded.
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws GetQuotaManagementFaultType quota management is not active on the WM.
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public void getFreeQuota (javax.xml.rpc.holders.LongHolder softLimit, javax.xml.rpc.holders.LongHolder hardLimit)
			throws 	java.rmi.RemoteException,
					org.glite.wms.wmproxy.GetQuotaManagementFaultType,
					org.glite.wms.wmproxy.GenericFaultType,
					org.glite.wms.wmproxy.AuthenticationFaultType {
		this.serviceStub. getFreeQuota(softLimit, hardLimit);
	}

	/**
	*  Removes from the WM managed space all files related to the job identified by the jobId provided as input.
	* It can only be applied for job related files that are managed by the WM.
	* E.g. Input/Output sandbox files that have been specified in the JDL through a URI will be not subjected to this management.
	*  @param jobId the identifier of the job
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType the given job Id is not valid.
	*  @throws JobUnknownFaultType the given job has not been registered to the system.
	*  @throws OperationNotAllowedFaultType current job status does not allow requested operation
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public void jobPurge(java.lang.String jobId)
			throws 	java.rmi.RemoteException,
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
	* Returns the list of URIs where the output files created during job execution have been stored in the WM managed space and the corresponding sizes in bytes.
	* It can only be applied for files of the Output Sandbox that are managed by the WM (i.e. not specified as URI in the JDL).
	*  @param jobId the identifier of the job
	*  @return  the list of objects containing the file URI and the corresponding size in bytes
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType the given job Id is not valid.
	*  @throws JobUnknownFaultType the given job has not been registered to the system
	*  @throws OperationNotAllowedFaultType current job status does not allow requested operation
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public org.glite.wms.wmproxy.StringAndLongList getOutputFileList (java.lang.String jobId)
			throws 	java.rmi.RemoteException,
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
        * Returns the list of CE Ids satisfying the job Requirements specified in the JDL, ordered according to the decreasing Rank.
	* The fault NoSuitableResourcesFault is returned if there are no resources matching job constraints.
        *  @param jdl the job description
        *  @param delegationId the string used previously to delegate credential
        *  @return the list of the found resources and their rank values
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType the given job JDL expression is not valid.
	*  @throws NoSuitableResourcesFault no suitable resources matching job requirements have been found.
	*  @throws GenericFaultType a generic problem occurred
        *
        */
        public org.glite.wms.wmproxy.StringAndLongList jobListMatch (java.lang.String jdl, java.lang.String delegationId)
                        throws  java.rmi.RemoteException,
				org.glite.wms.wmproxy.AuthorizationFaultType,
				org.glite.wms.wmproxy.AuthenticationFaultType,
				org.glite.wms.wmproxy.GenericFaultType,
				org.glite.wms.wmproxy.NoSuitableResourcesFaultType,
				org.glite.wms.wmproxy.InvalidArgumentFaultType {
                logger.debug ("INPUT: JDL=[" + jdl + "] - deleagtionId=[" + delegationId + "]");
                return this.serviceStub.jobListMatch(jdl, delegationId);
        }
	/**
	* Enables file perusal functionalities if not disabled with the specific jdl attribute during job
	* register operation.
	* Calling this operation, the user enables perusal for job identified by jobId, for files specified with fileList.
	* After this operation, the URIs of perusal files generated during job execution can be retrieved by calling the getPerusalFiles service
	* An empty fileList disables perusal.
	* This method can be only used invoking WMProxy servers with version greater than or equal to 2.0.0;
	*  the version of the server can be retrieved by calling the getVersion service.
	* @param jobid the string with the job identifier
	* @param fileList the list of filenames to be enabled
	* @throws AuthenticationFaultType An authentication problem occurred
	* @throws AuthorizationFaultType The user is not authorized to perform this operation
	* @throws InvalidArgumentFaultType If the given JDL is not valid
	* @throws JobUnknownFaultType The provided jobId has not been registered to the system
	* @throws OperationNotAllowedFaultType perusal was disabled with the specific jdl attribute.
	* @see #getPerusalFiles
	* @see #getVersion
	*/
	public void enableFilePerusal (java.lang.String  jobId, org.glite.wms.wmproxy.StringList fileList)
			throws  java.rmi.RemoteException,
					org.glite.wms.wmproxy.AuthenticationFaultType,
                                        org.glite.wms.wmproxy.AuthorizationFaultType,
					org.glite.wms.wmproxy.JobUnknownFaultType,
					org.glite.wms.wmproxy.GenericFaultType {
		logger.debug ("INPUT: jobId=[" +jobId + "]");
		this.serviceStub.enableFilePerusal(jobId, fileList);
	 }
	 /**
	* Gets the URIs of perusal files generated during job execution for the specified file file.
	* If allChunks is set to true all perusal URIs will be returned; also the URIs already requested with a
	* previous getPerusalFiles operation. Default value is false.
	* Perusal files have to be presiuosly enabled by calling the enableFilePerusal service
	* This method can be only used invoking WMProxy servers with version greater than or equal to 2.0.0;
	*  the version of the server can be retrieved by calling the getVersion service.
	* @param jobid the string with the job identifier
	* @param allchuncks boolean value to specify when to get all chuncks
	* @param jobid the string with the job identifier
	* @param file the name of the perusal file be enabled
	* @param jobid the string with the job identifier
	* @param file the name of the perusal file be enabled
	* @param cfs Non-default configuration context (proxy file, endpoint URL and trusted cert location) ;  if NULL, the object is created with the default parameters
	* @throws AuthenticationFaultType An authentication problem occurred
	* @throws AuthorizationFaultType The user is not authorized to perform this operation
	* @throws InvalidArgumentFaultType If the given JDL is not valid
	* @throws JobUnknownFaultType The provided jobId has not been registered to the system
	* @throws OperationNotAllowedFaultType perusal was disabled with the specific jdl attribute.
	* @see #enableFilePerusal
	* @see #getVersion
	* @see AuthenticationException, AuthorizationException, InvalidArgumentException, JobUnknownException, OperationNotAllowedException, BaseException
	*/
	public  org.glite.wms.wmproxy.StringList getPerusalFiles (java.lang.String  jobId, java.lang.String file, boolean allchunks)
			throws  java.rmi.RemoteException,
					org.glite.wms.wmproxy.AuthenticationFaultType,
                                        org.glite.wms.wmproxy.AuthorizationFaultType,
					org.glite.wms.wmproxy.JobUnknownFaultType,
					org.glite.wms.wmproxy.GenericFaultType {
		logger.debug ("INPUT: jobId=[" + jobId + "] - file=[" +file + "] - allchunck=[" + allchunks + "]");
		return this.serviceStub.getPerusalFiles(jobId, file, allchunks);
	 }
	/**
	*  Returns a JDL template for the requested job type.
	*  @param jobType the job type description
	*  @param executable the exacutable filename
	*  @param arguments the arguments of the executable
	*  @param requirements  a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template of the job
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType one or more of the given input parameters is not valid.
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public java.lang.String getJobTemplate (org.glite.wms.wmproxy.JobTypeList jobType,
										java.lang.String executable,
										java.lang.String arguments,
										java.lang.String requirements,
										java.lang.String rank)
				throws 	java.rmi.RemoteException,
						org.glite.wms.wmproxy.AuthorizationFaultType,
						org.glite.wms.wmproxy.GenericFaultType,
						org.glite.wms.wmproxy.AuthenticationFaultType,
						org.glite.wms.wmproxy.InvalidArgumentFaultType {
		logger.debug ("INPUT: executable=[" + executable + "] - arguments=[" + arguments + "]");
		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");
		return this.serviceStub.getJobTemplate(jobType, executable, arguments, requirements, rank);

	}
	/**
	*  Returns a JDL template for a DAG.
	*  @param dependencies the description of the DAG
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) for all the nodes of the dag
	*  @return the jdl template of the DAG
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType one or more of the given input parameters is not valid.
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public java.lang.String getDAGTemplate (org.glite.wms.wmproxy.GraphStructType dependencies,
										java.lang.String requirements,
										java.lang.String rank)
				throws	 java.rmi.RemoteException,
						org.glite.wms.wmproxy.AuthorizationFaultType,
						org.glite.wms.wmproxy.GenericFaultType,
						org.glite.wms.wmproxy.AuthenticationFaultType,
						org.glite.wms.wmproxy.InvalidArgumentFaultType {
		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");
		return this.serviceStub.getDAGTemplate(dependencies, requirements, rank);

	}

	/**
	*  Returns a JDL template for a collection of jobs, that is a set of independent jobs that can be submitted, controlled and monitored as a single entity.
	*  @param jobNumber integer representing the number of jobs of the collection.
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template of the collection
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType one or more of the given input parameters is not valid.
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public java.lang.String getCollectionTemplate (int jobNumber,
					java.lang.String requirements,
					java.lang.String rank)
					throws	 java.rmi.RemoteException,
							org.glite.wms.wmproxy.AuthorizationFaultType,
							org.glite.wms.wmproxy.GenericFaultType,
							org.glite.wms.wmproxy.AuthenticationFaultType,
							org.glite.wms.wmproxy.InvalidArgumentFaultType {
		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");
		return this.serviceStub.getCollectionTemplate( jobNumber, requirements, rank);

	}

	/**
	*  Returns a JDL template for a parametric of job, which is a job having one or more parametric attributes in the JDL.
	* The parametric attributes vary their values according to the "Parameter" attribute specified in the JDL itself (in this case the parametere has to be an integer).
	* The submission of a Parametric job results in the submission  of a set of jobs having the same descritpion apart from the parametrised attribute. They can be however controlled and monitored as a single entity.
	*  @param attributes list of strings representing the parametric JDL attributes
	*  @param param integer representing the value of the Parameter attribute
	*  @param paramStart	 integer representing the start value for varying the parametric attributes
	*  @param paramStep intege representing the step of each variation
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType one or more of the given input parameters is not valid.
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public java.lang.String getIntParametricJobTemplate (org.glite.wms.wmproxy.StringList attributes,
													int param,
													int parameterStart,
													int parameterStep,
													java.lang.String requirements,
													java.lang.String rank)
					throws 	java.rmi.RemoteException,
							org.glite.wms.wmproxy.AuthorizationFaultType,
							org.glite.wms.wmproxy.GenericFaultType,
							org.glite.wms.wmproxy.AuthenticationFaultType,
							org.glite.wms.wmproxy.InvalidArgumentFaultType {

		logger.debug ("INPUT: param=[" + param + "] - parameterStart=[" + parameterStart + "] - parameterStep=[" + parameterStep + "]");
		logger.debug ("INPUT: requirements=[" + requirements + "] - rank=[" + rank + "]");
		return this.serviceStub.getIntParametricJobTemplate(attributes, param, parameterStart, parameterStep, requirements, rank);

	}

	/**
	*  Returns a JDL template for a parametric of job, which is a job having one or more parametric attributes in the JDL.
	* The parametric attributes vary their values according to the "Parameter" attribute specified in the JDL itself (in this case the parametere has to be a list of strings).
	* The submission of a Parametric job results in the submission  of a set of jobs having the same descritpion apart from the value of the parametric attributes.
	* They can be however controlled and monitored as a single entity.
	*  @param attributes list of strings representing the parametric JDL attributes
	*  @param param list of strings representing the values of the Parameter attribute
	*  @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
	*  @param rank a string representing the expression for the rank (which is an attribute of double type) of the resources
	*  @return the jdl template
	*  @throws RemoteException a problem occurred during the remote call to the WMProxy server
	*  @throws AuthenticationFaultType a generic authentication problem occurred.
	*  @throws AuthorizationFaultType the client is not authorized to perform this operation.
	*  @throws InvalidArgumentFaultType one or more of the given input parameters is not valid.
	*  @throws GenericFaultType a generic problem occurred
	*
	*/
	public java.lang.String getStringParametricJobTemplate (org.glite.wms.wmproxy.StringList attributes,
													 org.glite.wms.wmproxy.StringList param,
													 java.lang.String requirements,
													  java.lang.String rank)
					throws 	java.rmi.RemoteException,
							org.glite.wms.wmproxy.AuthorizationFaultType,
							org.glite.wms.wmproxy.GenericFaultType,
							org.glite.wms.wmproxy.AuthenticationFaultType,
							org.glite.wms.wmproxy.InvalidArgumentFaultType {
		return this.serviceStub.getStringParametricJobTemplate(attributes, param, requirements, rank);
	}
	/**
	*  Generates a proxy from the input certificate and from the user proxy file on the user local machine
	* (which path has to be specified as input parameter in one of the class costructors)
	*  This service is called after the getProxyReq
	*  the input proxy string of this service is the string got by getProxyReq.
	*  @param certReq Service certificate request
	*  @return the generated proxy certificate
	*  @throws Exception If any error occurs during the creation of the proxy
	*  @throws  FileNotException If proxy file doesn't exist
	*  @throws CertificateException in case of any error during the local proxy loading operations
	*  @throws CertificateExpiredException if the local proxy has expired
	*/

	private String createProxyfromCertReq(java.lang.String certReq)
	throws    java.lang.Exception,
			java.io.FileNotFoundException,
			java.security.cert.CertificateException,
			java.security.cert.CertificateExpiredException {
		byte[ ] proxy = null;
		ByteArrayInputStream stream = null;
		CertificateFactory cf = null;
		X509Certificate cert = null ;
		long lifetime = 0;
		// generator object
		GrDProxyGenerator generator = new GrDProxyGenerator ( );
		// user proxy file
		File file = new File ( this.proxyFile);
		if ( file.isFile ( ) != true ) {
			throw new FileNotFoundException ( "proxy file not found at: "+ this.proxyFile );
		}
		try{
			// gets the local proxy as array of bye
			proxy = GrDPX509Util.getFilesBytes( file );
			// reads the proxy time-left
			stream = new ByteArrayInputStream(proxy);
			cf = CertificateFactory.getInstance("X.509");
			cert = (X509Certificate)cf.generateCertificate(stream);
 			stream.close();
			Date now = new Date ( );
			lifetime =( cert.getNotAfter().getTime ( ) - now.getTime() ) / 3600000  ; // in hour ! (TBC in secs)
		} catch (Exception exc){
			String errmsg = "an error occured while loading the local proxy  ("  + this.proxyFile +"): \n";
			errmsg += exc.toString();
			throw new CertificateException (errmsg);
		}
		// checks if the proxy is still valid
		if (lifetime < 0 ){
			throw new CertificateExpiredException ("the local proxy has expired(" + this.proxyFile +")" );
		}
		// sets the lifetime
		generator.setLifetime ((int)lifetime);
		// creates the new proxy
		proxy =  generator.x509MakeProxyCert(certReq.getBytes( ) , proxy, "");
		// converts the proxy from byte[] to String
		return new String(proxy);
	}
	 /**
	*
	* Sets up the service
	*
	*/
	private void setUpService ( ) throws javax.xml.rpc.ServiceException,
					java.net.MalformedURLException,
					java.io.FileNotFoundException {
		String protocol = "";
		// Gets a stub to the service
		this.serviceLocator = new WMProxyLocator( );
		// pointer to GliteWMProxy Stub object
		this.serviceStub = (WMProxyStub) serviceLocator.getWMProxy_PortType( this.serviceURL ) ;
		// pointer to Gridsite Stub object
		this.grstStub = (DelegationSoapBindingStub) serviceLocator.getWMProxyDelegation_PortType( this.serviceURL ) ;
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
	/** Service URL */
	private URL serviceURL = null;
	/** Proxy file location */
	private String proxyFile= null;
	/** Service location */
	private WMProxyLocator serviceLocator= null;
	/** Glite WMProxy Service Stub */
	private WMProxyStub serviceStub= null ;
	/** GridSite Service Stub */
	private DelegationSoapBindingStub grstStub = null;
	/** Log manager */
	private Logger logger= null ;
}
