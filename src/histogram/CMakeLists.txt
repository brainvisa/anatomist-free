SOMA_GET_FILE_LIST_FROM_PRO( histogram.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
SOMA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anahistogram MODULE ${_sources} )
set_property( TARGET anahistogram PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anahistogram anatomist "${QWT_LIBRARY}" )
SOMA_INSTALL( TARGETS anahistogram
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
