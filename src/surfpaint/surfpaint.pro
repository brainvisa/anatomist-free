TEMPLATE	= bundle
TARGET		= anasurfpaint${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_WITH_ALGO}

HEADERS = \
    module/surfpaintmodule.h \
    module/surfpainttools.h \
    action/surfpaintaction.h \
    control/surfpaintcontrol.h

SOURCES = \
    module/surfpaintmodule.cc \
    module/surfpainttools.cc \
    action/surfpaintaction.cc \
    control/surfpaintcontrol.cc

