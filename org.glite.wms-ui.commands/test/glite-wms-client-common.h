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

// JDL:
#include "glite/jdl/ExpDagAd.h"
#include "glite/jdl/JobAd.h"
#include "glite/jdl/NodeAd.h"
#include "glite/jdl/collectionad.h"
#include "glite/jdl/extractfiles.h"

// CPP UNIT:
#include "cppunit/TestRunner.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"
#include "cppunit/TextOutputter.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/extensions/HelperMacros.h"
//BOOST
#include <boost/lexical_cast.hpp>

#define DEF_REQUIREMENTS "other.GlueCEPolicyMaxCPUTime >= 6"
#define DEF_RANK "other.GlueCEStateFreeCPUs"
#define DEF_EXECUTABLE "/bin/date"
#define DEF_VO "egee"
#define DEF_ARGUMENTS "--some-arg"

#define DEF_NODES_NUMBER 3



void setJdlFile(const std::string& jdlFile);
std::string getJdlFile();

// debug mode
void setWCDM(bool value);
std::string sep();
void title(const std::string &title);
void toBCopiedFileList(const std::string &wmp_uri, const std::string &isb_uri,
	const std::vector <std::string> &paths, std::vector <std::pair<std::string, std::string> > to_bcopied);
std::string printExtracted(glite::jdl::ExtractedAd *extractedAd, unsigned int offs=0);
std::string printWarnings(std::vector<std::string>);
// void inherit( Ad &jab)
/********
DAGAD
********/
void dagClientSide(glite::jdl::ExpDagAd &dagad);
void dagServerSide(glite::jdl::ExpDagAd &dagad);
std::vector< std::string> insertJobIds (glite::jdl::ExpDagAd &dagad);


/********
JOBAd
********/
glite::jdl::Ad createAd();
glite::jdl::JobAd createJobAd();
glite::jdl::ExpDagAd createDagAd();
glite::jdl::CollectionAd createCollectionAd();

void jobClientSide(glite::jdl::JobAd &dagad);
void jobServerSide(glite::jdl::JobAd &dagad);





// endline

