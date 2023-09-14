set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(VTK REQUIRED)
find_package(ITK REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

project(simplemind)

file(GLOB SOURCE_FILES
   "*.h"
   "*.cc"
)
add_executable(sm ${SOURCE_FILES})
target_link_libraries(sm ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
target_include_directories(sm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PCL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${ITK_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
install(TARGETS sm RUNTIME DESTINATION think/bin/sm)

message(STATUS "PCL info: ==================================")
message(STATUS ${PCL_INCLUDE_DIR})
message(STATUS "BOOST info: ==================================")
message(STATUS ${Boost_FOUND})
message(STATUS ${Boost_INCLUDE_DIRS})
message(STATUS ${Boost_LIBRARIES})
message(STATUS ${Boost_SYSTEM_LIBRARY})
message(STATUS ${Boost_FILESYSTEM_LIBRARY})
message(STATUS "BOOST info: ==================================")
message(STATUS ${VTK_FOUND})
message(STATUS ${VTK_INCLUDE_DIRS})
message(STATUS "BOOST info: ==================================")
message(STATUS ${ITK_FOUND})
message(STATUS ${ITK_INCLUDE_DIRS})
message(STATUS "ZLIB info: ==================================")
message(STATUS ${ZLIB_FOUND})
message(STATUS ${ZLIB_INCLUDE_DIR})

