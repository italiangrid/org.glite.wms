/*
* UserCredential.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*/
package org.glite.wmsui.apij;
import java.util.* ;
import java.io.* ;
import org.globus.gsi.*;
import org.globus.gsi.bc.BouncyCastleOpenSSLKey;
import org.globus.util.Util;
import org.edg.security.voms.VOMSInfo;
import org.edg.security.voms.VOMSExtension;
import org.edg.security.authorization.DNConvert;
import java.security.cert.X509Certificate;

/**
 * The UserCredential class allows to control the general user security certificate's validity.
 * <p>
 * This class can manipulate standard proxies as well as VOMS certificates, in order to extract the extension information
 * The main operation are:
 * <ul>
 * <li> Create whether a default or a custom proxy certificate
 * <li> Read a proxy certificate and retrieve any possible information ( time left, subject, strengthetc...)
 * <li> Retrieve back the default proxy certificate at any time
 * <li> Extract and check extensions such as VO name and VO groups from a VOMS certificate
 * </ul>

 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it> */
public class UserCredential{
	/**
	* Empty Constructor  set a proxy
	@param proxy an instance of a proxy certificate*/
	protected UserCredential (GlobusCredential proxy){ this.proxy  = proxy ; } ;
	/**
	* Empty Constructor
	* try to Load the default proxycertificate
	* @throws FileNotFoundException Unable to retrieve find the default proxy certificate
	* @throws GlobusCredentialException Unable to parse the default proxy certificate   */
	public UserCredential () throws FileNotFoundException , GlobusCredentialException {
		proxy = new GlobusCredential(   getDefaultProxy()  );
	}
	/** Constructor with File.
	* Try to set the specified path as the default proxy
	* @param  credPath a File where to load the proxy from
	* @throws FileNotFoundException Unable to retrieve find the default proxy certificate
	* @throws GlobusCredentialException Unable to parse the default proxy certificate */
	public UserCredential (File credPath) throws java.io.FileNotFoundException , org.globus.gsi.GlobusCredentialException { setProxy   (credPath)  ; }
	/**
	* Destroy the proxy file used */
	public void destroyProxy ( )   { /** TBD */}
	/** Create the default proxy
	* Create the default proxy certificate with all the default values: 512-key-length bits, 24c hours, not limited
	* @param passPhrase the passphrase needed in oprder to generate the proxy certificate
	*@throws FileNotFoundException unable to find one or more default files
	* @throws IOException when Input Output occurred while creating/loading certificates
	* @throws java.security.GeneralSecurityException when error occurred while decrypting keys/passphrase
	* @throws GlobusCredentialException unable to create the proxy certificate
	*/
	public static UserCredential createProxy (String passPhrase)
	throws FileNotFoundException  , IOException , java.security.GeneralSecurityException, FileNotFoundException, GlobusCredentialException{
		return createProxy (passPhrase, null , null , null , 512 , 24 , false  )  ;
	}
	/** Create a different proxy than the default one
	* @param passPhrase the passphrase needed in oprder to generate the proxy certificate
	* @param userProxy a string representing the file that will be created
	* @param userCert the String which point to the user cert file if different from the default one
	* @param userKey the String which point to the user key file if different from the default one
	* @param bits the key-length bits dimension ( default value is 512 )
	* @param hours the validity length of proxy in hours (default value is 24)
	* @param limited Deterimne whether  the proxy is limited or not (default value is false)
	*@throws FileNotFoundException unable to find one or more default files
	* @throws IOException when Input Output occurred while creating/loading certificates
	* @throws java.security.GeneralSecurityException when error occurred while decrypting keys/passphrase
	* @throws GlobusCredentialException unable to create the proxy certificate
	*/
	public static UserCredential createProxy (String passPhrase, String userProxy, String userCert , String userKey , int bits , int hours , boolean limited )
	throws FileNotFoundException , IOException, java.security.GeneralSecurityException, GlobusCredentialException{
		if ( userCert == null)
			userCert = getDefaultCert() ;
		if ( userKey == null)
			userKey = getDefaultKey();
		if ( userProxy == null)
			userProxy= getDefaultProxy();
		org.globus.gsi.bc.BouncyCastleOpenSSLKey key =
                    new org.globus.gsi.bc.BouncyCastleOpenSSLKey(userKey);
                CertUtil.init(); // Loads and initializes class.
                if (key.isEncrypted()) {
                  key.decrypt(passPhrase);
                }
                org.globus.gsi.bc.BouncyCastleCertProcessingFactory factory =
                    org.globus.gsi.bc.BouncyCastleCertProcessingFactory.getDefault();
		GlobusCredential proxy =
                      factory.createCredential(new X509Certificate[] {CertUtil.loadCertificate(userCert)},
                                               key.getPrivateKey(), bits, (hours * 3600),
                                               limited ? GSIConstants.DELEGATION_LIMITED :
                                               GSIConstants.DELEGATION_FULL);
		OutputStream out = null;
		out = new FileOutputStream(   userProxy);
		// set read only permissions
		if (!Util.setFilePermissions( userProxy, 600)) {
			throw new IOException("Warning: Please check file permissions for your proxy file: " + userProxy);
		}
		// write the contents
		proxy.save(out);
		out.close();
		GlobusCredential.setDefaultCredential(proxy) ;
		return new UserCredential (proxy) ;
	}
	/**
	* Retrieve where default proxy file should be created
	* @return the string representation of the proxy file name  */
	public static String getDefaultProxyName ()  {
		// System.out.println(" UserCredential::getDefaultProxyName. DEFAULT_PROXY_VAR=" + DEFAULT_PROXY_VAR ) ;
		return DEFAULT_PROXY ;
	}
	/**
	* Set statically the default proxy used for the current session.
	*@param proxyName the string representation of the proxy file name
	*/
	public synchronized static void setDefaultProxy( String proxyName) {
		DEFAULT_PROXY_VAR = proxyName ;
	}
	/** Retrieve the default path for proxy  and check whether the file exists
	* @throws FileNotFoundException if the file is not present
	* @return the string representation of the proxy file name
	*/
	public static String getDefaultProxy () throws FileNotFoundException{
		String userProxy ;
		if ( DEFAULT_PROXY_VAR ==null ) userProxy = getDefaultProxyName () ;
		else userProxy=DEFAULT_PROXY_VAR  ;
		if ( !  new File ( userProxy).isFile()   )
			throw new  FileNotFoundException ("Unable to find the default proxy file:\n" + userProxy );
		return userProxy ;
	}
	/** retrieve the default path for user cert
	* @return the string representation of the User cert file name
	* @throws FileNotFoundException if the file is not present
	*/
	public static String getDefaultCert () throws FileNotFoundException{
		// Check for Default userCert
		String userCert = Api.getEnv (CERT) ;
		if ( userCert == null )
			userCert = HOME + SEP + ".globus" + SEP + "usercert.pem" ;
		if ( !  new File ( userCert).isFile()   )
			throw new  FileNotFoundException ("Unable to find the default usercert file:\n" + userCert );
		return userCert ;
	}
	/** retrieve the default path for user key
	* @throws FileNotFoundException if the file is not present
	* @return the string representation of the user key file name  */
	public static String getDefaultKey () throws FileNotFoundException{
		// Check for Default userkey
		String userKey = Api.getEnv ( KEY) ;
		// System.out.println ("GOT DEFAULT userKey  : " +  userKey);
		if ( userKey == null )
			userKey = HOME + SEP + ".globus" + SEP + "userkey.pem" ;
		if ( !  new File ( userKey).isFile()   )
			throw new  FileNotFoundException ("Unable to find the default userkey file:\n" + userKey );
		// System.out.println ("GOT DEFAULT userKey  : " +  userKey);
		return userKey ;
	}
	/** retrieve the default path for cert dir
	* @return the string representation of the user cert dir
	* @throws FileNotFoundException if the dir is not present */
	public static String getDefaultDir () throws FileNotFoundException{
		// Check for Default userkey
		String userDir= Api.getEnv ( DIR) ;
		if ( userDir== null )
			userDir = SEP + "etc" + SEP +"grid-security" ;
		if ( !  new File ( userDir).isDirectory()    )
			throw new  FileNotFoundException ("Unable to find the default certdir directory:\n" + userDir );
		return userDir ;
	}
	/**  Set the specified path to be the default x509 proxy file  */
	public void setEnvProxy (String proxyPath){
		Api.setEnv( PROXY , proxyPath ) ;
		userProxy = proxyPath ;
	}
	/**  Set the specified path to be the default x509 certificate file  */
	public void setEnvCert (String certPath){
		Api.setEnv( CERT ,  certPath ) ;
		userCert =  certPath ;
	}
	/**  Set the specified path to be the default x509 key file  */
	public void setEnvKey (String keyPath){
		Api.setEnv( KEY ,  keyPath ) ;
		userKey =  keyPath ;
	}
	/**  Set the specified path to be the default x509 user cert diectory  */
	public void setEnvDir (String dirPath){
		Api.setEnv( DIR , dirPath  ) ;
		userDir =  dirPath ;
	}

