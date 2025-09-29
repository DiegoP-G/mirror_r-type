cmake_minimum_required(VERSION 3.16)
project(RTypeClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}          # For headers in root
    ${CMAKE_SOURCE_DIR}/Network  # Network headers
    ${CMAKE_SOURCE_DIR}/../transferData
    ${CMAKE_SOURCE_DIR}/../ecs
)

# Source files (all .cpp and .hpp recursively)
set(SOURCES
    main.cpp
    ClientGame.cpp
    GraphicsManager.cpp
    Network/NetworkManager.cpp
    Network/Receiver.cpp
    Network/Sender.cpp
    NetworkECSMediator.cpp
    RType.cpp
    ../transferData/transferData.cpp
    ../transferData/serializers.cpp
    ../ecs/ecs.cpp
    ../ecs/systems.cpp
    ../ecs/GraphicsManager.cpp
    ../ecs/entityManager.cpp
)


list(FILTER SOURCES EXCLUDE REGEX ".*CMakeFiles.*")
# # Exclude unwanted files
# list(FILTER SOURCES EXCLUDE REGEX ".*RType\\.cpp")
# list(FILTER SOURCES EXCLUDE REGEX ".*GraphicsManager.*\\.cpp")
# list(FILTER SOURCES EXCLUDE REGEX ".*GraphicsManager.*\\.hpp")

# Add executable
add_executable(RTypeClient ${SOURCES})

# Link SFML libraries
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(RTypeClient sfml-graphics sfml-window sfml-system)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")

