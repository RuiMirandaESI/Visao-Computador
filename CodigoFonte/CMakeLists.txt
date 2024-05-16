cmake_minimum_required(VERSION 3.29.3)
project(CodigoFonte)

# Find OpenCV package
find_package(OpenCV REQUIRED)
# Include directories from OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Create an executable
add_executable(CodigoFonte main.cpp vc.c)

# Link OpenCV Libraries
target_link_libraries(CodigoFonte ${OpenCV_LIBS})

#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)