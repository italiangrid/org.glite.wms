
install(FILES  
  ${CMAKE_CURRENT_SOURCE_DIR}/glite-wms-job-status 
  ${CMAKE_CURRENT_SOURCE_DIR}/glite-wms-job-logging-info
  DESTINATION
  ${PREFIX}/${BINDIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )

if(DEFINED INSTALL_BUILD)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/glite-wms-job-status
  ${CMAKE_CURRENT_SOURCE_DIR}/glite-wms-job-logging-info
  DESTINATION
  ${INSTALL_BUILD}/${BINDIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
endif(DEFINED INSTALL_BUILD)
