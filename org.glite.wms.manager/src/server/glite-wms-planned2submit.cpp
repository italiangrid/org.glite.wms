// File: glite-wl-planned2submit.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <iostream>
#include <fstream>
#include <boost/scoped_ptr.hpp>
#include <classad_distribution.h>
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/jdl/convert.h"
#include "SubmitAdapter.h"
#include  "glite/wms/common/configuration/Configuration.h"

namespace jobcontrol = glite::wms::jobsubmission::controller;
namespace requestad = glite::wms::jdl;
namespace configuration = glite::wms::common::configuration;
namespace utilities = glite::wms::common::utilities;

namespace {

std::string program_name;

void usage(std::ostream& os)
{
  os
    << "Usage: " << program_name << " --help\n"
    << "   or: " << program_name << " <sequence code> SOURCE DEST\n";
}

} // {anonymous}

int
main(int argc, char* argv[])
try {
  program_name = argv[0];

  if (argc == 2 && std::string(argv[1]) == "--help") {

    usage(std::cout);
    return EXIT_SUCCESS;

  } else if (argc != 4) {

    usage(std::cerr);
    return EXIT_FAILURE;

  }

  configuration::Configuration config("glite_wl.conf",
                                      configuration::ModuleType::workload_manager);

  std::string sequence_code(argv[1]);
  std::string input_file(argv[2]);
  std::ifstream is(input_file.c_str());
  if (!is) {
    std::cerr << "Cannot open input file " << input_file << "\n";
    return EXIT_FAILURE;
  }
  std::string output_file(argv[3]);
  std::ofstream os(output_file.c_str());
  if (!os) {
    std::cerr << "Cannot open output file " << output_file << "\n";
    return EXIT_FAILURE;
  }

  boost::scoped_ptr<classad::ClassAd> input_ad(utilities::parse_classad(is));

  jobcontrol::SubmitAdapter sad(*input_ad);

  boost::scoped_ptr<classad::ClassAd> submit_ad(
    sad.adapt_for_submission(sequence_code)
  );

  if (!submit_ad) {
    std::cerr << "SubmitAdapter::adapt_for_submission() failed\n";
    return EXIT_FAILURE;
  }
  requestad::to_submit_stream(os, *submit_ad);

} catch (std::exception& e) {
  std::cerr << e.what() << "\n";
  return EXIT_FAILURE;
} catch (...) {
  std::cerr << "Caught generic exception\n";
  return EXIT_FAILURE;
}
