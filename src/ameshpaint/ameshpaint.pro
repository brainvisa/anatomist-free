TEMPLATE    = app
TARGET      = ameshpaint
win32:TARGET    = ameshpaint.exe

#!include ../../config-app

LIBS += -lanatomist -laimsalgo

HEADERS = glwidget.h \
    meshpaint.h \
    vector.h \
    trackball.h \
    spath/geodesic_algorithm_base.h \
    spath/geodesic_algorithm_subdivision.h \
    spath/geodesic_algorithm_dijkstra_alternative.h \
    spath/geodesic_constants_and_simple_functions.h \
    spath/geodesic_algorithm_dijkstra.h \
    spath/geodesic_memory.h \
    spath/geodesic_algorithm_exact_elements.h \
    spath/geodesic_mesh_elements.h \
    spath/geodesic_algorithm_exact.h \
    spath/geodesic_mesh.h \
    spath/geodesic_algorithm_graph_base.h

SOURCES = glwidget.cc \
    meshpaint.cc \
    trackball.cc \
    ameshpaint.cc
