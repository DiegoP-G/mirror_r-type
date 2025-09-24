cmake_minimum_required(VERSION 3.16)
project(GameServer LANGUAGES CXX)

# ----------------------
# C++ standard
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------
# Include directories
# ----------------------
include_directories(
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Game
    ${CMAKE_SOURCE_DIR}/Mediator
    ${CMAKE_SOURCE_DIR}/Network
    ${CMAKE_SOURCE_DIR}/../transferData
)

# ----------------------
# Source files
# ----------------------
# Only include actual source directories; avoid globbing CMakeFiles/ or root
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/Game/*.cpp
    ${CMAKE_SOURCE_DIR}/Mediator/*.cpp
    ${CMAKE_SOURCE_DIR}/Network/*.cpp
    ${CMAKE_SOURCE_DIR}/Network/TCP/*.cpp
    ${CMAKE_SOURCE_DIR}/Network/UDP/*.cpp
    ${CMAKE_SOURCE_DIR}/../transferData/*.cpp
)

# Exclude example files
list(FILTER SOURCES EXCLUDE REGEX ".*/exempleTranfersData/.*")

# ----------------------
# Executable
# ----------------------
# Only one main.cpp
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/main.cpp ${SOURCES})



# ----------------------
# Link libconfig++
# ----------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIG REQUIRED libconfig++)
include_directories(${LIBCONFIG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBCONFIG_LIBRARIES})

# ----------------------
# Optional: avoid globbing issues in future
# ----------------------
message(STATUS "Sources included in build:")
foreach(src IN LISTS SOURCES)
    message(STATUS "  ${src}")
endforeach()

