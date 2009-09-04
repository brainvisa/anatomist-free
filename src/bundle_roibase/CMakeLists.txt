BRAINVISA_GET_FILE_LIST_FROM_PRO( bundleroibase.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
add_library( bundle_roibase MODULE ${_sources} )
add_dependencies( bundle_roibase ${headersTarget} )
set_property( TARGET bundle_roibase PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( bundle_roibase anaroibase )
BRAINVISA_INSTALL( TARGETS bundle_roibase
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
