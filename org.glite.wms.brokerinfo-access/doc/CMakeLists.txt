add_custom_command(OUTPUT 
  ${PREFIX}/share/man/man1/glite-brokerinfo.1.gz
  COMMAND mkdir -p ${PREFIX}/share/man/man1
  COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/glite-brokerinfo.1 > ${PREFIX}/share/man/man1/glite-brokerinfo.1.gz
)

add_custom_command(OUTPUT 
  ${CMAKE_CURRENT_SOURCE_DIR}/glite-brokerinfo.1
  COMMAND 
  xsltproc /usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl ${CMAKE_CURRENT_SOURCE_DIR}/glite-brokerinfo.xml
)

add_custom_target(ZIPMAN ALL DEPENDS 
  ${PREFIX}/share/man/man1/glite-brokerinfo.1.gz
    )

add_custom_target(XMLPROC ALL DEPENDS 
  ${CMAKE_CURRENT_SOURCE_DIR}/glite-brokerinfo.1
    )
