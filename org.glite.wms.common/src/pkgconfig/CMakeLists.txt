configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wms-common-conf.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/wms-common-conf.pc
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wms-common-ii.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/wms-common-ii.pc
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wms-common-logger.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/wms-common-logger.pc
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wms-common.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/wms-common.pc
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wms-common-process.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/wms-common-process.pc
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wms-common-quota.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/wms-common-quota.pc
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wms-common-util.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/wms-common-util.pc
               @ONLY)



install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
        DESTINATION ${PREFIX}/usr/${LIBDIR}/pkgconfig
        FILES_MATCHING PATTERN "*.pc"
        PATTERN "CMakeFiles" EXCLUDE)
