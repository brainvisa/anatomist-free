TEMPLATE        = bundle
TARGET  = pyanatomistmodule${BUILDMODEEXT}

#!include ../../config

LIBS	= ${ANAPYTHON_LIBS}

HEADERS = \
        module/pyanatomistmodule.h \
        module/pythonlauncher.h

SOURCES = \
        module/pyanatomistmodule.cc \
        module/pythonlauncher.cc

