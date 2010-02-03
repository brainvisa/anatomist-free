BRAINVISA_GET_FILE_LIST_FROM_PRO( hierarchyeditor.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anahierarchyeditor MODULE ${_sources} )
add_dependencies( anahierarchyeditor ${headersTarget} )
set_property( TARGET anahierarchyeditor PROPERTY OUTPUT_NAME anahierarchyeditor.${${PROJECT_NAME}_VERSION} )
target_link_libraries( anahierarchyeditor anatomist )
BRAINVISA_INSTALL( TARGETS anahierarchyeditor
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
