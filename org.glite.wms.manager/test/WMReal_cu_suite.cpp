/*
  Copyright (c) 2004 on behalf of the EU EGEE Project:
  The European Organization for Nuclear Research (CERN),
  Istituto Nazionale di Fisica Nucleare (INFN), Italy
  Datamat Spa, Italy
  Centre National de la Recherche Scientifique (CNRS), France
  CS Systeme d'Information (CSSI), France
  Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
  Universiteit van Amsterdam (UvA), Netherlands
  University of Helsinki (UH.HIP), Finland
  University of Bergen (UiB), Norway
  Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
  
  WMReal suite
  
  Authors: Paolo Andreetto <paolo.andreetto@pd.infn.it>
  
  Version info: $Id$
  Release: $Name$
  
 */

#include "WMReal_cu_suite.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

using namespace CppUnit;
using namespace classad;

void
WMReal_test::setUp()
{
  wm_real = new WMReal();
}

void
WMReal_test::tearDown()
{
  if (wm_real) {
    delete wm_real;
  }
}

void
WMReal_test::test_submit()
{
  std::string key1("edg_jobid");
  std::string value1("https://lxb1420.cern.ch:9000/BqXQcCpXqnF92nvj94PZVw");


  ClassAd *classad =  new ClassAd();
  CPPUNIT_ASSERT( classad );

  classad->InsertAttr(key1,value1);

  try{
    wm_real->submit(classad);
  }catch(glite::wms::jdl::CannotGetAttribute att_error){
    cerr << "Cannot retrieve " << att_error.reason() << endl;
  }

  CPPUNIT_ASSERT( wm_real!=NULL );

  delete classad;
}

void
WMReal_test::test_cancel()
{
  CPPUNIT_ASSERT( wm_real!=NULL );
}

