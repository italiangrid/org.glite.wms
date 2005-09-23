import org.glite.wmsui.apij.* ;
import org.glite.security.util.DNHandler ;  // DN X509 expression handler
class GliteUserCredential {
	private static void testProxy (  UserCredential uc  ) throws Exception{


		String strangeX509 = "C=IT/O=INFN/OU=Personal Certificate/L=DATAMAT DSAGRD/CN=Alessandro Maraschini/Email=alessandro.maraschini@datamat.it/CN=proxy";

		System.out.println ("\n\t\tREADING PROXY: "+uc.getUserProxy ()  ) ;
		System.out.println (uc.getDefaultProxyName ());
		System.out.println (uc.getDefaultCert ());
		System.out.println (uc.getTimeLeft());
		System.out.println ("Subject: " +uc.getSubject()  );
		System.out.println ("Issuer: "  +uc.getIssuer()  );



		System.out.println ("DefaultPR --> " + uc.getDefaultProxyName ());
		System.out.println ( " getDefaultProxy --> " + uc.getDefaultProxy()      ) ;
		System.out.println ( "getDefaultCert  --> " + uc.getDefaultCert()      )  ;
		System.out.println ( " getDefaultKey () --> " + uc.getDefaultKey ()      )  ;
		System.out.println ( " getDefaultDir () --> " + uc.getDefaultDir ()      )  ;
		System.out.println ( " getCredType    ( )  --> " + uc. getCredType    ( )      )  ;
		System.out.println ( " getX500UserSubject() --> " + uc.getX500UserSubject()   )  ;
		System.out.println ( " getX500UserSubject(Subject) --> " + uc.getX500UserSubject(uc.getSubject())   )  ;
		// System.out.println ( " getX500UserSubject(strange) --> " + uc.getX500UserSubject(strangeX509)   )  ;
		// System.out.println ( " getX500UserSubject(itself) --> " + uc.getX500UserSubject(uc.getX500UserSubject())   )  ;

		//          VOMS IMPLEMENTATION

		System.out.println ( "\n\t\tVOMS IMPLEMENTATION \n");
		System.out.println ( " getVONames() --> " + uc. getVONames()     )  ;
		System.out.println ( " getDefaultVOName() --> " + uc. getDefaultVOName()     )  ;
		System.out.println ( " getGroups(String voName) --> " + uc.getGroups("voName")      )  ;
		System.out.println ( " getDefaultGroups() --> " + uc.getDefaultGroups()      )  ;
		System.out.println ( " containsVO(pippo) --> " + uc.containsVO("pippo")      )  ;
		System.out.println ( " containsVO(testVO) --> " + uc.containsVO("testVO")      )  ;
		System.out.println ( " containsVO(EGEE) --> " + uc.containsVO("EGEE")      )  ;
		System.out.println ( "hasVOMSExtension()  --> " + uc. hasVOMSExtension()     ) ;
		System.out.println ( "\n\t\t################## \n");
	}



	public static void testHandler() {
	String subject = "C=IT,O=INFN,OU=Personal Certificate,L=DATAMAT DSAGRD,CN=Alessandro Maraschini,E=alessandro.maraschini@datamat.it,CN=proxy";

	subject = DNHandler.getDN(subject).getX500()  ;
	       System.out.println( subject );
	subject = DNHandler.getDN(subject).getX500()  ;

}

	public static void main(String[] args)   throws Exception {
		System.out.println ("Creating credential....");
		UserCredential uc = new UserCredential() ;
		testProxy (uc);
		for (int i = 0 ; i< args.length ; i++) {
			uc.setProxy ( new java.io.File(args[i]) );
			testProxy (uc);
		}	
	}
}
