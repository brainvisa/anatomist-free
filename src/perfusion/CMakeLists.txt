BRAINVISA_GET_FILE_LIST_FROM_PRO( perfusion.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaperfusion MODULE ${_sources} )
add_dependencies( anaperfusion ${headersTarget} )
set_property( TARGET anaperfusion PROPERTY OUTPUT_NAME anaperfusion.${${PROJECT_NAME}_VERSION} )
target_link_libraries( anaperfusion anaprofile ${AIMS_AIMSALGO_LIBRARY} )
BRAINVISA_INSTALL( TARGETS anaperfusion
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
