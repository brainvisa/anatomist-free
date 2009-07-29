TEMPLATE	= lib
TARGET		= anaprofile${BUILDMODEEXT}

#!include ../../config

LIBS = $(LIBS_WITH_QWT)

HEADERS = \
	winprof/profFactory.h \
	winprof/profStrategy.h \
	winprof/profT.h \
	winprof/profWin.h \
	winprof/profX.h \
	winprof/profY.h \
	winprof/profZ.h \
	module/profile.h

SOURCES = \
	winprof/profFactory.cc \
	winprof/profStrategy.cc \
	winprof/profT.cc \
	winprof/profWin.cc \
	winprof/profX.cc \
	winprof/profY.cc \
	winprof/profZ.cc \
	module/profile.cc
