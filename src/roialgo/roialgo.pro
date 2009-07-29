TEMPLATE	= bundle
TARGET		= anaroialgo${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_ROIALGO}

HEADERS = \
	action/dynsegmentaction.h \
	action/morphomath.h \
        control/dynsegmentcontrol.h \
	application/roialgomodule.h

SOURCES = \
	action/dynsegmentaction.cc \
	action/morphomath.cc \
        control/dynsegmentcontrol.cc \
	application/roialgomodule.cc
