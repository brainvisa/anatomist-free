BRAINVISA_GET_FILE_LIST_FROM_PRO( surfpaint.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anasurfpaint MODULE ${_sources} )
add_dependencies( anasurfpaint ${headersTarget} )
set_property( TARGET anasurfpaint PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anasurfpaint anatomist ${AIMS_AIMSALGO_LIBRARY} )
BRAINVISA_INSTALL( TARGETS anasurfpaint
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
