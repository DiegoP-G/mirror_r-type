cmake_minimum_required(VERSION 3.16)
project(RTypeProject)

# ---- C++ standards ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Output directory for binaries (root of project) ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

# ---- Find required packages ----
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIG REQUIRED libconfig++)

# ---- transferData library ----
add_library(transferData
    transferData/transferData.cpp
    transferData/serializers.cpp
)
target_include_directories(transferData PUBLIC 
    transferData
    ecs  # Include ECS headers
)

# ---- Client executable ----
# Include directories for client
set(CLIENT_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/client
    ${CMAKE_SOURCE_DIR}/client/Network
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
)

# Client source files
file(GLOB_RECURSE CLIENT_SOURCES
    "client/*.cpp"
)
# Exclude build directories and unwanted files
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")

add_executable(RTypeClient ${CLIENT_SOURCES})
target_include_directories(RTypeClient PRIVATE ${CLIENT_INCLUDE_DIRS})
target_link_libraries(RTypeClient 
    transferData 
    sfml-graphics 
    sfml-window 
    sfml-system
)

# ---- Server executable ----
# Include directories for server
set(SERVER_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_SOURCE_DIR}/server/Game
    ${CMAKE_SOURCE_DIR}/server/Network
    ${CMAKE_SOURCE_DIR}/server/Network/TCP
    ${CMAKE_SOURCE_DIR}/server/Network/UDP
    ${CMAKE_SOURCE_DIR}/server/Mediator
    ${CMAKE_SOURCE_DIR}/server/Include
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
)

# Server source files
file(GLOB_RECURSE SERVER_SOURCES
    "server/*.cpp"
)
# Exclude build directories, example files, and unwanted files
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*exempleTranfersData.*")

add_executable(GameServer ${SERVER_SOURCES})
target_include_directories(GameServer PRIVATE ${SERVER_INCLUDE_DIRS})
target_link_libraries(GameServer 
    transferData 
    ${LIBCONFIG_LIBRARIES}
)
target_compile_options(GameServer PRIVATE ${LIBCONFIG_CFLAGS_OTHER})

# ---- Custom targets for convenience ----
add_custom_target(client DEPENDS RTypeClient)
add_custom_target(server DEPENDS GameServer)
add_custom_target(all_binaries DEPENDS RTypeClient GameServer)

# ---- Clean up build artifacts ----
set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_SOURCE_DIR}/RTypeClient"
    "${CMAKE_SOURCE_DIR}/GameServer"
)