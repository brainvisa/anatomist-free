TEMPLATE	= lib
TARGET		= anafold${BUILDMODEEXT}

#!include ../../config-local

darwin:LFLAGS_SHLIB	= -bundle
darwin:LFLAGS_SONAME	=
darwin:EXTENSION_SHLIB	= .so

solaris:INCLUDEPATH += \
	/usr/openwin/include

linux:INCLUDEPATH += \
	/usr/X11R6/include

HEADERS = \
	fgraph/afgHelpers.h \
	fgraph/fgMethod.h \
	fgraph/afgraph.h \
	fgraph/qwAnnealParams.h \
	fgraph/qwFFusionCtrl.h \
	module/anafold.h

SOURCES = \
	fgraph/afgHelpers.cc \
	fgraph/fgMethod.cc \
	fgraph/afgraph.cc \
	fgraph/qwAnnealParams.cc \
	fgraph/qwFFusionCtrl.cc \
	module/anafold.cc
