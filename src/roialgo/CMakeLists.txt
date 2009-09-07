BRAINVISA_GET_FILE_LIST_FROM_PRO( roialgo.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaroialgo MODULE ${_sources} )
add_dependencies( anaroialgo ${headersTarget} )
set_property( TARGET anaroialgo PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anaroialgo anatomist anaroibase ${AIMS_LIBRARIES} )
BRAINVISA_INSTALL( TARGETS anaroialgo
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
