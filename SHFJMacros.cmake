MACRO(SHFJ_COPY_HEADERS _dstdir)
        FOREACH(_current_FILE ${ARGN})
                CONFIGURE_FILE(
                ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
                ${_dstdir}/${_current_FILE}
                COPYONLY)
        ENDFOREACH(_current_FILE)
ENDMACRO(SHFJ_COPY_HEADERS)

set(CMAKE_COLOR_MAKEFILE 1)

# pour les paths hardcodes....
set(p4 "/volatile/thyreau/perforce")
