BRAINVISA_GET_FILE_LIST_FROM_PRO( ameshpaint.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
brainvisa_add_executable( ameshpaint ${_sources} )
target_link_libraries( ameshpaint anatomist ${AIMS_AIMSALGO_LIBRARY} )
BRAINVISA_INSTALL( TARGETS ameshpaint
              DESTINATION bin
              COMPONENT ${PROJECT_NAME} )
