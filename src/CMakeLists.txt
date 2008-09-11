if( soma-infra_FOUND )

  add_subdirectory(lib)
  add_subdirectory(anatomist)
  add_subdirectory(mkhierarchy)
  add_subdirectory(hierarchyeditor)
  add_subdirectory(anasurfmatch)
  add_subdirectory(volrender)
  add_subdirectory(profile)
  if( APPLE )
    add_subdirectory(bundle_profile)
  endif( APPLE )
  if( QWT_FOUND )
    add_subdirectory(roibase)
    if( APPLE )
      add_subdirectory(bundle_roibase)
    endif( APPLE )
    add_subdirectory(histogram)
  else( QWT_FOUND )
    message( "WARNING: anatomist modules anaroibase and anahistogram will not be build because Qwt library has not been found" )
  endif( QWT_FOUND )
  add_subdirectory(perfusion)
  add_subdirectory(roialgo)
  # TODO
  if( VTK_FOUND AND vtkINRIA3D_FOUND )
    add_subdirectory( anavtk )
  else( VTK_FOUND AND vtkINRIA3D_FOUND )
    message( "WARNING: anatomist modules anavtk will not be build because VTK or vtkINRIA3D library has not been found" )
  endif( VTK_FOUND AND vtkINRIA3D_FOUND )
#   

else( soma-infra_FOUND )

  #module(anaqwt):SUBDIRS	+= profile
  #module(anaalgo)-module(anaqwt):SUBDIRS	+= roibase
  
  #PSUBDIRS = \
  #		anatomist \
  #		mkhierarchy \
  #		hierarchyeditor
  
  #module(anaalgo):PSUBDIRS	+= \
  #		anasurfmatch \
  #		volrender
  #
  #module(anaalgo)-module(anaqwt):PSUBDIRS	+= \
  #		perfusion \
  #		roialgo
  
  #darwin-module(anaqwt):PSUBDIRS	+= \
  #		bundle_profile \
  #		bundle_roibase
  #
  #darwin:PSUBDIRS	+= mac-frontend
  
  add_subdirectory(lib)
  add_subdirectory(anatomist)
  add_subdirectory(mkhierarchy)
  add_subdirectory(hierarchyeditor)
  add_subdirectory(anasurfmatch)
  add_subdirectory(volrender)
  add_subdirectory(profile)
  add_subdirectory(roibase)
  add_subdirectory(perfusion)
  add_subdirectory(roialgo)
  add_subdirectory(histogram)
  
  IF(VTK_FOUND AND vtkINRIA3D_FOUND)
    add_subdirectory(anavtk)
  ENDIF(VTK_FOUND AND vtkINRIA3D_FOUND)
  
  IF( APPLE )
    add_subdirectory(bundle_profile)
    add_subdirectory(bundle_roibase)
  ENDIF( APPLE )

endif( soma-infra_FOUND )
