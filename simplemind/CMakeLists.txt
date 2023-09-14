cmake_minimum_required(VERSION 3.10)
project(simplemind)
find_package(PythonExtensions REQUIRED)

# cpp flags
set (CMAKE_CXX_STANDARD 14)

add_subdirectory("simplemind/dependencies/src/PCL")
add_subdirectory("simplemind/dependencies/src/Dicom/obj/qia/common/dicom")
add_subdirectory("simplemind/dependencies/src/Img/common/qia/common/img")
add_subdirectory("simplemind/think/src/pclMIU")