



install(TARGETS glite-wms-job-status glite-wms-job-logging-info
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${BINDIR})

if(DEFINED INSTALL_BUILD)
  install(TARGETS glite-wms-job-status glite-wms-job-logging-info
    DESTINATION ${INSTALL_BUILD}/${BINDIR})
endif(DEFINED INSTALL_BUILD)

