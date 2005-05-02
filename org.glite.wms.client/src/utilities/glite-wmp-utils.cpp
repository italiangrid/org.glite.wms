/*
 * edg-wl-job-submit.cpp
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

#include "excman.h"
#include "utils.h"
#include "adutils.h"
#include <iostream>
using namespace std ;
using namespace glite::wms::client::utilities;

Utils utils;

void answer(){
	// ANSWER
	utils.answerYes("ciao secco ti chiami ale false?", false);
}

void resolve(){
	// RESOLVE
	string resolved;
	utils.resolveHost("http://matrix",resolved);
}

void getLbs(){
	// GET LBS
	std::vector<std::vector<std::string> > lbGroup;
	string lbBase="http://lbaddress:1234";
	for (int i=0; i< 4; i++){
		// cout << "   ***   LB =" << i << endl ;
		lbBase+="N#";
		std::vector<std::string> lbs;
		for (int j=0; j<3; j++){
			lbBase+="X_";
			lbs.push_back(lbBase);
			// cout << "Added lbs: " << lbBase << endl ;
		}
		lbGroup.push_back(lbs);
	}
	std::vector<std::string> lbResult=utils.getLbs(lbGroup,2);
	for (unsigned int i=0;i<lbResult.size();i++) cout << "LB: "<<lbResult[i]<< endl ;
}

void checkLBNS(const string tc){
	cout << endl<<"=======>" << tc << endl ;
	std::pair <std::string, unsigned int> ad = utils.checkLb(tc);
	cout << "found LB: "<< ad.first << "     :      " << ad.second << endl ;
	ad = utils.checkNs(tc);
	cout << "found NS: "<< ad.first << "     :      " << ad.second << endl ;
}

int main(int argc,char *argv[]){
	WMS_EXCM_TRY()
		utils.jobAdExample();
		answer();
		resolve();
	WMS_EXCM_CATCH(WMS_ERROR)
	WMS_EXCM_TRY()
		getLbs();
		checkLBNS("https://address:123456");
		checkLBNS("://address");
	WMS_EXCM_CATCH(WMS_ERROR)
	return 0;
}
