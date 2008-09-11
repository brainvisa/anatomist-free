
getFromPro(anafold.pro HEADERS SOURCES)

SHFJ_COPY_HEADERS(include/anafold ${HEADERS})

kde3_add_moc_files(SOURCES ${HEADERS})

ADD_LIBRARY(anafold MODULE
${SOURCES}
)

TARGET_LINK_LIBRARIES(anafold
anatomist
sigraph
${QT_LIBRARIES}
)

INSTALL(TARGETS anafold DESTINATION lib/)

