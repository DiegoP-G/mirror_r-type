cmake_minimum_required(VERSION 3.16)
project(RTypeProject)

# ---- C++ standards ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Output directory for binaries ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

# ---- FetchContent for dependencies ----
include(FetchContent)

# ---- SFML ----
find_package(SFML 2.5 COMPONENTS graphics window system audio QUIET)

if (NOT SFML_FOUND)
    message(STATUS "SFML not found, fetching it using FetchContent...")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
    )
    FetchContent_MakeAvailable(SFML)
endif()

# ---- libconfig++ ----
find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(LIBCONFIG libconfig++ QUIET)
endif()

if (NOT LIBCONFIG_FOUND)
    message(STATUS "libconfig++ not found, fetching it using FetchContent...")
    FetchContent_Declare(
        libconfig
        GIT_REPOSITORY https://github.com/hyperrealm/libconfig.git
        GIT_TAG v1.7.3
    )
    FetchContent_MakeAvailable(libconfig)
    set(LIBCONFIG_INCLUDE_DIRS ${libconfig_SOURCE_DIR})
    set(LIBCONFIG_LIBRARIES libconfig++)
endif()

# ---- LZ4 ----
find_package(LZ4 QUIET)

if (NOT LZ4_FOUND)
    message(STATUS "LZ4 not found, fetching it using FetchContent...")
    FetchContent_Declare(
        lz4
        GIT_REPOSITORY https://github.com/lz4/lz4.git
        GIT_TAG v1.9.4
    )
    FetchContent_MakeAvailable(lz4)

    # Build the LZ4 source into a static library
    add_library(lz4_src ${lz4_SOURCE_DIR}/lib/lz4.c)
    target_include_directories(lz4_src PUBLIC ${lz4_SOURCE_DIR}/lib)
endif()

# ---- transferData library ----
add_library(transferData
    transferData/transferData.cpp
    transferData/serializers.cpp
)
target_include_directories(transferData PUBLIC
    transferData
    ecs
)

# Link against either system LZ4 or fetched version
if (LZ4_FOUND)
    target_link_libraries(transferData PUBLIC LZ4::LZ4)
else()
    target_link_libraries(transferData PUBLIC lz4_src)
endif()

# ---- ECS + Components library ----
file(GLOB_RECURSE ECS_SOURCES
    ecs/*.cpp
)
add_library(ecs ${ECS_SOURCES})
target_include_directories(ecs PUBLIC ecs)

# ECS depends on SFML
target_link_libraries(ecs PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# ---- Client executable ----
file(GLOB_RECURSE CLIENT_SOURCES
    client/*.cpp
)
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")

add_executable(RTypeClient ${CLIENT_SOURCES})

target_include_directories(RTypeClient PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/client
    ${CMAKE_SOURCE_DIR}/client/Network
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
)

target_link_libraries(RTypeClient
    transferData
    ecs
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# ---- Server executable ----
file(GLOB_RECURSE SERVER_SOURCES
    server/*.cpp
)
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*exempleTranfersData.*")

add_executable(GameServer ${SERVER_SOURCES})

target_include_directories(GameServer PRIVATE
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

target_link_libraries(GameServer
    transferData
    ecs
    ${LIBCONFIG_LIBRARIES}
    sfml-graphics
    sfml-window
    sfml-system
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
