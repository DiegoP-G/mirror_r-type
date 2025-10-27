cmake_minimum_required(VERSION 3.16)
project(RTypeProject)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

# ---- Windows: Define NOMINMAX globally ----
if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

include(FetchContent)

# ---- OpenSSL ----
find_package(OpenSSL REQUIRED)
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
    set(BUILD_SHARED_LIBS TRUE CACHE BOOL "Build SFML as shared libs" FORCE)
    FetchContent_MakeAvailable(SFML)
endif()

# ---- SQLite3 local ----
set(SQLITE3_DIR ${CMAKE_SOURCE_DIR}/sqlite)

if(EXISTS ${SQLITE3_DIR}/sqlite3.c AND EXISTS ${SQLITE3_DIR}/sqlite3.h)
    message(STATUS "Using local SQLite3 from ${SQLITE3_DIR}")

    add_library(sqlite3_lib STATIC ${SQLITE3_DIR}/sqlite3.c)
    target_include_directories(sqlite3_lib PUBLIC ${SQLITE3_DIR})

    if(MSVC)
        target_compile_options(sqlite3_lib PRIVATE /W0)
    endif()

    add_library(unofficial::sqlite3::sqlite3 ALIAS sqlite3_lib)
else()
    message(FATAL_ERROR "SQLite3 files not found in ${SQLITE3_DIR}. Please add sqlite3.c and sqlite3.h to the sqlite/ directory.")
endif()

# ---- transferData (STATIC) ----
file(GLOB_RECURSE TRANSFER_SOURCES transferData/*.cpp)
list(FILTER TRANSFER_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER TRANSFER_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")

message(STATUS "transferData sources: ${TRANSFER_SOURCES}")

add_library(transferData STATIC ${TRANSFER_SOURCES})
target_include_directories(transferData PUBLIC
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
)

if (WIN32)
    target_compile_definitions(transferData PUBLIC TRANSFERDATA_STATIC)
    target_link_libraries(transferData PUBLIC ws2_32)
endif()

find_library(CRYPT_LIB crypt REQUIRED)
if (CRYPT_LIB)
    message(STATUS "Found libcrypt library: ${CRYPT_LIB}")
    target_link_libraries(transferData PRIVATE ${CRYPT_LIB})
else()
    message(FATAL_ERROR "libcrypt library not found")
endif()

# ---- ECS + Components library ----
file(GLOB_RECURSE ECS_SOURCES ecs/*.cpp)
add_library(ecs STATIC ${ECS_SOURCES})
target_include_directories(ecs PUBLIC
    ${CMAKE_SOURCE_DIR}/ecs
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/server/Game
)
target_link_libraries(ecs PUBLIC
    transferData
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sqlite3_lib
)

# ---- Client ----
file(GLOB_RECURSE CLIENT_SOURCES client/*.cpp)
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")

# ! Changed executable name
add_executable(r-type_client ${CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/client
    ${CMAKE_SOURCE_DIR}/client/Network
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
)
target_link_libraries(r-type_client PRIVATE
    transferData
    ecs
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    OpenSSL::Crypto
)

# ---- Server ----
file(GLOB_RECURSE SERVER_SOURCES server/*.cpp)
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*exempleTranfersData.*")

# ! Changed executable name
add_executable(r-type_server ${SERVER_SOURCES})

target_include_directories(r-type_server PRIVATE
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
    ${SQLITE3_DIR}
)

target_link_libraries(r-type_server PRIVATE
    transferData
    ecs
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sqlite3_lib
    OpenSSL::Crypto
)

# ---- Windows ----
if (WIN32)
    # ! Updated linkage for new binary names
    target_link_libraries(r-type_client PRIVATE winmm opengl32 ws2_32)
    target_link_libraries(r-type_server PRIVATE winmm opengl32 ws2_32)

    # ! Updated post-build copy commands
    add_custom_command(TARGET r-type_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE_DIR:r-type_client>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE_DIR:r-type_client>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE_DIR:r-type_client>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-audio>
        $<TARGET_FILE_DIR:r-type_client>
    )

    add_custom_command(TARGET r-type_server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE_DIR:r-type_server>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE_DIR:r-type_server>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE_DIR:r-type_server>
    )

    add_custom_command(TARGET r-type_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x64/openal32.dll"
        $<TARGET_FILE_DIR:r-type_client>
        COMMENT "Copying OpenAL DLL for r-type_client"
    )
endif()

# ---- Linux ----
if (UNIX AND NOT WIN32)
    target_link_libraries(r-type_client PRIVATE pthread)
    target_link_libraries(r-type_server PRIVATE pthread)
endif()

# ---- Custom targets ----
# ! Updated to match renamed binaries
add_custom_target(client DEPENDS r-type_client)
add_custom_target(server DEPENDS r-type_server)
add_custom_target(all_binaries DEPENDS r-type_client r-type_server)

set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_SOURCE_DIR}/r-type_client"
    "${CMAKE_SOURCE_DIR}/r-type_server"
)

