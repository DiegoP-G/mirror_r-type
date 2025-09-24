# Project Structure

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
├── doc/                        # Project documentation
├── submodules/               # Git submodules (cmake, SFML)
└── CMakeLists.txt            # Root build configuration
```

## Architecture Overview

This game engine uses a distributed client-server architecture with an Entity-Component-System (ECS) pattern for game logic and a hybrid TCP/UDP networking protocol for communication.

## 1. Client Architecture

### Client Component Diagram

This diagram shows the main client-side components and their relationships.
The NetworkECSMediator bridges network communication with the ECS system for seamless game state updates.

```
┌─────────────────────────────────────────────────────────────────┐
│                      Client Application                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────┐              ┌──────────────────────┐      │
│  │   ClientGame    │              │  GraphicsManager     │      │
│  └─────────────────┘              └──────────────────────┘      │
│           │                                   │                 │
│           │                                   │renders          │
│           │                                   ▼                 │
│           │              ┌─────────────────────────────────┐    │
│           │              │          ECS System             │    │
│           │              │  ┌─────────────────────────┐    │    │
│           │              │  │   EntityManager        │    │    │
│           │              │  └─────────────────────────┘    │    │
│           │              │  ┌─────────────────────────┐    │    │
│           │              │  │      Entity            │    │    │
│           │              │  └─────────────────────────┘    │    │
│           │              │  ┌─────────────────────────┐    │    │
│           │              │  │     Component          │    │    │
│           │              │  └─────────────────────────┘    │    │
│           │              └─────────────────────────────────┘    │
│           │                                                     │
│           ▼                                                     │
│  ┌─────────────────────────────────────────────────────┐       │
│  │              Network Layer                          │       │
│  │  ┌─────────────────┐                               │       │
│  │  │ NetworkManager  │                               │       │
│  │  └─────────────────┘                               │       │
│  │           │                                         │       │
│  │           ├──────────┬─────────────────────────┐    │       │
│  │           ▼          ▼                         ▼    │       │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────┐  │
│  │  │   Sender    │  │  Receiver   │  │NetworkECSMediator│  │
│  │  └─────────────┘  └─────────────┘  └──────────────────┘  │
│  │                                             │             │
│  │                                             │updates      │
│  │                                             ▼             │
│  └─────────────────────────────────────────────────────────┘
│                                              ECS             │
└─────────────────────────────────────────────────────────────────┘
```

### Client Data Flow

This diagram illustrates the data flow between client components during gameplay.
Messages flow from ClientGame through NetworkManager to the server, with ECS handling game state updates.

```
ClientGame ─┐
            │
            ▼
    NetworkManager ─┐
                    │
                    ├─► Receiver ─┐
                    │             │
                    └─► Sender ◄──┤
                                  │
                                  ▼
                               Server
                                  │
                                  ▼
                                ECS ─┐
                                     │
                                     ▼
                            GraphicsManager
                                     │
                                     ▼
                                 Render
```

## 2. Server Architecture

### Server Component Diagram

This diagram shows the server architecture with separate TCP/UDP managers for different communication needs.
GameMediator coordinates between network events and game logic, while ClientManager tracks connected players.

```
┌─────────────────────────────────────────────────────────────────┐
│                     Server Application                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────┐              ┌──────────────────────┐      │
│  │   GameLoop      │              │   GameMediator       │      │
│  └─────────────────┘              └──────────────────────┘      │
│           │                                   │                 │
│           │                                   │                 │
│           ▼                                   ▼                 │
│  ┌─────────────────────────────────────────────────────┐       │
│  │              Network Layer                          │       │
│  │  ┌─────────────────┐                               │       │
│  │  │ NetworkManager  │                               │       │
│  │  └─────────────────┘                               │       │
│  │           │                                         │       │
│  │           ├──────────┬─────────────────────────┐    │       │
│  │           ▼          ▼                         ▼    │       │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │       │
│  │  │ TCPManager  │  │ UDPManager  │  │ClientManager│  │       │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  │       │
│  │                                             │       │       │
│  │                                             ▼       │       │
│  │                                    ┌─────────────┐  │       │
│  │                                    │   Client    │  │       │
│  │                                    └─────────────┘  │       │
│  └─────────────────────────────────────────────────────┘       │
│                                                                 │
│  ┌─────────────────────────────────────────────────────┐       │
│  │             ECS Game Logic                          │       │
│  │  (Same ECS classes as client                        │       │
│  │   but with server-side game systems)                │       │
│  └─────────────────────────────────────────────────────┘       │
│                                                                 │
│  ┌─────────────────────────────────────────────────────┐       │
│  │         ConfigurationManager                        │       │
│  └─────────────────────────────────────────────────────┘       │
└─────────────────────────────────────────────────────────────────┘
```

### Server Data Flow

This diagram shows how data flows through the server during game operations.
The GameLoop drives ECS updates, which trigger network broadcasts to all connected clients via UDP.

```
GameLoop ──┐
           │
           ▼
    TCPManager ──┐
                 │
                 ▼
           ClientManager ──┐
                          │
                          ▼
                    GameMediator ──┐
                                   │
                                   ▼
                                 ECS ──┐
                                       │
                                       ▼
                               UDPManager ──► Clients
