if( soma-infra_FOUND )

  SOMA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}"
                       "share/doc/anatomist-${anatomist_VERSION_MAJOR}.${anatomist_VERSION_MINOR}"
                       anatomist-html html-doc )

else( soma-infra_FOUND )

  SHFJ_COPY_CURRENT_DIRECTORY(share/doc/anatomist-${BrainVISA_VERSION})

endif( soma-infra_FOUND )
