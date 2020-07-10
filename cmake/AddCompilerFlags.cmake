#----------------------------------------------------------------------------------------------------------------------#
# This CMake include defines compiler flags for the project                                                            #
#                                                                                                                      #
#                                                                                                                      #
#----------------------------------------------------------------------------------------------------------------------#
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()
message("C++ compiler flags: ${CMAKE_CXX_FLAGS}")

