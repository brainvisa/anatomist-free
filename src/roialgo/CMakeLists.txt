BRAINVISA_GET_FILE_LIST_FROM_PRO( roialgo.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
set(ROIALGO_HEADERS ${_headers} PARENT_SCOPE)
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaroialgo MODULE ${_sources} )
add_dependencies( anaroialgo ${headersTarget} )

if( APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anaroialgo PROPERTY OUTPUT_NAME anaroialgo.${${PROJECT_NAME}_VERSION} )
else()
  # in cmake 3.5 on linux, making a "module" removes the version number in the
  # .so so the suffix has to be forced.
  set_target_properties( anaroialgo
    PROPERTIES SOVERSION ${${PROJECT_NAME}_VERSION} SUFFIX ".so.${${PROJECT_NAME}_VERSION}" )
endif()

target_link_libraries( anaroialgo anaroibase ${AIMS_AIMSALGO_LIBRARY} )
BRAINVISA_INSTALL( TARGETS anaroialgo
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
