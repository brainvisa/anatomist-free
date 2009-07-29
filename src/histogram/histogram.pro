TEMPLATE	= bundle
TARGET		= anahistogram${BUILDMODEEXT}

#!include ../../config

LIBS = $(LIBS_WITH_QWT)

HEADERS = \
	winhisto/histoWin.h \
	winhisto/histo.h \
	module/histogram.h

SOURCES = \
	winhisto/histoWin.cc \
	winhisto/histo.cc \
	module/histogram.cc
