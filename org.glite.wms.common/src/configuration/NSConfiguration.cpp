/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://project.eu-egee.org/index.php?id=115 for details on the
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

#include "NSConfiguration.h"

namespace glite {
namespace wms {
namespace common {
namespace configuration {

NSConfiguration::NSConfiguration( const classad::ClassAd *ad ) : confbase_c( ad )
{}

NSConfiguration::~NSConfiguration( void )
{}

} // configuration namespace
} // common namespace
} // wms namespace
} // glite namespace

