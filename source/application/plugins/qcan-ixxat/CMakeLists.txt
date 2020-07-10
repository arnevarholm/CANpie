#----------------------------------------------------------------------------------------------------------------------#
# CMake file plugin QCanIxxat                                                                                       #
#                                                                                                                      #
#----------------------------------------------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.16.4 FATAL_ERROR)
cmake_policy(SET CMP0048 NEW)

message("-- Configure plugin QCanIxxat")

#-------------------------------------------------------------------------------------------------------
# define the project name and version
#
project(QCanIxxat VERSION 1.00.4)


#-------------------------------------------------------------------------------------------------------
# add project CMake module include path
#
SET( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake" ${CMAKE_MODULE_PATH})
INCLUDE(AddCompilerFlags )
INCLUDE(AddSourceDirectories )


#-------------------------------------------------------------------------------------------------------
# specify include paths 
#
include_directories(${CP_PATH_MISC})
include_directories(${CP_PATH_QCAN})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("C:\\Program Files\\HMS\\IXXAT VCI 4.0\\sdk\\vci\\inc")


#-------------------------------------------------------------------------------------------------------
# pass version information to application
#
add_definitions(-DVERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_definitions(-DVERSION_MINOR=${PROJECT_VERSION_MINOR})
add_definitions(-DVERSION_BUILD=${PROJECT_VERSION_PATCH})


#------------------------------------------------------------------------------------------------------- 
# define source files for compilation
#
list(
   APPEND PLUGIN_SOURCES
   qcan_interface_ixxat.cpp
   qcan_plugin_ixxat.cpp
   qcan_ixxat_vci.cpp
   qcan_plugin_ixxat.cpp
)

list(
   APPEND QCAN_SOURCES
   ${CP_PATH_QCAN}/qcan_frame.cpp
   ${CP_PATH_QCAN}/qcan_timestamp.cpp
)

list (APPEND RESOURCES qcan_ixxat.qrc)


#-------------------------------------------------------------------------------------------------------
# Qt specific setup: use moc and rcc, check for required Qt libraries
#
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)


#-------------------------------------------------------------------------------------------------------
# create plugin from selected source files
#
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES} ${QCAN_SOURCES} ${RESOURCES})


#-------------------------------------------------------------------------------------------------------
# link Qt libs 
#
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)

				   				   
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD                   
                   COMMAND ${CMAKE_COMMAND} -E echo "       Copy $<TARGET_FILE:${PROJECT_NAME}> to ${CP_PATH_PLUGINS}"				   
				   COMMAND ${CMAKE_COMMAND} -E make_directory ${CP_PATH_PLUGINS} 
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CP_PATH_PLUGINS})
				   
