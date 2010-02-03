BRAINVISA_GET_FILE_LIST_FROM_PRO( bundleprofile.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
add_library( bundle_anaprofile MODULE ${_sources} )
add_dependencies( bundle_anaprofile ${headersTarget} )
set_property( TARGET bundle_anaprofile PROPERTY OUTPUT_NAME anaprofile.${${PROJECT_NAME}_VERSION} )
target_link_libraries( bundle_anaprofile anaprofile )
BRAINVISA_INSTALL( TARGETS bundle_anaprofile
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
