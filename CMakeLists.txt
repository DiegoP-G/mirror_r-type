cmake_minimum_required(VERSION 3.16)
project(RTypeProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

# ============================================================
#                         VCPKG AUTO-INSTALL
# ============================================================
set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")

if(NOT EXISTS "${VCPKG_ROOT}/vcpkg.exe" AND NOT EXISTS "${VCPKG_ROOT}/vcpkg")
    message(STATUS "Cloning vcpkg...")
    execute_process(
        COMMAND git clone https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE GIT_CLONE_RESULT
    )
    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone vcpkg")
    endif()
    if(WIN32)
        execute_process(COMMAND cmd /c bootstrap-vcpkg.bat WORKING_DIRECTORY "${VCPKG_ROOT}")
    else()
        execute_process(COMMAND ./bootstrap-vcpkg.sh WORKING_DIRECTORY "${VCPKG_ROOT}")
    endif()
endif()

include(FetchContent)

# ---- OpenSSL (found via vcpkg) ----

if (WIN32)
    set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows")
    set(OPENSSL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/include")
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    message(STATUS "✓ OpenSSL trouvé!")
    message(STATUS "  - Version: ${OPENSSL_VERSION}")
    message(STATUS "  - Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "  - Libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "✗ OpenSSL non trouvé!")
endif()
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
set(PA_USE_JACK OFF CACHE BOOL "Disable JACK" FORCE)
set(PA_USE_ASIO OFF CACHE BOOL "Disable ASIO" FORCE)
set(PA_USE_WASAPI ON CACHE BOOL "Enable WASAPI for Windows" FORCE)
set(PA_USE_DS OFF CACHE BOOL "Disable DirectSound" FORCE)
FetchContent_MakeAvailable(portaudio)

# ---- SFML ----
find_package(SFML 2.6 COMPONENTS graphics window system audio QUIET)
if (NOT SFML_FOUND)
    message(STATUS "SFML not found, fetching it...")
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
    add_library(sqlite3_lib STATIC ${SQLITE3_DIR}/sqlite3.c)
    target_include_directories(sqlite3_lib PUBLIC ${SQLITE3_DIR})
    if(MSVC)
        target_compile_options(sqlite3_lib PRIVATE /W0)
    endif()
    add_library(unofficial::sqlite3::sqlite3 ALIAS sqlite3_lib)
else()
    message(FATAL_ERROR "SQLite3 files not found in ${SQLITE3_DIR}")
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
if (UNIX)
    find_library(CRYPT_LIB crypt)
    if (CRYPT_LIB)
        target_link_libraries(transferData PRIVATE ${CRYPT_LIB})
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
    OpenSSL::SSL
    OpenSSL::Crypto
)

# ---- Prometheus-cpp ----
find_package(prometheus-cpp CONFIG QUIET)
if (NOT prometheus-cpp_FOUND)
    message(STATUS "prometheus-cpp not found, fetching it...")
    FetchContent_Declare(
        prometheus_cpp
        GIT_REPOSITORY https://github.com/jupp0r/prometheus-cpp.git
        GIT_TAG v1.2.4
    )
    set(ENABLE_PUSH OFF CACHE BOOL "Disable prometheus push support" FORCE)
    set(ENABLE_PULL ON CACHE BOOL "Enable prometheus pull support" FORCE)
    set(ENABLE_COMPRESSION ON CACHE BOOL "Enable compression support" FORCE)
    set(ENABLE_TESTING OFF CACHE BOOL "Disable tests" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static prometheus-cpp libs" FORCE)
    FetchContent_MakeAvailable(prometheus_cpp)
endif()
if (prometheus_cpp_SOURCE_DIR)
    include_directories(
        ${prometheus_cpp_SOURCE_DIR}/core/include
        ${prometheus_cpp_SOURCE_DIR}/pull/include
        ${prometheus_cpp_BINARY_DIR}/core/include
        ${prometheus_cpp_BINARY_DIR}/pull/include
    )
endif()

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
    OpenSSL::SSL
    OpenSSL::Crypto
    prometheus-cpp::core
    prometheus-cpp::pull
)

# ============================================================
#                           OS-SPECIFIC
# ============================================================
if (WIN32)
    target_link_libraries(r-type_client PRIVATE winmm opengl32 ws2_32)
    target_link_libraries(r-type_server PRIVATE winmm opengl32 ws2_32)

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

if (UNIX AND NOT WIN32)
    target_link_libraries(r-type_client PRIVATE pthread)
    target_link_libraries(r-type_server PRIVATE pthread)
endif()

add_custom_target(client DEPENDS r-type_client)
add_custom_target(server DEPENDS r-type_server)
add_custom_target(all_binaries DEPENDS r-type_client r-type_server)

set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_SOURCE_DIR}/r-type_client"
    "${CMAKE_SOURCE_DIR}/r-type_server"
)


