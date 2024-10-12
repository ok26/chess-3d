cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(chess-3d VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the directories for the source files
include_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add subdirectories for the libraries
add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
add_subdirectory(libs/assimp)
add_subdirectory(libs/freetype)

# Specify glad include directory
include_directories(${CMAKE_SOURCE_DIR}/libs/glad/include)

# Collect all source files from src and subdirectories
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/index_model/*.cpp
    ${CMAKE_SOURCE_DIR}/src/index_model/*.h
    ${CMAKE_SOURCE_DIR}/src/model/*.cpp
    ${CMAKE_SOURCE_DIR}/src/model/*.h
    ${CMAKE_SOURCE_DIR}/src/util/*.cpp
    ${CMAKE_SOURCE_DIR}/src/util/*.h
)

# Create the executable
add_executable(chess-3d ${SOURCES})

# Link the external libraries
target_link_libraries(chess-3d
    glfw
    glm
    assimp
    freetype
)

# Add glad source files
target_sources(chess-3d PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c
)

target_include_directories(chess-3d PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/stb
)

set_target_properties(chess-3d PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)
