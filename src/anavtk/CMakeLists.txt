BRAINVISA_GET_FILE_LIST_FROM_PRO( anavtk.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anavtk MODULE ${_sources} )
set_property( TARGET anavtk PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anavtk
anatomist
vtkHybrid
vtkVisuManagement
vtkDataManagement
vtkHWShading
vtkRenderingAddOn
vtkWidgets
vtkRendering
vtkGraphics
vtkIO
vtkCommon
)
BRAINVISA_INSTALL( TARGETS anavtk
DESTINATION lib
COMPONENT ${PROJECT_NAME} )

