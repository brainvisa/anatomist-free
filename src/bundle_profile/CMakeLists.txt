SOMA_GET_FILE_LIST_FROM_PRO( bundleprofile.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
add_library( bundle_anaprofile MODULE ${_sources} )
set_property( TARGET bundle_anaprofile PROPERTY VERSION ${anatomist_VERSION} )
target_link_libraries( bundle_anaprofile anaprofile )
SOMA_INSTALL( TARGETS bundle_anaprofile
              DESTINATION lib
              COMPONENT anatomist )
