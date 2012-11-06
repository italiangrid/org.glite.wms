/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/***************************************************************************
* filename : BrokerCLI.cpp
* authors : Livio Salconi <livio.salconi@pi.infn.it>
* copyright : (C) 2001 by INFN
***************************************************************************/

// $Id: BrokerCLI.C,v 1.6.2.2 2010/04/07 12:53:50 mcecchi Exp $
#include <iostream>

#include "BrokerInfo.h"

class BrokerCLI {

  private:
    BrokerInfo* binfo_;
    int vflag;
    void testArgs(std::vector<std::string> lineparam, int minargs, std::string shelp);
    void vDisplay(std::string label, std::vector<std::string>& vOut);
    void sDisplay(std::string label, std::string sOut);
    void vPipeDisplay(std::vector<std::string>& vOut);
    void sPipeDisplay(std::string sOut);
    void biTest(BI_Result bires);

  public:
    BrokerCLI(void);
    void Run(std::vector<std::string> lineparam, int vflag);
};

BrokerCLI::BrokerCLI(void) {

  binfo_ = BrokerInfo::instance();
}

void BrokerCLI::sDisplay(std::string label, std::string sOut) {

  std::cout << label << std::endl;
  std::cout << " -> " << sOut << std::endl;
}

void BrokerCLI::vDisplay(std::string label, std::vector<std::string>& vOut) {

  std::cout << label << std::endl;
  if ( vOut.size() != 0 ) {
    for(int i = 0; i < vOut.size(); i++) {
       std::cout << " -> " << vOut[i] << std::endl;
    }
  }
  else {
    std::cout << " ... not found!" << std::endl;
  }
}

void BrokerCLI::sPipeDisplay(std::string sOut) {

   std::cout << sOut << std::endl;
}

void BrokerCLI::vPipeDisplay(std::vector<std::string>& vOut) {

  if ( vOut.size() != 0 ) {
    for(int i = 0; i < vOut.size(); i++) {
      std::cout << vOut[i] << std::endl;
    }
  }
}

void BrokerCLI::biTest(BI_Result bires) {

  if(bires == BI_SUCCESS) {
    std::cout << " -> BI_SUCCESS" << std::endl;
  }
  else {
    std::cout << " -> BI_ERROR" << std::endl;
  }
}

void BrokerCLI::testArgs(std::vector<std::string> lineparam, int minargs, std::string shelp) {

  if ((lineparam.size() - 1) != minargs) {
    std::cout << "... error parsing arguments!" << std::endl;
    std::cout << "Use: " << shelp << std::endl;
    exit(-1);
  }
}

void BrokerCLI::Run(std::vector<std::string> lineparam, int vflag) {

  BI_Result bires = BI_ERROR;
  std::vector<std::string> vOut;
  std::string sOut1 = "";
  std::string sOut2 = "";
  int rcommand = 0;

  vOut.clear();

  // list BrokerInfo file name
  if(vflag == 1) {
    std::cout << "BrokerInfo::getBIFileName(): " << binfo_->getBIFileName() << std::endl;
  }
  // BrokerInfo::getCE()
  if(lineparam[0].compare("getCE") == 0) {
    rcommand = 1;
    testArgs(lineparam, 0, " getCE");
    bires = binfo_->getCE(sOut1);
    if(vflag != 0) {
      sDisplay("BrokerInfo::getCE(): ", sOut1);
      biTest(bires);
    } else {
      sPipeDisplay(sOut1);
    }
  }
  // BrokerInfo::getInputData()
  if(lineparam[0].compare("getInputData") == 0) {
    rcommand = 1;
    testArgs(lineparam, 0, " getInputData");
    bires = binfo_->getInputData(vOut);
    if(vflag != 0) {
      vDisplay("BrokerInfo::getInputData(): ", vOut);
      biTest(bires);
    } else {
      vPipeDisplay(vOut);
    }
  }
  //BrokerInfo::getDataAccessProtocol()
  if(lineparam[0].compare("getDataAccessProtocol") == 0) {
    rcommand = 1;
    testArgs(lineparam, 0, " getDataAccessProtocol");
    bires = binfo_->getDataAccessProtocol(vOut);
    if(vflag != 0) {
      vDisplay("BrokerInfo::getDataAccessProtocol(): ", vOut);
      biTest(bires);
    } else {
      vPipeDisplay(vOut);
    }
  }
  //BrokerInfo::getSEs
  if(lineparam[0].compare("getSEs") == 0) {
    rcommand = 1;
    testArgs(lineparam, 0, " getSEs");
    bires = binfo_->getSEs(vOut);
    if(vflag != 0){
      vDisplay("BrokerInfo::getSEs(): ", vOut);
      biTest(bires);
    } else {
      vPipeDisplay(vOut);
    }
  }
  //BrokerInfo::getCloseSEs()
  if(lineparam[0].compare("getCloseSEs") == 0) {
    rcommand = 1;
    testArgs(lineparam, 0, " getCloseSEs");
    bires = binfo_->getCloseSEs(vOut);
    if(vflag != 0) {
      vDisplay("BrokerInfo::getCloseSEs(): ", vOut);
      biTest(bires);
    } else {
      vPipeDisplay(vOut);
    }
  }
  //BrokerInfo::getSEMountPoint()
  if(lineparam[0].compare("getSEMountPoint") == 0) {
    rcommand = 1;
    testArgs(lineparam, 1, " getSEMountPoint <SE>");
    bires = binfo_->getSEMountPoint(lineparam[1], sOut1);
    if(vflag != 0) {
      sDisplay("BrokerInfo::getSEMountPoint(" + lineparam[1] + "): ", sOut1);
      biTest(bires);
    } else {
      sPipeDisplay(sOut1);
    }
  }
  //BrokerInfo::getSEFreeSpace()
  if(lineparam[0].compare("getSEFreeSpace") == 0) {
    rcommand = 1;
    testArgs(lineparam, 1, " getSEFreeSpace <SE>");
    bires = binfo_->getSEFreeSpace(lineparam[1], sOut1);
    if(vflag != 0) {
      sDisplay("BrokerInfo::getSEFreeSpace(" + lineparam[1] + "): ", sOut1);
      biTest(bires);
    } else {
      sPipeDisplay(sOut1);
    }
  }
  //BrokerInfo::getLFN2SFN()
  if(lineparam[0].compare("getLFN2SFN") == 0) {
    rcommand = 1;
    testArgs(lineparam, 1, " getLFN2SFN <PFN>");
    bires = binfo_->getLFN2SFN(lineparam[1], vOut);
    if(vflag != 0) {
      vDisplay("BrokerInfo::getLFN2SFN(" + lineparam[1] + "): ", vOut);
      biTest(bires);
    } else {
      vPipeDisplay(vOut);
    }
  }
  //BrokerInfo::getSEProtocol()
  if(lineparam[0].compare("getSEProtocols") == 0) {
    rcommand = 1;
    testArgs(lineparam, 1, " getSEProtocols <SE>");
    bires = binfo_->getSEProtocols(lineparam[1], vOut);
    if(vflag != 0) {
      vDisplay("BrokerInfo::getSEProtocols(" + lineparam[1] + "): ", vOut);
      biTest(bires);
    } else {
      vPipeDisplay(vOut);
    }
  }
  //BrokerInfo::getSEPort()
  if(lineparam[0].compare("getSEPort") == 0) {
    rcommand = 1;
    testArgs(lineparam, 2, " getSEPort <SE> <Protocol>");
    bires = binfo_->getSEPort(lineparam[1], lineparam[2], sOut1);
    if(vflag != 0) {
      sDisplay("BrokerInfo::getSEPort(" + lineparam[1] + ", " + lineparam[2] + "): ", sOut1);
      biTest(bires);
    } else {
      sPipeDisplay(sOut1);
    }
  }
  // BrokerInfo::getVirtualOrganization()
  if(lineparam[0].compare("getVirtualOrganization") == 0) {
    rcommand = 1;
    testArgs(lineparam, 0, " getVirtualOrganization");
    bires = binfo_->getVirtualOrganization(sOut1);
    if(vflag != 0) {
      sDisplay("BrokerInfo::getVirtualOrganization: ", sOut1);
      biTest(bires);
    } else {
      sPipeDisplay(sOut1);
    }
  }
	// not recognized command
  if (rcommand == 0) {
    std::cout << std::endl << lineparam[0] << " is not a valid command!" << std::endl << std::endl;
  }
}

