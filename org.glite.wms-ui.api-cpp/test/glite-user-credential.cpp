/*
 * edg-wl-job-submit.cpp
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */


#include "glite/wmsutils/exception/Exception.h"
#include "glite/wmsui/api/UserCredential.h"
#include <iostream>
using namespace std ;

int main(int argc,char *argv[]){

try{
	if (argc<1){
		cout << "Usage : " << argv[0] << " [proxyfile]"<< endl  ;
		return 1;
	}
	string voName  = "EGEE" ;
	glite::wmsui::api::UserCredential uc ;
	cout << "Checking proxy (time Left)..."<< uc.checkProxy () << endl;
	cout << "Default Vo Name: " <<  uc.getDefaultVoName ()<< endl;	
	cout << "Issuer: " << uc.getIssuer () << endl;
	cout << "Subject: "<< uc.getSubject () << endl;
	// cout << "cred Type: "<< uc.getCredType ( )<< endl;  DEPRECATED
	// cout << "Strenght: "<< uc.getStrenght ( )<< endl;   DEPRECATED
	cout << "time Left:" << uc.getTimeLeft ( )<< endl;
	cout << "(Not destroying!)" << endl ; /*uc.destroy ( )*/
	cout << "Contains vo EGEE?" << uc.containsVo (voName)<< endl;

/** ADDITIONAL INFO:
std::vector< std::string >
getVoNames ()
std::vector< std::string >
getGroups (const std::string &voName)
std::vector< std::string >
getDefaultGroups ()
**/

   return 0;
 }catch  (glite::wmsutils::exception::Exception &exc){
         cout << "\nOperation not performed\n" ;
         cout <<  exc.printStackTrace();
}catch  (std::exception &exc){
         cout << "\nOperation not performed. std::exception found:\n" ;
         cout <<  exc.what() << endl ;
}

return 1 ;
};