	/**
	* Set the proxy certificate to a non-default value
	* @param credPath the path pointing to the proxy certificate file
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate  */
	public void setProxy (File credPath) throws GlobusCredentialException {
		// This Environment must be set in order to let the native api recognise the proxy
		setEnvProxy (credPath.toString()) ;
		//!!!proxy = GlobusCredential.load(credPath.toString()) ;
                proxy = new GlobusCredential(credPath.toString());
		GlobusCredential.setDefaultCredential(proxy) ;
	}
	/** Unset a previous non-default proxy and look for the default one
	* @throws GlobusCredentialException - Unable to get the default proxy certificate */
	public void unsetProxy () throws GlobusCredentialException, FileNotFoundException {
		//!!!proxy = GlobusCredential.load(  getDefaultProxy() , null  );
                proxy = new GlobusCredential(getDefaultProxy());
		GlobusCredential.setDefaultCredential(proxy) ;
	}
	/**Check if the Proxy Certificate is valid
	* @throws GlobusCredentialException - Unable to get the proxy certificate  */
	public void checkProxy ( ) throws GlobusCredentialException{
	if (  getTimeLeft()  <  0 )
		throw  new GlobusCredentialException(GlobusCredentialException.EXPIRED, "Proxy Certificate time left expired", new Throwable()) ;
	else if (  getTimeLeft()  <  EXPIRATION_LIMIT )
		throw  new GlobusCredentialException(GlobusCredentialException.EXPIRED, "Proxy Certificate time left not enough to perform any operation", new Throwable()) ;
	};
	/**
	*    Return the Subject of the Proxy Certificate
	* @throws GlobusCredentialException - Unable to get the proxy certificate */
	public String getSubject(   )throws GlobusCredentialException {
		return proxy.getSubject()  ;
	}
	/**
	*    Return the Issuer of the Proxy Certificate
	* @throws GlobusCredentialException - Unable to get the proxy certificate */
	public String getIssuer(   )throws GlobusCredentialException {
		return CertUtil.toGlobusID(  new DNConvert(   proxy.getIssuer().trim()   ).reformat( DNConvert.RFC2253  )   );
	}
	/**
	*   Return whether the proxy is a full proxy (true) or a limited proxy (false)
	* @throws GlobusCredentialException - Unable to get the proxy certificate
	*/
	public boolean  getCredType    ( )  {
		return !CertUtil.isLimitedProxy(proxy.getProxyType()) ; };
	/**
	*    Return the Cred type of the Proxy Certificate
	* @throws GlobusCredentialException - Unable to get the proxy certificate
	*/
	public int  getStrenght    ( )  { return  proxy.getStrength() ; };
	/**
	*    Return the Strenght of the Proxy Certificate
	* @throws GlobusCredentialException - Unable to get the proxy certificate
	*/
	public int  getTimeLeft     ( ) { return (int)proxy.getTimeLeft()  ; };


