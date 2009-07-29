TEMPLATE	= bundle
TARGET		= anaperfusion${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_PERFUSION}

HEADERS = \
	perfusion/perfProcQtDeco.h \
	winperf/perfWin.h \
	winperf/qfloatspinbox.h \
	module/perfusion.h

SOURCES = \
	perfusion/perfProcQtDeco.cc \
	winperf/perfWin.cc \
	winperf/qfloatspinbox.cc \
	module/perfusion.cc
