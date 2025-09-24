# Technical Documentation

## Architecture Overview

This game engine uses a distributed client-server architecture with an Entity-Component-System (ECS) pattern for game logic and a hybrid TCP/UDP networking protocol for communication.

---

## 1. Client Architecture

### Client Component Diagram

mermaid:
graph TD
    ClientGame --> NetworkManager
    ClientGame --> GraphicsManager
    NetworkManager --> Sender
    NetworkManager --> Receiver
    NetworkECSMediator --> Sender
    NetworkECSMediator --> Receiver
    GraphicsManager --> EntityManager
    EntityManager --> Entity
    Entity --> Component

Notes:
- NetworkECSMediator updates ECS values and allows ECS to send updates to the server.
- GraphicsManager queries the ECS for entities and renders them in the game loop.

---

### Client Data Flow

mermaid:
sequenceDiagram
    ClientGame ->> NetworkManager: Setup connection
    NetworkManager ->> Receiver: Receive TCP/UDP messages
    Receiver ->> ECS: Update entities
    ECS ->> Sender: Send updates to server
    ClientGame ->> ECS: Update game logic
    ClientGame ->> GraphicsManager: Render entities
    GraphicsManager ->> ECS: Get entities for rendering

---

## 2. Server Architecture

### Server Component Diagram

mermaid:
graph TD
    GameLoop --> GameMediator
    NetworkManager --> ClientManager
    NetworkManager --> TCPManager
    NetworkManager --> UDPManager
    ClientManager --> Client
    GameMediator --> IMediator
    GameMediator --> AMediator
    ECS --> EntityManager
    EntityManager --> Entity
    Entity --> Component

---

### Server Data Flow

mermaid:
sequenceDiagram
    GameLoop ->> TCPManager: Accept connections
    TCPManager ->> ClientManager: Add client
    TCPManager ->> Client: Send UDP code
    UDPManager ->> ClientManager: Lookup client by code
    ClientManager ->> UDPManager: Client found
    UDPManager ->> GameMediator: Notify CLIENT_AUTHENTICATED

    loop Game Loop
        GameLoop ->> ECS: Update game state
        ECS ->> GameMediator: Notify GAME_STATE_CHANGED
        GameMediator ->> ClientManager: Broadcast game state
        ClientManager ->> UDPManager: Send to all clients
        UDPManager ->> GameMediator: Receive client input
        GameMediator ->> ECS: Update player components
    end

---

## 3. ECS System Details

### Component Hierarchy

mermaid:
graph TD
    Component --> TransformComponent
    Component --> VelocityComponent
    Component --> SpriteComponent
    Component --> PlayerComponent
    Component --> ColliderComponent
    Component --> HealthComponent
    Component --> InputComponent

### System Architecture

mermaid:
graph TD
    MovementSystem --> EntityManager
    RenderSystem --> EntityManager
    CollisionSystem --> EntityManager
    InputSystem --> EntityManager
    PlayerSystem --> EntityManager
    GameLogicSystem --> EntityManager
    EnemySystem --> EntityManager
    ProjectileSystem --> EntityManager
    GravitySystem --> EntityManager
    LaserWarningSystem --> EntityManager

---

## 4. Network Protocol

### Message Flow Protocol

mermaid:
sequenceDiagram
    participant Client
    participant ServerTCP
    participant ServerUDP

    %% Connection Phase
    Client ->> ServerTCP: TCP Connect
    ServerTCP ->> Client: CODE_UDP
    Client ->> ServerUDP: UDP_AUTH
    ServerUDP ->> Client: Auth ACK

    %% Game Phase
    loop Game Loop
        Client ->> ServerUDP: PLAYER_STATE
        ServerUDP ->> Client: WORLD_UPDATE
    end

    %% Chat
    Client ->> ServerTCP: CHAT_MESSAGE
    ServerTCP ->> Client: CHAT_BROADCAST

    %% Disconnection
    Client ->> ServerTCP: CLOSE_CONNECTION
    ServerTCP ->> Client: ACK

---

## 5. Data Serialization

All components implement serialization methods for network synchronization. Simple types are copied directly; complex types use custom serialize/deserialize methods.

---

## 6. Threading Model

Client:
- Main Thread: Game loop, ECS updates, rendering
- Network Thread: TCP/UDP message handling

Server:
- Main Thread: Game logic, ECS updates
- Network Thread: TCP/UDP connection management
- Per-Client Thread: Message processing

---

## 7. Error Handling

Network Error Recovery:
- TCP timeouts and reconnection attempts
- UDP packet loss tolerance via state synchronization
- Graceful degradation in poor network conditions

Game State Synchronization:
- Server authoritative
- Client prediction with server reconciliation
- Lag compensation for responsive gameplay
