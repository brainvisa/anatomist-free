
SET(pyanatomistmodule_HDRS
module/pyanatomistmodule.h
module/pythonlauncher.h
)

SHFJ_COPY_HEADERS(include/pyanatomist ${pyanatomistmodule_HDRS})

SET(pyanatomistmodule_SRCS
module/pyanatomistmodule.cc
module/pythonlauncher.cc
)

KDE3_ADD_MOC_FILES(pyanatomistmodule_SRCS
module/pythonlauncher.h
)

ADD_LIBRARY(pyanatomistmodule MODULE
${pyanatomistmodule_SRCS}
)

TARGET_LINK_LIBRARIES(pyanatomistmodule
pyanatomistexports
${PYTHON_LIBRARY}
)

INSTALL(TARGETS pyanatomistmodule DESTINATION lib/)
