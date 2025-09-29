cmake_minimum_required(VERSION 3.16)
project(RTypeClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}          # For headers in root
    ${CMAKE_SOURCE_DIR}/Network  # Network headers
    ${CMAKE_SOURCE_DIR}/../transferData
    ${CMAKE_SOURCE_DIR}/../ecs
)

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/Network/*.cpp"
    "${CMAKE_SOURCE_DIR}/../transferData/*.cpp"
    "${CMAKE_SOURCE_DIR}/../ecs/*.cpp"
    "${CMAKE_SOURCE_DIR}/../ecs/components/*.cpp"
)

list(FILTER SOURCES EXCLUDE REGEX ".*CMakeFiles.*")
# list(FILTER SOURCES EXCLUDE REGEX ".*RType\\.cpp")
# list(FILTER SOURCES EXCLUDE REGEX ".*GraphicsManager.*\\.cpp")
# list(FILTER SOURCES EXCLUDE REGEX ".*GraphicsManager.*\\.hpp")

add_executable(RTypeClient ${SOURCES})

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

target_link_libraries(RTypeClient sfml-graphics sfml-window sfml-system)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")
