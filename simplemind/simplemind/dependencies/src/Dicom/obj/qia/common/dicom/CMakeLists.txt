find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)
#find_package(VTK REQUIRED)
#find_package(ITK REQUIRED)
find_package(DCMTK REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

project(dcm)

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

include_directories(${CMAKE_CURRENT_LIST_DIR} ${DCMTK_INCLUDE_DIRS})

add_cython_target(obj CXX)
add_library(obj MODULE ${obj})
target_link_libraries(obj ${DCMTK_LIBRARIES} pcl)
python_extension_module(obj)

install(TARGETS obj LIBRARY DESTINATION dependencies/bin/qia/common/dicom)

