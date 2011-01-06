BRAINVISA_GET_FILE_LIST_FROM_PRO( anasurfmatch.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anasurfmatch MODULE ${_sources} )
add_dependencies( anasurfmatch ${headersTarget} )
if( NOT APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anasurfmatch PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
else()
  set_property( TARGET anasurfmatch PROPERTY OUTPUT_NAME anasurfmatch.${${PROJECT_NAME}_VERSION} )
endif()
target_link_libraries( anasurfmatch anatomist ${AIMS_AIMSALGO_LIBRARY} )
BRAINVISA_INSTALL( TARGETS anasurfmatch
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
