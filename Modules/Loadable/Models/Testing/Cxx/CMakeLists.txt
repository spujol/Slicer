set(KIT qSlicer${MODULE_NAME}Module)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${MODULE_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  qSlicerModelsModuleWidgetTest1
  qSlicerModelsModuleWidgetTestScene
  vtkSlicerModelsLogicAddFileTest
  vtkSlicerModelsLogicAutoRemoveTest
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

set(MODEL_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/cube.vtk")
set(MODEL_SCENE "${MRMLCore_SOURCE_DIR}/Testing/modelHierarchy.mrml")

SIMPLE_TEST( qSlicerModelsModuleWidgetTest1 ${MODEL_FILE} )
SIMPLE_TEST( qSlicerModelsModuleWidgetTestScene ${MODEL_SCENE} )
SIMPLE_TEST(vtkSlicerModelsLogicAddFileTest ${MODEL_FILE})
SIMPLE_TEST(vtkSlicerModelsLogicAutoRemoveTest)
