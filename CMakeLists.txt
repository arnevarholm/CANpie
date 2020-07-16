#----------------------------------------------------------------------------------------------------------------------#
# Top level CMake file for building the CANpie FD project                                                              #
#                                                                                                                      #
#----------------------------------------------------------------------------------------------------------------------#

#-------------------------------------------------------------------------------------------------------
# specify minimum version of CMake that can be used with CMakeLists.txt file
#
cmake_minimum_required(VERSION 3.16.4 FATAL_ERROR)
cmake_policy(SET CMP0048 NEW)

#-------------------------------------------------------------------------------------------------------
# This project can be translated with the following commands:
# > cd build
# > cmake ..
# > cmake --build .

message(CMAKE_HOST_SYSTEM_NAME=${CMAKE_HOST_SYSTEM_NAME})
if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
set(Qt5_DIR "C:/Qt/5.9.9/msvc2015/lib/cmake/Qt5")
set(Qt5_WINDEPLOYQT "C:\\\\Qt\\\\5.9.9\\\\msvc2015\\\\bin\\\\windeployqt.exe")
endif()


#-------------------------------------------------------------------------------------------------------
# specify project
#
project("CANpie FD")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/application)
if(NOT ${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

