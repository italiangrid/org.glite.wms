
#include <string>
#include <iostream>


#include "gaclexception.h"



// Gridsite C library
extern "C" {
#include "gridsite.h"
 }

// permission codes
#define WMPGACL_NOPERM 	GRST_PERM_NONE
#define WMPGACL_READ		GRST_PERM_READ
#define WMPGACL_EXEC		GRST_PERM_EXEC
#define WMPGACL_LIST		GRST_PERM_LIST
#define WMPGACL_WRITE		GRST_PERM_WRITE
#define WMPGACL_ADMIN		GRST_PERM_ADMIN

/*
#define WMPGACL_ANYUSER_TYPE	0
#define WMPGACL_PERSON_TYPE		1
#define WMPGACL_DNLIST_TYPE		2
*/

#define WMPGACL_ANYUSER_CRED	"any-user"
#define WMPGACL_PERSON_CRED		"person"
#define WMPGACL_DNLIST_CRED		"dn-list"

#define WMPGACL_ANYUSER_TAG		"any-user"
#define WMPGACL_PERSON_TAG		"dn"
#define WMPGACL_DNLIST_TAG		"dn-list"

#define WMPGACL_DEFAULT_FILE		".gacl"

#define WMPGACL_SUCCESS		0
#define WMPGACL_ERROR		-1

typedef GRSTgaclPerm WMPgaclPerm ;

typedef enum{
			WMPGACL_UNDEFCRED_TYPE,
			WMPGACL_ANYUSER_TYPE,
			WMPGACL_PERSON_TYPE,
			 WMPGACL_DNLIST_TYPE } WMPgaclCredType;

class GaclManager {

 public:

 	GaclManager ( );

	GaclManager (std::string file, WMPgaclCredType type, std::string dn, bool unset_perm = false, bool create = false);

	int checkAllowPermission (WMPgaclPerm permission) ;

	int allowPermission( WMPgaclPerm permission );

	int denyPermission(WMPgaclPerm permission );

	int loadNewCredential ( std::string file, WMPgaclCredType type, std::string dn, bool unset_perm = false, bool create = false) ;

	int saveGacl ( std::string gaclFile ) ;

	int saveGacl (  ) ;

	void printGacl ( ) ;

	bool gaclExists (std::string file);

private :

	void loadFromFile (std::string gaclFile) ;

	void newGacl () ;

	void newCredential ( ) ;

	int loadCredential (bool unset_perm =false ) ;

	GRSTgaclAcl *gaclAcl  ;

	GRSTgaclEntry *gaclEntry;

	GRSTgaclCred *gaclCred  ;

	GRSTgaclUser *gaclUser ;

	WMPgaclPerm  gaclAllowed ;

	WMPgaclPerm  gaclDenied ;

	WMPgaclPerm  gaclPerm ;

	std::string gaclFile ;

	std::string credType;

	std::string rawName ;

	std::string rawValue ;





 };

 
