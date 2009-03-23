SOMA_GET_FILE_LIST_FROM_PRO( roibase.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
SOMA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaroibase SHARED ${_sources} )
set_property( TARGET anaroibase PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anaroibase anatomist "${QWT_LIBRARY}" )
SOMA_INSTALL( TARGETS anaroibase
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
