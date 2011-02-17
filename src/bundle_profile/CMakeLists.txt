BRAINVISA_GET_FILE_LIST_FROM_PRO( bundleprofile.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
set(BUNDLEPROFILE_HEADERS ${_headers} PARENT_SCOPE)
add_library( bundle_anaprofile MODULE ${_sources} )
add_dependencies( bundle_anaprofile ${headersTarget} )
if( NOT APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET bundle_anaprofile PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
endif()
set_property( TARGET bundle_anaprofile PROPERTY OUTPUT_NAME anaprofile.${${PROJECT_NAME}_VERSION} )
target_link_libraries( bundle_anaprofile anaprofile )
BRAINVISA_INSTALL( TARGETS bundle_anaprofile
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
