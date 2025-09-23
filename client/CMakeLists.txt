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

# Source files (all .cpp and .hpp recursively)
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/*.cpp"
	"${CMAKE_SOURCE_DIR}/Network/*.cpp"
	"${CMAKE_SOURCE_DIR}/../transferData/*.cpp"
)

list(FILTER SOURCES EXCLUDE REGEX ".*CMakeFiles.*")
# # Exclude unwanted files
# list(FILTER SOURCES EXCLUDE REGEX ".*RType\\.cpp")
# list(FILTER SOURCES EXCLUDE REGEX ".*GraphicsManager.*\\.cpp")
# list(FILTER SOURCES EXCLUDE REGEX ".*GraphicsManager.*\\.hpp")

# Add executable
add_executable(RTypeClient ${SOURCES})

# Link SFML libraries
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(RTypeClient sfml-graphics sfml-window sfml-system)

