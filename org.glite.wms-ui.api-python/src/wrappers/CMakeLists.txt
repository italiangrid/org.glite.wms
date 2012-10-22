
include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/interface
		    ${PROJECT_SOURCE_DIR}/src)



add_library(glite_wmsui_LbWrapper
	    SHARED 
  wm.h 
  soapC.cpp 
  soapClient.cpp 
  wmproxy_api.cpp 
  wmproxy_api_utilities.cpp 
  )

add_library(glite_wmsui_LogWrapper
            SHARED

add_library(glite_wmsui_UcWrapper
            SHARED

add_library(glite_wmsui_AdWrapper
	    SHARED

set_target_properties(glite_wmsui_LbWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_LogWrapper PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_UcWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_AdWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)



install(TARGETS glite_wmsui_LbWrapper 
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

install(TARGETS glite_wmsui_LogWrapper
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

install(TARGETS glite_wmsui_UcWrapper
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

install(TARGETS glite_wmsui_AdWrapper
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )



add_subdirectory(pkgconfig)
