TEMPLATE	= bundle
TARGET		= anavtk${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_ANAVTK}

HEADERS = \
        module/vtkmodule.h \
        module/vtkAnatomistCamera.h \
        module/vtkAnatomistRenderer.h \
        module/vtkQtRenderWindow2.h \
        module/vtkQtRenderWindowInteractor2.h \
        window/vtkglcontext.h \
        window/vtkglwidget.h \
        window3D/vtkglwidget3D.h

SOURCES = \
        module/vtkmodule.cc \
        module/vtkAnatomistCamera.cc \
        module/vtkAnatomistRenderer.cc \
        module/vtkQtRenderWindow2.cxx \
        module/vtkQtRenderWindowInteractor2.cc \
        window/vtkglcontext.cc \
        window/vtkglwidget.cc \
        window3D/vtkglwidget3D.cc
