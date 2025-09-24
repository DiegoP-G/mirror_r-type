# Game Engine Project

A C++ multiplayer game engine built with Entity-Component-System (ECS) architecture and client-server networking.

## Features

- **ECS Architecture**: Flexible component-based entity system
- **Multiplayer Networking**: TCP/UDP hybrid networking with client-server architecture
- **Graphics**: SFML-based rendering system
- **Game Types**: Support for various game types (R-Type, Flappy Bird, etc.)
- **Serialization**: Built-in component serialization for network synchronization

## Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- SFML 2.5+
- libconfig++ (for configuration management)
- CMake 3.16+

### Building

```bash
# Clone the repository
git clone git@github.com:DiegoP-G/mirror_r-type.git
cd mirror_r-type

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)
```

### Running

```bash
# Start the server
./server

# Start a client (in another terminal)
./client
```

## Project Structure

```
├── client/                    # Client-side code
│   ├── ClientGame.*           # Main client application
│   ├── GraphicsManager.*      # SFML rendering system  
│   ├── RType.cpp              # Game-specific client logic
│   ├── Network/               # Client networking
│   │   ├── NetworkManager.*   # Client network coordination
│   │   ├── Sender.*          # TCP/UDP message sending
│   │   └── Receiver.*        # TCP/UDP message receiving
│   ├── NetworkECSMediator.*   # Network-ECS bridge
│   ├── assetsPath.hpp         # Asset path definitions
│   └── CMakeLists.txt         # Client build configuration
├── server/                    # Server-side code
│   ├── Network/               # Server networking
│   │   ├── NetworkManager.*   # Server network coordination
│   │   ├── ClientManager.*    # Client session management
│   │   ├── Client.*          # Individual client representation
│   │   ├── TCP/TCPManager.*   # TCP connection handling
│   │   └── UDP/UDPManager.*   # UDP packet handling
│   ├── Game/                  # Game logic
│   │   ├── GameLoop.*         # Main game loop
│   │   ├── GameMediator.*     # Game event coordination
│   │   └── ConfigurationManager.*  # Configuration management
│   ├── Mediator/              # Mediator pattern implementation
│   │   ├── IMediator.hpp      # Mediator interface
│   │   └── AMediator.*        # Abstract mediator base
│   ├── Include/Errors/        # Error handling
│   └── CMakeLists.txt         # Server build configuration
├── ecs/                       # Shared ECS implementation
│   ├── ecs.hpp               # Core ECS classes (Entity, Component, EntityManager)
│   ├── components.hpp        # Game components (Transform, Velocity, etc.)
│   └── systems.hpp           # Game systems (Movement, Collision, etc.)
├── transferData/              # Shared network protocol
│   ├── opcode.hpp            # Network message opcodes
│   ├── transferData.*        # Serialization utilities
│   ├── serializers.*         # Data serialization functions
│   └── structTransfer.hpp    # Data structures for network
├── assets/                    # Game assets
│   ├── fonts/                # Font files
│   └── sprites/              # Sprite images
├── documentation/             # Project documentation PDFs
├── submodules/               # Git submodules (cmake, SFML)
└── CMakeLists.txt            # Root build configuration
```

## Architecture Overview

### ECS (Entity-Component-System)

- **Entities**: Game objects identified by unique IDs
- **Components**: Data containers (Position, Velocity, Sprite, etc.)
- **Systems**: Logic processors that operate on entities with specific components

### Networking

- **TCP**: Reliable connection for authentication and critical data
- **UDP**: Fast unreliable protocol for real-time game state updates
- **Hybrid Protocol**: Combines both for optimal performance

## Games Supported

- **R-Type**: Side-scrolling shooter with enemies and projectiles
- **Flappy Bird**: Physics-based platformer with gravity and jumping
- **Multiplayer**: Synchronized multiplayer gameplay

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## Building

See [BUILD.md](BUILD.md) for detailed build instructions.


## Documentation

See [DOC.md](DOC.md) for a detailed technical documentation for developers.

