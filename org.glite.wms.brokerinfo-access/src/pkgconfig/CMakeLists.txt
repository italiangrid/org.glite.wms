configure_file(${CMAKE_CURRENT_SOURCE_DIR}/brokerinfo-access.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/brokerinfo-access.pc
               @ONLY)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
  DESTINATION ${PREFIX}/usr/${LIBDIR}/pkgconfig
  FILES_MATCHING PATTERN "*.pc"
  PATTERN "CMakeFiles" EXCLUDE)
