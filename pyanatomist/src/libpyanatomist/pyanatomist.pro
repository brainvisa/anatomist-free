TEMPLATE        = lib
TARGET  = pyanatomistexports${BUILDMODEEXT}

#!include ../../config

LIBS = ${ANATOMIST_LIBS}

HEADERS = \
        event.h \
        pyanatomist.h \
        serializingcontext.h \
        sipconverthelpers.h

SOURCES = \
        pyanatomist.cc \
        serializingcontext.cc

