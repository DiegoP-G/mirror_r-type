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
# Assumer que sqlite3.h et sqlite3.c sont dans le dossier sqlite/
set(SQLITE3_DIR ${CMAKE_SOURCE_DIR}/sqlite)

if(EXISTS ${SQLITE3_DIR}/sqlite3.c AND EXISTS ${SQLITE3_DIR}/sqlite3.h)
    message(STATUS "Using local SQLite3 from ${SQLITE3_DIR}")
    
    add_library(sqlite3_lib STATIC ${SQLITE3_DIR}/sqlite3.c)
    target_include_directories(sqlite3_lib PUBLIC ${SQLITE3_DIR})
    
    # Désactiver les warnings SQLite sur MSVC
    if(MSVC)
        target_compile_options(sqlite3_lib PRIVATE /W0)
    endif()
    
    # Alias pour correspondre à la convention
    add_library(unofficial::sqlite3::sqlite3 ALIAS sqlite3_lib)
else()
    message(FATAL_ERROR "SQLite3 files not found in ${SQLITE3_DIR}. Please add sqlite3.c and sqlite3.h to the sqlite/ directory.")
endif()

# ---- transferData (STATIC) ----
file(GLOB_RECURSE TRANSFER_SOURCES transferData/*.cpp)
list(FILTER TRANSFER_SOURCES EXCLUDE REGEX ".*build.*")
list(FILTER TRANSFER_SOURCES EXCLUDE REGEX ".*CMakeFiles.*")

message(STATUS "transferData sources: ${TRANSFER_SOURCES}")

# ---- ZLIB ----
find_package(ZLIB QUIET)

if (NOT ZLIB_FOUND)
    message(STATUS "Zlib not found, fetching it using FetchContent...")

    include(FetchContent)
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.2.13
    )
    FetchContent_MakeAvailable(zlib)
endif()

# ---- transferData library ----
add_library(transferData STATIC ${TRANSFER_SOURCES})
target_include_directories(transferData PUBLIC
    ${CMAKE_SOURCE_DIR}/transferData
    ${CMAKE_SOURCE_DIR}/ecs
)

if (WIN32)
    target_compile_definitions(transferData PUBLIC TRANSFERDATA_STATIC)
    target_link_libraries(transferData PUBLIC ws2_32)
endif()

# ---- ecs (STATIC) ----
file(GLOB_RECURSE ECS_SOURCES ecs/*.cpp)
add_library(ecs STATIC ${ECS_SOURCES})
target_include_directories(ecs PUBLIC 
    ${CMAKE_SOURCE_DIR}/ecs 
    ${CMAKE_SOURCE_DIR}/transferData
)
target_link_libraries(ecs PUBLIC 
    transferData
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sqlite3_lib
)

target_link_libraries(transferData PUBLIC ZLIB::ZLIB)

# # ECS depends on SFML
# target_link_libraries(ecs PUBLIC
# )

# ---- Client ----
file(GLOB_RECURSE CLIENT_SOURCES client/*.cpp)
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
target_link_libraries(RTypeClient PRIVATE
    transferData
    ecs
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# ---- Server ----
file(GLOB_RECURSE SERVER_SOURCES server/*.cpp)
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
    ${SQLITE3_DIR}  # ✅ Ajouter le dossier SQLite pour le serveur
)

target_include_directories(GameServer PRIVATE /usr/local/include)

target_link_libraries(GameServer PRIVATE
    transferData
    ecs
    sfml-graphics
    sfml-window
    sfml-system
    sqlite3_lib
    pthread
    prometheus-cpp-core
    prometheus-cpp-pull
)

# ---- Windows ----
if (WIN32)
    target_link_libraries(RTypeClient PRIVATE winmm opengl32 ws2_32)
    target_link_libraries(GameServer PRIVATE winmm opengl32 ws2_32)

    # Copy SFML DLLs for RTypeClient
    add_custom_command(TARGET RTypeClient POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE_DIR:RTypeClient>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE_DIR:RTypeClient>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE_DIR:RTypeClient>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-audio>
        $<TARGET_FILE_DIR:RTypeClient>
    )
    
    # Copy SFML DLLs for GameServer
    add_custom_command(TARGET GameServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE_DIR:GameServer>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE_DIR:GameServer>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE_DIR:GameServer>
    )

    # Copy OpenAL DLL for RTypeClient
    add_custom_command(TARGET RTypeClient POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x64/openal32.dll"
        $<TARGET_FILE_DIR:RTypeClient>
        COMMENT "Copying OpenAL DLL for RTypeClient"
    )
endif()

# ---- Linux ----
if (UNIX AND NOT WIN32)
    target_link_libraries(RTypeClient PRIVATE pthread)
    target_link_libraries(GameServer PRIVATE pthread)
endif()

add_custom_target(client DEPENDS RTypeClient)
add_custom_target(server DEPENDS GameServer)
add_custom_target(all_binaries DEPENDS RTypeClient GameServer)

set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_SOURCE_DIR}/RTypeClient"
    "${CMAKE_SOURCE_DIR}/GameServer"
)
