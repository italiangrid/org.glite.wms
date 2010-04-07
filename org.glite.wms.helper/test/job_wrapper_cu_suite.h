/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://project.eu-egee.org/partners for details on the
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

#ifndef JOB_WRAPPER_CU_SUTE_H
#define JOB_WRAPPER_CU_SUTE_H

#include "jobadapter/jobwrapper/JobWrapper.h"
#include <cppunit/extensions/HelperMacros.h>
#include <string>

class JobWrapper_test: public CppUnit::TestFixture {
   
   
    CPPUNIT_TEST_SUITE(JobWrapper_test);
        CPPUNIT_TEST(get_set_methods_case);
//        CPPUNIT_TEST();
//        CPPUNIT_TEST();        
    CPPUNIT_TEST_SUITE_END();   

public:    
    void setUp();
    void tearDown();
    void get_set_methods_case();
    
};


#endif // JOB_WRAPPER_CU_SUTE_H
