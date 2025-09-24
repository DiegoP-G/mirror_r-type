cmake_minimum_required(VERSION 3.16)
project(RType VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ecs)
include_directories(${CMAKE_SOURCE_DIR}/ecs/components)
include_directories(${CMAKE_SOURCE_DIR}/network)
include_directories(${CMAKE_SOURCE_DIR}/client)
include_directories(${CMAKE_SOURCE_DIR}/server)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add executable
add_executable(rtype_client
    client/RType.cpp
    client/GraphicsManager.cpp
)

# Link SFML libraries
target_link_libraries(rtype_client 
    sfml-graphics
    sfml-window
    sfml-system
)

# Install target
install(TARGETS rtype_client DESTINATION bin)

# Install target
install(TARGETS rtype_client DESTINATION bin)

# Custom targets for traditional Makefile commands
add_custom_target(fclean
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CTestTestfile.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Full clean: removing all build files and directories"
)

add_custom_target(re
    COMMAND ${CMAKE_COMMAND} --build . --target fclean
    COMMAND ${CMAKE_COMMAND} --build . --target all
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Rebuild: full clean and build"
    DEPENDS fclean
)

# Override default clean to be more verbose
add_custom_target(clean_verbose
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning object files and executables"
)