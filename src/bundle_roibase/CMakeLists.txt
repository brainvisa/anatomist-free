SOMA_GET_FILE_LIST_FROM_PRO( bundleroibase.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
add_library( bundle_roibase MODULE ${_sources} )
set_property( TARGET bundle_roibase PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( bundle_roibase anaroibase )
SOMA_INSTALL( TARGETS bundle_roibase
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
