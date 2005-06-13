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
