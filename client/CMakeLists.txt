cmake_minimum_required(VERSION 3.16)
project(RTypeClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}          # For headers in root
    ${CMAKE_SOURCE_DIR}/Network  # Network headers
	${CMAKE_SOURCE_DIR}/../transferData
)

# Source files (exclude RType.cpp and GraphicsManager*)
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/*.hpp"
)

# Add executable
add_executable(RTypeClient ${SOURCES})

# If using SFML or other libraries, link them here
# Example (for SFML):
# find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
# target_link_libraries(RTypeClient sfml-graphics sfml-window sfml-system)

