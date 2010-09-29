TEMPLATE	= bundle
TARGET		= anasurfpaint${BUILDMODEEXT}

#!include ../../config

LIBS=${LIBS_FULL}

HEADERS = \
	module/surfpaintmodule.h \
    action/surfpaintaction.h \
    control/surfpaintcontrol.h

SOURCES = \
	module/surfpaintmodule.cc \
    action/surfpaintaction.cc \
    control/surfpaintcontrol.cc