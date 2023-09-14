find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)
find_package(VTK REQUIRED)
find_package(ITK REQUIRED)
find_package(DCMTK REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

project(qia_core)

#get numpy include path
execute_process(
  COMMAND
  ${PYTHON_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
  OUTPUT_VARIABLE NUMPY_INCLUDE_PATH
  RESULT_VARIABLE NUMPY_ERR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
if(NUMPY_ERR)
  message(SEND_ERROR "WARNING: NumPy header not found.")
endif(NUMPY_ERR)
include_directories(${NUMPY_INCLUDE_PATH})

include_directories(${CMAKE_CURRENT_LIST_DIR})
message(STATUS ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "#######################")

# BUILD EACH MODULE

############### image
add_cython_target(image CXX)
add_library(image MODULE ${image})
target_link_libraries(image pcl)
python_extension_module(image)
install(TARGETS image LIBRARY DESTINATION dependencies/bin/qia/common/img)

############### statistics
add_cython_target(statistics CXX)
add_library(statistics MODULE ${statistics})
target_link_libraries(statistics pcl)
python_extension_module(statistics)
install(TARGETS statistics LIBRARY DESTINATION dependencies/bin/qia/common/img)

############### element
add_cython_target(element CXX)
add_library(element MODULE ${element})
target_link_libraries(element pcl)
python_extension_module(element)
install(TARGETS element LIBRARY DESTINATION dependencies/bin/qia/common/img)

############### filter
add_cython_target(filter CXX)
add_library(filter MODULE ${filter})
target_link_libraries(filter pcl)
python_extension_module(filter)
install(TARGETS filter LIBRARY DESTINATION dependencies/bin/qia/common/img)

############### measure
add_cython_target(measure CXX)
add_library(measure MODULE ${measure})
target_link_libraries(measure pcl)
python_extension_module(measure)
install(TARGETS measure LIBRARY DESTINATION dependencies/bin/qia/common/img)

############### overlay
add_cython_target(overlay CXX)
add_library(overlay MODULE ${overlay})
target_link_libraries(overlay pcl)
python_extension_module(overlay)
install(TARGETS overlay LIBRARY DESTINATION dependencies/bin/qia/common/img)

############### lut
add_cython_target(lut CXX)
add_library(lut MODULE ${lut})
target_link_libraries(lut pcl)
python_extension_module(lut)
install(TARGETS lut LIBRARY DESTINATION dependencies/bin/qia/common/img)
