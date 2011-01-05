BRAINVISA_GET_FILE_LIST_FROM_PRO( surfpaint.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anasurfpaint MODULE ${_sources} )
add_dependencies( anasurfpaint ${headersTarget} )
if( NOT APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anasurfpaint PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
endif()
target_link_libraries( anasurfpaint anatomist ${AIMS_AIMSALGO_LIBRARY} )
BRAINVISA_INSTALL( TARGETS anasurfpaint
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
