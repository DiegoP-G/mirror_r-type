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

# ---- OpenSSL ---- (MOVED TO TOP, BEFORE TARGETS)
find_package(OpenSSL REQUIRED)

# Make sure to include OpenSSL headers
include_directories(${OPENSSL_INCLUDE_DIR})

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

# ---- SQLite3 ----
find_package(SQLite3 QUIET)

if (NOT SQLite3_FOUND)
    message(STATUS "SQLite3 not found, fetching it using FetchContent...")
    FetchContent_Declare(
        sqlite3
        GIT_REPOSITORY https://github.com/sqlite/sqlite.git
        GIT_TAG        version-3.43.1 # Replace with the desired version
    )
    FetchContent_MakeAvailable(sqlite3)

    # Set SQLite3 include and library paths
    set(SQLite3_INCLUDE_DIR ${sqlite3_SOURCE_DIR})
    set(SQLite3_LIBRARIES sqlite3)
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

# ---- transferData library ----
add_library(transferData
    transferData/transferData.cpp
    transferData/serializers.cpp
    transferData/hashUtils.cpp
)

target_include_directories(transferData PUBLIC
    transferData
    ecs
)

# Link libcrypt for password hashing
find_library(CRYPT_LIB crypt REQUIRED)

if (CRYPT_LIB)
    message(STATUS "Found libcrypt library: ${CRYPT_LIB}")
    target_link_libraries(transferData PRIVATE ${CRYPT_LIB})
else()
    message(FATAL_ERROR "libcrypt library not found")
endif()

# ---- ECS + Components library ----
file(GLOB_RECURSE ECS_SOURCES
    ecs/*.cpp
)
add_library(ecs ${ECS_SOURCES})
target_include_directories(ecs PUBLIC
    ecs
    server
    ${CMAKE_SOURCE_DIR}/server/Game
)

# ECS depends on SFML
target_link_libraries(ecs PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sqlite3
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
    OpenSSL::Crypto
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
    ${OPENSSL_LIBRARIES}
    OpenSSL::Crypto
)
target_compile_options(GameServer PRIVATE ${LIBCONFIG_CFLAGS_OTHER})

# filepath: /home/keyvan/repos/mirror_r-type/CMakeLists.txt
target_sources(GameServer PRIVATE
    transferData/hashUtils.cpp
)

target_sources(RTypeClient PRIVATE
    transferData/hashUtils.cpp
)

# ---- Custom targets for convenience ----
add_custom_target(client DEPENDS RTypeClient)
add_custom_target(server DEPENDS GameServer)
add_custom_target(all_binaries DEPENDS RTypeClient GameServer)

# ---- Clean up build artifacts ----
set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_SOURCE_DIR}/RTypeClient"
    "${CMAKE_SOURCE_DIR}/GameServer"
)

