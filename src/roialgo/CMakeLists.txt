SOMA_GET_FILE_LIST_FROM_PRO( roialgo.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
SOMA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaroialgo MODULE ${_sources} )
set_property( TARGET anaroialgo PROPERTY VERSION ${anatomist_VERSION} )
target_link_libraries( anaroialgo anatomist anaroibase ${AIMS_LIBRARIES} )
SOMA_INSTALL( TARGETS anaroialgo
              DESTINATION lib
              COMPONENT anatomist )
