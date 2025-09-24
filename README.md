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

- C++17 compatible compiler
- SFML
- libconfig++ +1.7.3
- CMake 3.16+

### Building

```bash
# Clone the repository
git clone --recurse-submodules git@github.com:DiegoP-G/mirror_r-type.git
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
./bin/rtype_server

# Start a client (in another terminal)
./bin/rtype_client
```

## Project Structure

```
├── client/                    # Client-side code
├── server/                    # Server-side code
├── ecs/                       # Shared ECS implementation
├── transferData/              # Shared network protocol
├── assets/                    # Game assets
├── doc/             # Project documentation PDFs
├── submodules/               # Git submodules (cmake, SFML)
└── CMakeLists.txt            # Root build configuration
```
For more details see [ARCHITECTURE.md](ARCHITECTURE.md).

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
- **Multiplayer**: Synchronized multiplayer gameplay

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## Documentation

See [ARCHITECTURE.md](ARCHITECTURE.md) for a detailed technical documentation for developers.