```

## 3. ECS System Details

### Component Hierarchy

This diagram displays the inheritance structure of all ECS components in the game engine.
Each specialized component extends the base Component class with game-specific data and behavior.

```
                    ┌─────────────┐
                    │  Component  │
                    └─────────────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
              ▼            ▼            ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │ Transform   │ │ Velocity    │ │  Sprite     │
    │ Component   │ │ Component   │ │ Component   │
    └─────────────┘ └─────────────┘ └─────────────┘
              │            │            │
              ▼            ▼            ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │   Player    │ │  Collider   │ │   Health    │
    │ Component   │ │ Component   │ │ Component   │
    └─────────────┘ └─────────────┘ └─────────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │   Input     │
                    │ Component   │
                    └─────────────┘
```

### System Architecture

This diagram shows the organization of ECS systems into core engine systems and game-specific logic.
All systems query the EntityManager to process entities with relevant component combinations.

```
┌─────────────────────────────────────────────────────────────────┐
│                        Core Systems                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ Movement    │  │   Render    │  │ Collision   │              │
│  │  System     │  │  System     │  │  System     │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐                              │
│  │   Input     │  │   Player    │                              │
│  │  System     │  │  System     │                              │
│  └─────────────┘  └─────────────┘                              │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Game-Specific Systems                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ GameLogic   │  │   Enemy     │  │ Projectile  │              │
│  │  System     │  │  System     │  │  System     │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐                              │
│  │  Gravity    │  │LaserWarning │                              │
│  │  System     │  │  System     │                              │
│  └─────────────┘  └─────────────┘                              │
│                                                                 │
│                         │                                       │
│                         ▼                                       │
│                 EntityManager                                   │
└─────────────────────────────────────────────────────────────────┘
```

## 4. Network Protocol

### Message Flow Protocol

This diagram shows the complete network communication sequence from connection to disconnection.
TCP handles reliable messaging (auth, chat) while UDP manages fast-paced game state updates.

```
Client          Server TCP       Server UDP
  │                  │               │
  │──TCP Connect────►│               │
  │                  │               │
  │◄─CODE_UDP────────│               │
  │                  │               │
  │──UDP_AUTH──────────────────────► │
  │                  │               │
  │◄─────────────Auth ACK────────────│
  │                  │               │
  │                                  │
  ├──── Game Loop ───────────────────┤
  │                  │               │
  │──PLAYER_STATE──────────────────► │
  │                  │               │
  │◄─────WORLD_UPDATE────────────────│
  │                  │               │
  │                                  │
  ├─── Chat/Messaging ──────────────┤
  │                  │               │
  │──CHAT_MESSAGE───►│               │
  │                  │               │
  │◄─CHAT_BROADCAST──│               │
  │                  │               │
  │                                  │
  ├─── Disconnection ───────────────┤
  │                  │               │
  │──CLOSE_CONNECTION►│               │
  │                  │               │
  │◄─────ACK─────────│               │
  │                  │               │
```

### Opcode Structure

| Opcode | Name | Direction | Protocol | Purpose |
|--------|------|-----------|----------|---------|
| 0x00 | CODE_UDP | Server→Client | TCP | Send UDP authentication code |
| 0x01 | CLOSE_CONNECTION | Bidirectional | TCP | Graceful disconnect |
| 0x02 | INCOMPLETE_DATA | Server→Client | TCP | Request data retransmission |
| 0x10 | UDP_AUTH | Client→Server | UDP | Authenticate UDP connection |
| 0x20 | PLAYER_STATE | Client→Server | UDP | Send player state |
| 0x21 | WORLD_UPDATE | Server→Client | UDP | Broadcast world state |
| 0x30 | CHAT_MESSAGE | Client→Server | TCP | Send chat message |
| 0x31 | CHAT_BROADCAST | Server→Client | TCP | Broadcast chat message |

## 5. Data Serialization

### Serialization Interface

All components implement serialization methods for network synchronization:

```cpp
class SerializableComponent : public Component {
public:
    virtual std::vector<uint8_t> serialize() const = 0;
    static ComponentType deserialize(const uint8_t* data);
};
```

### Network Data Structures

Components are serialized using memcpy for simple types and custom serialization for complex structures:

- **Simple types** (int, float, bool): Direct memory copy
- **Strings**: Length-prefixed serialization
- **Vectors/Arrays**: Size + elements
- **Complex types**: Custom serialize/deserialize methods

## 6. Threading Model

### Client Threading
- **Main Thread**: Game loop, ECS updates, rendering
- **Network Thread**: TCP/UDP socket polling and message handling

### Server Threading
- **Main Thread**: Game logic, ECS systems
- **Network Thread**: TCP/UDP connection management
- **Per-Client**: Individual client message processing

## 7. Error Handling

### Network Error Recovery
- TCP connection timeouts with reconnection attempts
- UDP packet loss tolerance through state synchronization
- Graceful degradation when network conditions worsen

### Game State Synchronization
- Server authoritative game state
- Client prediction with server reconciliation
- Lag compensation for responsive gameplay
