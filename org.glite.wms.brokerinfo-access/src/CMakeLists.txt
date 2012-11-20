
include_directories(${CLASSAD_INCLUDE_DIRS}
		    ${PROJECT_SOURCE_DIR}/src
		    ${CLASSAD_INCLUDE_DIRS})

add_library(glite-brokerinfo SHARED 
  BrokerInfo.C
  BrokerInfo.h
  bi_result.h
  )

add_executable(
  glite-brokerinfo-cli BrokerCLI.C
)

target_link_libraries(
  glite-brokerinfo-cli
  glite-brokerinfo
  ${CLASSAD_LDFLAGS}
  )

set_target_properties(glite-brokerinfo PROPERTIES VERSION 0.0.0 SOVERSION 0)


install(FILES glite-brokerinfo-cli
  DESTINATION ${PREFIX}/${BINDIR} 
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  RENAME glite-brokerinfo
  )


install( FILES
	 BrokerInfo.h
	 bi_result.h
	 DESTINATION
	 ${PREFIX}/include/glite/wms/brokerinfo-access
)

install(TARGETS 
  glite-brokerinfo
  LIBRARY
  DESTINATION ${PREFIX}/${LIBDIR})

if(DEFINED INSTALL_BUILD_DEV)
  install(FILES BrokerInfo.h  bi_result.h
    DESTINATION
    ${PREFIX}/include/glite/wms/brokerinfo-access
    )
endif(DEFINED INSTALL_BUILD_DEV)


add_subdirectory(pkgconfig)
