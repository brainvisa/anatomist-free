TEMPLATE    = app
TARGET      = ameshpaint

#!include ../../config-app

LIBS = ${LIBS_WITH_ALGO}

HEADERS = glwidget.h \
    meshpaint.h \
    vector.h \
    trackball.h \

SOURCES = glwidget.cc \
    meshpaint.cc \
    trackball.cc \
    ameshpaint.cc
