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
using namespace CppUnit;

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
  CPPUNIT_ASSERT( wm_real!=NULL );
}

void
WMReal_test::test_cancel()
{
  CPPUNIT_ASSERT( wm_real!=NULL );
}

