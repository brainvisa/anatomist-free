BRAINVISA_GET_FILE_LIST_FROM_PRO( volrender.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anavolrender MODULE ${_sources} )
add_dependencies( anavolrender ${headersTarget} )
if( NOT APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anavolrender PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
endif()
target_link_libraries( anavolrender anatomist )
BRAINVISA_INSTALL( TARGETS anavolrender
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
