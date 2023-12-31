find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)
find_package(VTK REQUIRED)
find_package(ITK REQUIRED)
find_package(DCMTK REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

include(${VTK_USE_FILE})
include(${ITK_USE_FILE})

project(pcl)

add_library(pcl INTERFACE)# no sources
target_include_directories(pcl INTERFACE ${PCL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${ITK_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS} ${DCMTK_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
target_link_libraries(pcl INTERFACE ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${DCMTK_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})

message(STATUS ${PCL_INCLUDE_DIR})
message(STATUS ${Boost_FOUND})
message(STATUS ${Boost_INCLUDE_DIRS})
message(STATUS ${DCMTK_FOUND})
message(STATUS ${DCMTK_INCLUDE_DIRS})
message(STATUS ${VTK_FOUND})
message(STATUS ${VTK_INCLUDE_DIRS})
message(STATUS ${ITK_FOUND})
message(STATUS ${ITK_INCLUDE_DIRS})
message(STATUS ${ZLIB_FOUND})
message(STATUS ${ZLIB_INCLUDE_DIR})
message(STATUS "!!!!!!!!!!!!!!!!!!!!!")