int main(int argc, char** argv) {

  std::vector<std::string> lineparam;
  std::string bifile = "";
  int vflag = 0;

  if(argc < 2) {
    std::cout << std::endl << argv[0] << ": please specify a function with parameters" << std::endl;
    std::cout << "use " << argv[0] << " --help for functions and parameters list" << std::endl << std::endl;
  }
  else {
    lineparam.clear();
    for(int i = 1 ; i < argc; i++) {
      int flag = 0;
      if(((std::string)argv[i]).compare("-v") == 0) {
        flag = 1;
        vflag = 1;
      }
      if(((std::string)argv[i]).compare("-f") == 0) {
        flag = 1;
        i++;
	if(i < argc){
         bifile = argv[i];
	}
      }
      if(flag == 0) {
        lineparam.push_back(argv[i]);
      }
    }
    if(lineparam.size()==0){
     std::cout << std::endl << argv[0] << ": please specify a function with parameters" << std::endl;
     std::cout << "use " << argv[0] << " --help for functions and parameters list" << std::endl << std::endl;
    }
    else if(lineparam[0].compare("--help") == 0) {
      // help text
      std::cout << std::endl << "Use: # " << argv[0] << " [-v] [-f filename] function [parameter] [parameter] ..." << std::endl;
      std::cout << "-v : for verbose, formatted output" << std::endl;
      std::cout << "-f : for specify a brokerinfo file" << std::endl << std::endl;
      std::cout << "supported funtions are:" << std::endl;
      std::cout << argv[0] << " getCE" << std::endl;
      std::cout << argv[0] << " getDataAccessProtocol" << std::endl;
      std::cout << argv[0] << " getInputData" << std::endl;
      std::cout << argv[0] << " getSEs" << std::endl;
      std::cout << argv[0] << " getCloseSEs" << std::endl;
      std::cout << argv[0] << " getSEMountPoint <SE>" << std::endl;
      std::cout << argv[0] << " getSEFreeSpace <SE>" << std::endl;
      std::cout << argv[0] << " getLFN2SFN <LFN>" << std::endl;
      std::cout << argv[0] << " getSEProtocols <SE>" << std::endl;
      std::cout << argv[0] << " getSEPort <SE> <Protocol>" << std::endl;
      std::cout << argv[0] << " getVirtualOrganization" << std::endl;
      std::cout << std::endl;
    }
    else {
      if(bifile.compare("") != 0) {
        setenv("GLITE_WMS_RB_BROKERINFO", bifile.c_str(), 1);
      }
      BrokerCLI bicli;
      bicli.Run(lineparam, vflag);
    }
  }
}
