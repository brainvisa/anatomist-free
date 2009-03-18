SOMA_GET_FILE_LIST_FROM_PRO( hierarchyeditor.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
SOMA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anahierarchyeditor MODULE ${_sources} )
set_property( TARGET anahierarchyeditor PROPERTY VERSION ${anatomist_VERSION} )
target_link_libraries( anahierarchyeditor anatomist )
SOMA_INSTALL( TARGETS anahierarchyeditor
              DESTINATION lib
              COMPONENT anatomist )
