
#add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/wm.h
#  DEPENDS /var/lib/delegation-interface/interface/www.gridsite.org-delegation-2.0.0.wsdl
#  COMMAND wsdl2h -o ${PROJECT_SOURCE_DIR}/src/wm.h 
#          -I/usr/include/gsoap 
#          -t ${PROJECT_SOURCE_DIR}/src/wmptypemap.dat 
#	  ${PROJECT_SOURCE_DIR}/src/WMProxy.wsdl
#	  )



add_custom_command(
  OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/soapC.cpp 
  ${PROJECT_SOURCE_DIR}/src/soapClient.cpp
  DEPENDS /var/lib/delegation-interface/interface/www.gridsite.org-delegation-2.0.0.wsdl
  COMMAND
  wsdl2h -o ${PROJECT_SOURCE_DIR}/src/wm.h 
         -I/usr/include/gsoap 
         -t ${PROJECT_SOURCE_DIR}/src/wmptypemap.dat 
         ${PROJECT_SOURCE_DIR}/src/WMProxy.wsdl
  COMMAND 
  soapcpp2 ${PROJECT_SOURCE_DIR}/src/wm.h
  )


add_custom_target(GENSTUBS ALL DEPENDS 
  ${PROJECT_SOURCE_DIR}/src/soapC.cpp 
  ${PROJECT_SOURCE_DIR}/src/soapClient.cpp
  )



include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/interface
		    ${PROJECT_SOURCE_DIR}/src)



#add_custom_target(WMPROXYAPI DEPENDS 
#  wm.h 
#  soapC.cpp 
#  soapClient.cpp 
#  wmproxy_api.cpp 
#  wmproxy_api_utilities.cpp 
#  dom_${GSOAP_VERSION}.cpp )

add_library(glite_wms_wmproxy_api_cpp SHARED 
  soapC.cpp
  soapClient.cpp 
  wmproxy_api.cpp 
  wmproxy_api_utilities.cpp 
  dom_${GSOAP_VERSION}.cpp )

set_target_properties(glite_wms_wmproxy_api_cpp PROPERTIES VERSION 0.0.0 SOVERSION 0)

install(TARGETS glite_wms_wmproxy_api_cpp
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

add_subdirectory(pkgconfig)
