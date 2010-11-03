TEMPLATE    = app
TARGET      = ameshpaint
win32:TARGET    = ameshpaint.exe

#!include ../../config-app

LIBS += -lanatomist -laimsalgo

HEADERS = glwidget.h \
    meshpaint.h \
    vector.h \
    trackball.h \

SOURCES = glwidget.cc \
    meshpaint.cc \
    trackball.cc \
    ameshpaint.cc
