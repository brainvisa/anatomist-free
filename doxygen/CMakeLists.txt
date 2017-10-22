IF ( DOXYGEN_FOUND )
  set(component_name "anatomist")
#   SET( DOXYFILE_HTML_HEADER "\"${CMAKE_CURRENT_SOURCE_DIR}/head_${component_name}.html\"" )
  set( _doxygenInput ${HEADERS} )
  set( DOXYFILE_PREDEFINED ${ANATOMIST_DEFINITIONS} ${AIMS-FREE_DEFINITIONS}
    ${SOMA-IO_DEFINITIONS} CARTO_USE_BLITZ )
  set(aims-free_version
    "${aims-free_VERSION_MAJOR}.${aims-free_VERSION_MINOR}")
  set(_somaio_version "${soma-io_VERSION_MAJOR}.${soma-io_VERSION_MINOR}")
  set( DOXYFILE_TAGFILES
    "${CMAKE_BINARY_DIR}/share/doc/aimsgui-${aims-free_version}/doxygen/aimsgui.tag=../../aimsgui-${aims-free_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/aimsdata-${aims-free_version}/doxygen/aimsdata.tag=../../aimsdata-${aims-free_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/graph-${aims-free_version}/doxygen/graph.tag=../../graph-${aims-free_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/cartobase-${_somaio_version}/doxygen/cartobase.tag=../../cartobase-${_somaio_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/soma-io-${_somaio_version}/doxygen/soma-io.tag=../../cartobase-${_somaio_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/cartodata-${aims-free_version}/doxygen/cartodata.tag=../../cartodata-${aims-free_version}/doxygen")
  set( DOXYFILE_PROJECT_LOGO "\"${CMAKE_CURRENT_SOURCE_DIR}/anatomist.png\"" )
  set( DOXYFILE_PROJECT_BRIEF "\"3D neuroimaging data viewer\"" )
  BRAINVISA_GENERATE_DOXYGEN_DOC(
    _doxygenInput
    head_${component_name}.html ${component_name}.png ${component_name}.gif
    INPUT_PREFIX "${CMAKE_BINARY_DIR}/include/${component_name}"
    COMPONENT "${component_name}" )
  add_dependencies( ${component_name}-doxygen aimsgui-doxygen aimsdata-doxygen graph-doxygen soma-io-doxygen cartobase-doxygen cartodata-doxygen )
ENDIF ( DOXYGEN_FOUND )
