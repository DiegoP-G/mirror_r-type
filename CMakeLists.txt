cmake_minimum_required(VERSION 3.16)
project(RTypeProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

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
)

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
)

target_link_libraries(GameServer PRIVATE
    transferData
    ecs
    sfml-graphics
    sfml-window
    sfml-system
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
    
    # Copy SFML DLLs for GameServer (doesn't use audio)
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

    # Copy OpenAL DLL for RTypeClient (required for SFML-audio)
    add_custom_command(TARGET RTypeClient POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x64/openal32.dll"
        $<TARGET_FILE_DIR:RTypeClient>
        COMMENT "Copying OpenAL DLL for RTypeClient"
    )
    
    # GameServer doesn't use audio, so no OpenAL needed
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