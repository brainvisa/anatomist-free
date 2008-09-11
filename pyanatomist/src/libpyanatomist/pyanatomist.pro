TEMPLATE        = lib
TARGET  = pyanatomistexports${BUILDMODEEXT}

#!include ../../config

LIBS = ${ANATOMIST_LIBS}

HEADERS = \
        event.h \
        pyanatomist.h \
        serializingcontext.h

SOURCES = \
        pyanatomist.cc \
        serializingcontext.cc

