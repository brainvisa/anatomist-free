BRAINVISA_GET_FILE_LIST_FROM_PRO( anasurfmatch.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anasurfmatch MODULE ${_sources} )
set_property( TARGET anasurfmatch PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anasurfmatch anatomist )
BRAINVISA_INSTALL( TARGETS anasurfmatch
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
