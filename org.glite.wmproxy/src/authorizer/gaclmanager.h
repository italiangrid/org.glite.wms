

#include <iostream>
#include <string>

#include "gaclexception.h"

// Gridsite C library
extern "C" {
#include "gridsite.h"
 }

// permission strings
#define GACLMGR_PERM_NONE 	"none"
#define GACLMGR_PERM_READ 	"read"
#define GACLMGR_PERM_EXEC		"exec"
#define GACLMGR_PERM_LIST		"list"
#define GACLMGR_PERM_WRITE	"write"
#define GACLMGR_PERM_ADMIN	"admin"

// permission codes
#define GACLMGR_NONE 			0
#define GACLMGR_READ 			1
#define GACLMGR_EXEC			2
#define GACLMGR_LIST			3
#define GACLMGR_WRITE			4
#define GACLMGR_ADMIN			5

#define GACLMGR_NUMDEF_CODES	6

#define GACLMGR_ANYUSER		"any-user"

#define DEFAULT_GACL_FILE		".gacl"

#define GACLMGR_SUCCESS	0
#define GACLMGR_ERROR		-1

class GaclManager {

 public:

	GaclManager ( );

	GaclManager ( std::string gaclFile, bool create = true );

	int getPermissionCode (std::string permission);

	int checkAllowPermission (std::string user, std::string permission) ;

	int checkAllowReadForAnyUser ( );

	int checkAllowWriteForAnyUser ( );

	int checkAllowExecForAnyUser ( );

	int checkAllowListForAnyUser ( );

	int allowPermission(std::string user, std::string permission, std::string rawname = "",  std::string rawvalue = "" );

	int denyPermission(std::string user, std::string permission, std::string rawname = "",  std::string rawvalue = "" );


	int allowReadToAnyUser  (  ) ;
	int allowWriteToAnyUser  (  ) ;
	int allowExecToAnyUser  (  ) ;
	int allowListToAnyUser  (  ) ;

	int denyReadToAnyUser  (  ) ;
	int denyWriteToAnyUser  (  ) ;
	int denyExecToAnyUser  (  ) ;
	int denyListToAnyUser  (  ) ;


	int saveGacl ( std::string gaclFile ) ;

	void printGacl ( ) ;
private :

	GRSTgaclAcl *gaclAcl  ;
	GRSTgaclEntry *gaclEntry;
	GRSTgaclCred *gaclCred  ;
	GRSTgaclUser *gaclUser ;
	GRSTgaclPerm  gaclAllowed ;
	GRSTgaclPerm  gaclDenied ;
	GRSTgaclPerm  gaclPerm ;
	std::string gaclFile ;
	/*
	GRSTgaclEntry *anyUserEntry;
	GRSTgaclCred *anyUserCred  ;
	GRSTgaclUser *anyUser ;
	GRSTgaclPerm  anyUserAllowed ;
	GRSTgaclPerm  anyUserDenied ;
	GRSTgaclPerm  anyUserPerm ;
	*/
	bool gaclExists (std::string file);

	void loadFromFile (std::string gaclFile) ;

	void newUserCredential ( std::string username, std::string rawname = "",  std::string rawvalue = "" ) ;

	int loadUserCredential (std::string user, std::string rawname = "",  std::string rawvalue = "" ) ;



 };

 
