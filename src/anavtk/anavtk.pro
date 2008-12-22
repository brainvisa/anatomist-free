TEMPLATE	= bundle
TARGET		= anavtk${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_ANAVTK}

HEADERS = \
        module/vtkmodule.h \
        module/vtkAnatomistCamera.h \
        module/vtkAnatomistRenderer.h \
        module/vtkAnatomistRendererFactory.h \
        module/vtkQtRenderWindow2.h \
        module/vtkQtRenderWindowInteractor2.h \
        vtkobject/vtkaobject.h \
        vtkobject/vtkreader.h \
        window/vtkglcontext.h \
        window/vtkglwidget.h \
        window3D/vtkglwidget3D.h \
        vtkobject/vtkfiberaobject.h \
        vtkobject/vtktensoraobject.h \
        vtkobject/vtkvectoraobject.h \
        vtkobject/vtkmetadatasetaobject.h

SOURCES = \
        module/vtkmodule.cc \
        module/vtkAnatomistCamera.cc \
        module/vtkAnatomistRenderer.cc \
        module/vtkAnatomistRendererFactory.cc \
        module/vtkQtRenderWindow2.cxx \
        module/vtkQtRenderWindowInteractor2.cc \
        vtkobject/vtkaobject.cc \
        vtkobject/vtkreader.cc \
        window/vtkglcontext.cc \
        window/vtkglwidget.cc \
        window3D/vtkglwidget3D.cc \
        vtkobject/vtkfiberaobject.cc \
        vtkobject/vtktensoraobject.cc \
        vtkobject/vtkvectoraobject.cc \
        vtkobject/vtkmetadatasetaobject.cc
