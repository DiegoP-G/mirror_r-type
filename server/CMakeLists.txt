cmake_minimum_required(VERSION 3.16)
project(GameServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Inclure tous les headers
include_directories(
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Game
    ${CMAKE_SOURCE_DIR}/Mediator
    ${CMAKE_SOURCE_DIR}/Network
    ${CMAKE_SOURCE_DIR}/transferData
)

# Récupération de tous les fichiers sources
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/*.cpp
    ${CMAKE_SOURCE_DIR}/Game/*.cpp
    ${CMAKE_SOURCE_DIR}/Mediator/*.cpp
    ${CMAKE_SOURCE_DIR}/Network/*.cpp
    ${CMAKE_SOURCE_DIR}/Network/TCP/*.cpp
    ${CMAKE_SOURCE_DIR}/Network/UDP/*.cpp
    ${CMAKE_SOURCE_DIR}/transferData/*.cpp
)

# Exclure le dossier exemple
file(GLOB_RECURSE EXAMPLES ${CMAKE_SOURCE_DIR}/exempleTranfersData/*.cpp)
list(REMOVE_ITEM SOURCES ${EXAMPLES})

# Création de l'exécutable principal
add_executable(${PROJECT_NAME}
    main.cpp
    server.cpp
    ${SOURCES}
)
