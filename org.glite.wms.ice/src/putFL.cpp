/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
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
 * ICE main daemon
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileLocker.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "classad_distribution.h"
#include "iceConfManager.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <exception>
//#include <fstream>
#include <sys/time.h>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
//#include <boost/algorithm/string.hpp>

namespace utils = glite::wms::common::utilities;
namespace iceUtil = glite::wms::ice::util;
using namespace std;
namespace po = boost::program_options;

int getRandom(double);

  /**
   * Usage: putFL <filelist> <jdlfile>
   *
   */

int main(int argc, char* argv[]) {

    string opt_conf_file;
    string opt_ad;

    po::positional_options_description p;
    p.add("ad", -1);

    po::options_description desc("Usage");
    desc.add_options()
        ("help", "display this help and exit")
        (
         "conf",
         po::value<string>(&opt_conf_file)->default_value("glite_wms.conf"),
         "configuration file"
         )
        (
         "noproc", "Do not process the input classad (put it as-is in ICE filelist)"
         )
        ( 
         "ad",
         po::value<string>(&opt_ad)->default_value("ad.jdl"),
         "ClassAd file to put into the filelist"
         )
        ;
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(p).run(), vm);
    } catch( std::exception& ex ) {
        cerr << "There was an error parsing the command line. "
             << "Error was: " << ex.what() << endl
             << "Type " << argv[0] 
             << " --help for the list of available options"
             << endl;
        exit( 1 );
    }
    po::notify(vm);

    if ( vm.count("help") ) {
        cout << desc << endl;
        return 0;
    }
    
    //
    // Init ICE conf file
    iceUtil::iceConfManager::init( opt_conf_file );
    try{
        iceUtil::iceConfManager::getInstance();
    }
    catch(iceUtil::ConfigurationManager_ex& ex) {
        cerr << "putFL::main() - ERROR: " << ex.what() << endl;
        exit(1);
    }
    
    // int j, howmany;
    utils::FileList<string> fl;
    
    ifstream is( opt_ad.c_str() );
    string a_line;
    string Buf;

    while(is.peek()!=EOF) {
        std::getline(is, a_line, '\n');
        Buf += a_line;
    }

    is.close();

    string request; // request to put in the filelist

    if ( vm.count("noproc") ) {
        request = Buf;
    } else {        
        classad::ClassAdParser parser;
        classad::ClassAd *ad = parser.ParseClassAd( Buf.c_str() );
        
        if(!ad) {
            cerr << "Error parsing classad: " << Buf <<endl;
            exit(1);
        }
        struct timeval T;
        gettimeofday(&T, 0);
        
        ad->InsertAttr("edg_jobid", boost::str( boost::format( "https://grid005.pd.infn.it:9000/000%1%.%2%" ) % time(NULL) % T.tv_usec ) );
        
        classad::ClassAdUnParser unp;
        
        Buf.clear();
        unp.Unparse(Buf, ad);
        
        request = "[arguments = [ jobad = " + Buf
            + " ]; command = \"Submit\"; protocol = \"1.0.0\" ]";
    }


    string filelist_name( iceUtil::iceConfManager::getInstance()->getICEInputFile() );

    cout << "Adding JDL <" << request << "> to filelist " << filelist_name << endl;

    try{
        fl.open( filelist_name );
    }
    catch(std::exception& ex) {
        cerr << ex.what()<<endl;
        exit(1);
    }

    utils::FileListMutex mx(fl);
    utils::FileListLock  lock(mx);
    try {
        fl.push_back(request);
    } catch(std::exception& ex) {
        cerr << ex.what() << endl;
        exit(1);
    }
}


//______________________________________________________________________________
int getRandom(double max) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand((unsigned int)tv.tv_usec);
  int j= 1 + (int)(max*rand()/(RAND_MAX+1.0));
  return j;
}
