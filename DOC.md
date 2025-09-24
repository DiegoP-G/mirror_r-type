# Technical Documentation

## Architecture Overview

This game engine uses a distributed client-server architecture with an Entity-Component-System (ECS) pattern for game logic and a hybrid TCP/UDP networking protocol for communication.

## 1. Client Architecture

### Client Component Diagram

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
