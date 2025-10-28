cmake_minimum_required(VERSION 3.16)
project(RTypeProject)

# FIX: The CMAKE_TOOLCHAIN_FILE must be set on the command line, not in the script.
# This entire block has been removed to prevent conflicts.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")

# Set output directories for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})


# ---- Windows: Define NOMINMAX globally ----
if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

# ============================================================
#                         VCPKG AUTO-INSTALL
# ============================================================
# NOTE: This section attempts to auto-install vcpkg and its packages.
# This can slow down configuration. For faster builds, consider installing
# dependencies manually with vcpkg once.

set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")

# Clone vcpkg if it doesn't exist
if(NOT EXISTS "${VCPKG_ROOT}/vcpkg.exe" AND NOT EXISTS "${VCPKG_ROOT}/vcpkg")
    message(STATUS "⚙️ Cloning vcpkg...")
    execute_process(
        COMMAND git clone https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE GIT_CLONE_RESULT
    )

    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "❌ Failed to clone vcpkg")
    endif()

    if(WIN32)
        execute_process(COMMAND cmd /c bootstrap-vcpkg.bat WORKING_DIRECTORY "${VCPKG_ROOT}")
    else()
        execute_process(COMMAND ./bootstrap-vcpkg.sh WORKING_DIRECTORY "${VCPKG_ROOT}")
    endif()
endif()

# ============================================================
#                       Dépendances & Libs
# ============================================================

include(FetchContent)

# ---- OpenSSL (found via vcpkg) ----
find_package(OpenSSL REQUIRED)
# FIX: Removed `include_directories(${OPENSSL_INCLUDE_DIR})`.
# Linking with OpenSSL::SSL and OpenSSL::Crypto handles this automatically.

# ---- PortAudio ----
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG v19.7.0
    GIT_SHALLOW TRUE
)

set(PA_BUILD_SHARED OFF CACHE BOOL "Build shared library" FORCE)
set(PA_BUILD_STATIC ON CACHE BOOL "Build static library" FORCE)
# Disable platform-specific backends you don't need
set(PA_USE_JACK OFF CACHE BOOL "Disable JACK" FORCE)
set(PA_USE_ASIO OFF CACHE BOOL "Disable ASIO" FORCE)
set(PA_USE_WASAPI ON CACHE BOOL "Enable WASAPI for Windows" FORCE) # Assuming WASAPI is desired on Windows
set(PA_USE_DS OFF CACHE BOOL "Disable DirectSound" FORCE)

FetchContent_MakeAvailable(portaudio)

# ---- SFML ----
# Attempt to find SFML; if not found, download it via FetchContent
find_package(SFML 2.6 COMPONENTS graphics window system audio QUIET)

if (NOT SFML_FOUND)
    message(STATUS "SFML not found, fetching it using FetchContent...")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
    )
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build SFML as shared libs" FORCE)
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

# ============================================================
#                         TRANSFER DATA
# ============================================================

file(GLOB_RECURSE TRANSFER_SOURCES transferData/*.cpp)
add_library(transferData STATIC ${TRANSFER_SOURCES})
target_include_directories(transferData PUBLIC
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
)

if (WIN32)
    target_compile_definitions(transferData PUBLIC TRANSFERDATA_STATIC)
    target_link_libraries(transferData PUBLIC ws2_32)
    target_link_libraries(transferData PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto
)
endif()

# FIX: libcrypt is a Linux-specific library. This avoids build errors on Windows.
if (UNIX)
    find_library(CRYPT_LIB crypt)
    if (CRYPT_LIB)
        message(STATUS "Found libcrypt library: ${CRYPT_LIB}")
        target_link_libraries(transferData PRIVATE ${CRYPT_LIB})
    else()
        message(WARNING "libcrypt library not found, some features may not work")
    endif()
endif()


# ============================================================
#                               ECS
# ============================================================

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

# ============================================================
#                              CLIENT
# ============================================================

file(GLOB_RECURSE CLIENT_SOURCES client/*.cpp)
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")

add_executable(r-type_client ${CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/client
    ${CMAKE_SOURCE_DIR}/client/Network
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
    ${portaudio_SOURCE_DIR}/include
)
target_link_libraries(r-type_client PRIVATE
    transferData
    ecs
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    OpenSSL::SSL
    OpenSSL::Crypto
    portaudio_static
)

# ============================================================
#                              SERVER
# ============================================================

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
    # FIX: Link to both OpenSSL targets provided by vcpkg for full functionality.
    OpenSSL::SSL
    OpenSSL::Crypto
)

# ============================================================
#                           OS-SPECIFIC
# ============================================================

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

