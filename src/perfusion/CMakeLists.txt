BRAINVISA_GET_FILE_LIST_FROM_PRO( perfusion.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
set(PERFUSION_HEADERS ${_headers} PARENT_SCOPE)
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaperfusion MODULE ${_sources} )

if( APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anaperfusion
    PROPERTY OUTPUT_NAME anaperfusion.${${PROJECT_NAME}_VERSION} )
elseif( CMAKE_VERSION VERSION_LESS "3.5" )
    set_target_properties( anaperfusion
        PROPERTIES SOVERSION ${${PROJECT_NAME}_VERSION} )
else()
  # in cmake 3.5 on linux, making a "module" removes the version number in the
  # .so so the suffix has to be forced.
  set_target_properties( anaperfusion
    PROPERTIES SOVERSION ${${PROJECT_NAME}_VERSION} SUFFIX ".so.${${PROJECT_NAME}_VERSION}" )
endif()

add_dependencies( anaperfusion ${headersTarget} )
target_link_libraries( anaperfusion anaprofile ${AIMS_AIMSALGO_LIBRARY} )
BRAINVISA_INSTALL( TARGETS anaperfusion
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
