# This is an obsolete file it have to be rewritten

getFromPro(anavtk.pro HDRS SRCS)


# kde3_add_moc_files(SRCS ${HDRS})


SHFJ_COPY_HEADERS(include/anatomist ${HDRS})


SET(QTVTK2_HDRS
module/vtkQtRenderWindowInteractor2.h
)

SET(QTVTK2_SRCS
module/vtkQtRenderWindowInteractor2.cc
module/vtkQtRenderWindow2.cxx
)

kde3_add_moc_files(QTVTK2_SRCS ${QTVTK2_HDRS})


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


INSTALL(TARGETS qtvtk2 DESTINATION lib/)




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


SHFJ_COPY_HEADERS(include/anatomist ${ANAVTK_HDRS})


kde3_add_moc_files(ANAVTK_SRCS ${ANAVTK_HDRS})


ADD_LIBRARY(anavtk MODULE
${ANAVTK_SRCS}
)


TARGET_LINK_LIBRARIES(anavtk
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

SET_TARGET_PROPERTIES(anavtk
PROPERTIES SUFFIX "-${CARTO_DEBUGMODE}.so"
${BrainVISA_LIBRARY_PROPERTIES}
)

IF(APPLE)
  SET(GUI_EXECUTABLE MACOSX_BUNDLE)
ELSE(APPLE)
  SET(GUI_EXECUTABLE)
ENDIF(APPLE)

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

INSTALL(TARGETS anavtk DESTINATION lib/)
