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

add_custom_command(OUTPUT 
	${CMAKE_CURRENT_SOURCE_DIR}/autodoc/html/index.html
	COMMAND 
	SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/.."	PROJECT="glite-wms-brokerinfo-access" DOCDIR="autodoc" VERSION="3.3.2" PERL_PATH="/usr/bin/perl" HAVE_DOT="NO" GENERATE_MAN="NO" GENERATE_RTF="NO" GENERATE_XML="NO" GENERATE_HTMLHELP="NO" GENERATE_CHI="NO" GENERATE_HTML="YES" GENERATE_LATEX="NO" /usr/bin/doxygen ${CMAKE_CURRENT_SOURCE_DIR}/../project/doxygen/doxygen.cfg
)

add_custom_target(DOXPROC ALL DEPENDS
   	${CMAKE_CURRENT_SOURCE_DIR}/autodoc/html/index.html
)
