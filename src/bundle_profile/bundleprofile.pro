TEMPLATE	= bundle
TARGET		= anaprofile${BUILDMODEEXT}

#!include ../../config

SOURCES =			\
    bundle/profilebundle.cc

HEADERS =


LIBS = $(LIBS_WITH_QWT)

default:LIBS	+= -lanaprofile
debug:LIBS	+= -lanaprofile-debug
release:LIBS	+= -lanaprofile
