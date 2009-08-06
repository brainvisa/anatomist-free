# This is an obsolete file it have to be rewritten

BRAINVISA_GET_FILE_LIST_FROM_PRO( anavtk.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )

# getFromPro(anavtk.pro HDRS SRCS)
# kde3_add_moc_files(SRCS ${HDRS})
# SHFJ_COPY_HEADERS(include/anatomist ${HDRS})


SET(QTVTK2_HDRS
module/vtkQtRenderWindowInteractor2.h
)

SET(QTVTK2_SRCS
module/vtkQtRenderWindowInteractor2.cc
module/vtkQtRenderWindow2.cxx
)

# kde3_add_moc_files(QTVTK2_SRCS ${QTVTK2_HDRS})
BRAINVISA_ADD_MOC_FILES( QTVTK2_SRCS ${QTVTK2_HDRS} )


add_library(qtvtk2 SHARED
${QTVTK2_SRCS}
)

target_link_libraries(qtvtk2
${QT_LIBRARIES}
vtkRendering
vtkHybrid
vtkGraphics
vtkIO
vtkCommon 
)

set_property( TARGET qtvtk2 PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )



SET(ANAVTK_SRCS
module/vtkmodule.cc
module/vtkAnatomistCamera.cc
module/vtkAnatomistRenderer.cc
module/vtkAnatomistRendererFactory.cc
window/vtkglcontext.cc
window/vtkglwidget.cc
window3D/vtkglwidget3D.cc
vtkobject/vtkreader.cc
vtkobject/vtkaobject.cc
vtkobject/vtkfiberaobject.cc
vtkobject/vtktensoraobject.cc
vtkobject/vtkvectoraobject.cc
vtkobject/vtkmetadatasetaobject.cc
)


SET(ANAVTK_HDRS
module/vtkmodule.h
module/vtkAnatomistCamera.h
module/vtkAnatomistRenderer.h
module/vtkAnatomistRendererFactory.h
window/vtkglcontext.h
window/vtkglwidget.h
window3D/vtkglwidget3D.h
vtkobject/vtkreader.h
vtkobject/vtkaobject.h
vtkobject/vtkfiberaobject.h
vtkobject/vtktensoraobject.h
vtkobject/vtkvectoraobject.h
vtkobject/vtkmetadatasetaobject.h
)


# SHFJ_COPY_HEADERS(include/anatomist ${ANAVTK_HDRS})
BRAINVISA_COPY_AND_INSTALL_HEADERS( ${ANAVTK_HDRS} anatomist )

# kde3_add_moc_files(ANAVTK_SRCS ${ANAVTK_HDRS})
BRAINVISA_ADD_MOC_FILES(ANAVTK_SRCS ${ANAVTK_HDRS})


add_library(anavtk MODULE
${ANAVTK_SRCS}
)


target_link_libraries(anavtk
anatomist
qtvtk2
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

set_property( TARGET anavtk PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )

#SET_TARGET_PROPERTIES(anavtk
#PROPERTIES SUFFIX "-${CARTO_DEBUGMODE}.so"
#${BrainVISA_LIBRARY_PROPERTIES}
#)


if(APPLE)
  set(GUI_EXECUTABLE MACOSX_BUNDLE)
else(APPLE)
  set(GUI_EXECUTABLE)
endif(APPLE)

add_executable(qtCone ${GUI_EXECUTABLE}
module/testMain.cxx
)

target_link_libraries(qtCone
qtvtk2
${QT_LIBRARIES}
vtkRendering
vtkGraphics
vtkIO
vtkCommon
vtkRenderingAddOn
)

BRAINVISA_INSTALL( TARGETS qtvtk2 anavtk
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )

BRAINVISA_INSTALL( TARGETS qtCone
              DESTINATION bin
              COMPONENT ${PROJECT_NAME} )