  /****************************************
  *     VOMS METHODS IMPLEMENTATION
  *****************************************/

  /**
   * Returns the proxy user subject removing all occurencies of "CN=/Proxy".
   * @throws GlobusCredentialException - Unable to load proxy certificate.
   * @return proxy user subject.
   */
  public String getX500UserSubject() throws GlobusCredentialException {
    DNConvert dnConvert = new DNConvert(this.proxy.getSubject().trim());
    return dnConvert.removeProxySuffix(DNConvert.X500);
  }


  /**
   * Returns the proxy user subject removing all occurencies of "CN=/Proxy" from
   * the specified proxySubject String.
   * @return proxy user subject.
   */
  public static String getX500UserSubject(String proxySubject) {
    DNConvert dnConvert = new DNConvert(proxySubject.trim());
    return dnConvert.removeProxySuffix(DNConvert.X500);
  }


  /**
   * Returns a Vector containing the names of all Virtual Organisations contained
   * in the proxy certificate extension if the proxy certificate is a VOMS proxy certificate.
   * @throws FileNotFoundException - Unable to find proxy certificate file.
   * @throws GlobusCredentialException - Unable to load proxy certificate.
   * @throws Exception
   * @return a String Vector with all proxy certificate Virtual Organisations, if the proxy is a VOMS proxy,
   *         an empty Vector if the proxy is not a VOMS proxy.
   */
  public Vector getVONames() throws FileNotFoundException, GlobusCredentialException, Exception {
    VOMSExtension extension = getProxyVOMSExtension();
    java.util.List infoList = extension.getVOMSInfos();
    Vector voVector = new Vector();
    VOMSInfo vomsInfo;
    for (int i = 0; i < infoList.size(); i++) {
      voVector.add(((VOMSInfo) infoList.get(i)).getVO());
    }
    return voVector;
  }
  /**
   * Returns the name of the default Virtual Organisation contained in the proxy
   * certificate if the proxy certificate is a VOMS proxy certificate. (the default
   * Virtual Organisation is the first in the VOMS extension).
   * @throws IOException - Unable to find proxy certificate file.
   * @throws GlobusCredentialException - Unable to load proxy certificate.
   * @throws Exception
   * @return the name of the default proxy certificate Virtual Organisation, if the proxy is a VOMS proxy,
   *         an empty String if the proxy is not a VOMS proxy.
   */
  public String getDefaultVOName() throws IOException, GlobusCredentialException, Exception {
    VOMSExtension extension = getProxyVOMSExtension();
    VOMSInfo vomsInfo = null;
    if (extension != null) {
      vomsInfo = extension.getDefaultVOMSInfo();
    }
    if (vomsInfo != null) {
      return vomsInfo.getVO();
    } else {
      return "";
    }
    //return (extension != null) ? extension.getDefaultVOMSInfo().getVO() : "";
  }
  /**
   * Returns a String Vector containing the names of all voName groups present
   * in the proxy certificate extension if the proxy certificate is a VOMS proxy certificate.
   * @throws IOException - Unable to find proxy certificate file.
   * @throws GlobusCredentialException - Unable to load proxy certificate.
   * @throws Exception
   * @param voName the name of the Virtual Organisation.
   * @return a Vector containing all voName groups, if the proxy is a VOMS proxy,
   *         an empty Vector if the proxy is not a VOMS proxy or no groups are present.
   */
  public Vector getGroups(String voName) throws IOException, GlobusCredentialException, Exception {
    VOMSExtension extension = getProxyVOMSExtension();
    java.util.List infoList = extension.getVOMSInfos();
    VOMSInfo vomsInfo;
    Vector groupVector = new Vector();
    for (int i = 0; i < infoList.size(); i++) {
      if (((VOMSInfo) infoList.get(i)).getVO().trim().equals(voName.trim())) {
        java.util.List groupsList = ((VOMSInfo) infoList.get(i)).getGroups();
        for (int j = 0; j < groupsList.size(); j++) {
          groupVector.add(groupsList.get(j));
        }
        break;
      }
    }
    return groupVector;
  }
  /**
   * Returns a String Vector containing the names of all default Virtual Organisation groups present
   * in the proxy certificate extension if the proxy certificate is a VOMS proxy certificate.
   * @throws IOException - Unable to find proxy certificate file.
   * @throws GlobusCredentialException - Unable to load proxy certificate.
   * @throws Exception
   * @return a Vector containing all default Virtual Organisation groups, if the proxy is a VOMS proxy,
   *         an empty Vector if the proxy is not a VOMS proxy or no groups are present.
   */
  public Vector getDefaultGroups() throws IOException, GlobusCredentialException, Exception {
    String defaultVO = getDefaultVOName();
    if (!defaultVO.equals("")) {
      return getGroups(getDefaultVOName());
    } else {
      return new Vector();
    }
  }
  /**
   * Returns a Boolean value indicating if the Virtual Organisation voName is present in the
   * VOMS proxy extension or not.
   * @throws IOException - Unable to find proxy certificate file.
   * @throws GlobusCredentialException - Unable to load proxy certificate.
   * @throws Exception
   * @param voName the name of the Virtual Organisation.
   * @return true, if the proxy is a VOMS proxy and voName is present,
   *         false otherwise.
   */
  public boolean containsVO(String voName) throws IOException, GlobusCredentialException, Exception {
    VOMSExtension extension = getProxyVOMSExtension();
    java.util.List infoList = extension.getVOMSInfos();
    for (int i = 0; i < infoList.size(); i++) {
      if (((VOMSInfo) infoList.get(i)).getVO().trim().equals(voName.trim())) {
        return true;
      }
    }
    return false;
  }
	/**
	* Returns a Boolean value indicating if the proxy certificate has a VOMS extension or not
	* (if the proxy is a VOMS proxy certificate or not).
	* @throws IOException - Unable to find proxy certificate file.
	* @throws GlobusCredentialException - Unable to load proxy certificate.
	* @throws Exception
	* @return true, if the proxy has a VOMS extension,
	*         false otherwise.
	*/
	public boolean hasVOMSExtension() throws IOException, GlobusCredentialException, Exception {
		VOMSExtension extension = getProxyVOMSExtension();
		if ((extension != null) && (extension.getDefaultVOMSInfo() != null)) return true;
		return false;
	}
	private VOMSExtension getProxyVOMSExtension() throws FileNotFoundException, GlobusCredentialException, Exception {
		VOMSExtension extension;
		String proxyName = "";
		try {
			if ((this.userProxy != null) && !this.proxy.equals(""))  proxyName = this.userProxy;
			else  proxyName = getDefaultProxyName();
			X509Certificate certificate = org.edg.security.info.CertUtil.loadCertificate(proxyName);
			extension = new VOMSExtension(certificate);
		} catch (IOException ioe) {
			throw new FileNotFoundException("Unable to find proxy certificate file: " + proxyName);
		} catch (java.security.cert.CertificateException ce) {
			throw new GlobusCredentialException(GlobusCredentialException.FAILURE,"Unable to load proxy certificate",new Throwable());
		}
		return extension;
	}
	// Private / Package memebrs:
	private GlobusCredential proxy= null ;
	private String userProxy, userCert, userKey, userDir ;
	private static String PROXY = "X509_USER_PROXY";
	private static String CERT = "X509_USER_CERT";
	private static String KEY = "X509_USER_KEY";
	private static String DIR = "X509_CERT_DIR";
	private static String HOME =  System.getProperty ("user.home") ;
	private static String SEP    =   System.getProperty ( "file.separator"  ) ;
	/**
	This is the default proxy as been se by the user  */
	private static String DEFAULT_PROXY_VAR ;
	/**  This is a final variable. It is just created for the first instance.
	It points to the static default proxy seen by grid-proxy-info  */
	private static final String DEFAULT_PROXY =
		/*if*/	(Api.getEnv (PROXY)==null) ?
		/*then**/	(SEP + "tmp" +SEP +"x509up_u" + Api.getUid()  ) :
		/*else*/	(Api.getEnv (PROXY)  );
	// private GridProxyProperties proxy= null ;
	private static final int  EXPIRATION_LIMIT = 100 ; //  The minimal expiration time allowed for a proxy certificate   (in seconds)
};
