#include <iostream>
#include <fstream>

#include <cppunit/TestResult.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/XmlOutputter.h>

#include "url_cu_suite.h"
#include "job_wrapper_cu_suite.h"

int main (int argc, char* argv[]) {
    //TODO
   // std::ofstream xml("./cppUnit_output.xml", ios::app);
    
    CppUnit::TestResult controller;
    CppUnit::TestResultCollector result;
    controller.addListener( &result );
    
    CppUnit::TestRunner runner;
    runner.addTest(Url_test::suite()); 
    runner.addTest(JobWrapper_test::suite()); 
//    runner.addTest();  a new test
    runner.run(controller);
 
  // to do: da sistemare output in xml
   // CppUnit::XmlOutputter xmlOutputter( &result, xml );
  CppUnit::TextOutputter textOutputter( &result, std::cerr ); 
    textOutputter.write();
    //xmlOutputter.write();
    return result.wasSuccessful() ? 0 : 1 ;
}