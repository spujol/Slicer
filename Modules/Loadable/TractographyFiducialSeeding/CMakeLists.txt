
#-----------------------------------------------------------------------------
set(MODULE_NAME "TractographyFiducialSeeding") # Do not use 'project()'
set(MODULE_TITLE "Tractography Fiducial Seeding")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

set(MODULE_SRCS
  qSlicerTractographyFiducialSeedingModule.cxx
  qSlicerTractographyFiducialSeedingModule.h
  qSlicerTractographyFiducialSeedingModuleWidget.cxx
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerTractographyFiducialSeedingModule.h
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerTractographyFiducialSeedingModule.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicerTractographyFiducialSeedingModuleLogic
  vtkSlicerAnnotationsModuleMRML
  )

set(MODULE_RESOURCES
  Resources/qSlicerTractographyFiducialSeedingModule.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
