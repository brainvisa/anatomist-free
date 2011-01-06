include_directories( "${QWT_INCLUDE_DIR}" )

BRAINVISA_GET_FILE_LIST_FROM_PRO( histogram.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anahistogram MODULE ${_sources} )
add_dependencies( anahistogram ${headersTarget} )
if( NOT APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anahistogram PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
else()
  set_property( TARGET anahistogram PROPERTY OUTPUT_NAME anahistogram.${${PROJECT_NAME}_VERSION} )
endif()
target_link_libraries( anahistogram anatomist "${QWT_LIBRARY}" )
BRAINVISA_INSTALL( TARGETS anahistogram
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
