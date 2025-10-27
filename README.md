# Game Engine Project
A C++ multiplayer game engine built with Entity-Component-System (ECS) architecture and client-server networking.

## Features
- **ECS Architecture**: Flexible component-based entity system
- **Multiplayer Networking**: TCP/UDP hybrid networking with client-server architecture
- **Graphics**: SFML-based rendering system
- **Game Types**: Support for various game types (R-Type, Flappy Bird, etc.)
- **Serialization**: Built-in component serialization for network synchronization
- **Monitoring**: Prometheus metrics integration for server performance monitoring

## Quick Start

### Prerequisites
- C++17 compatible compiler
- SFML
- CMake 3.16+
- SQLite 3.45.1
- OpenSSL 3.0.13
- Docker & Docker Compose (for monitoring stack)

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

**Using Docker:**
```bash
# Client
(sudo) docker build -f Dockerfile.client -t rtype-client

# Server
(sudo) docker build -f Dockerfile.server -t rtype-server
```

### Running
```bash
# Start the server
./GameServer

# Start a client (in another terminal)
./RTypeClient
```

### Monitoring with Prometheus & Grafana

The game server exposes Prometheus metrics on **port 8082** for monitoring server performance, player statistics, and network metrics.

#### Quick Setup with Docker Compose

1. **Launch the Prometheus instance:**
   ```bash
   docker-compose up -d
   ```
   This will start Prometheus on **port 9090**.

2. **Access Prometheus:**
   - Open your browser and navigate to `http://localhost:9090`
   - Verify that the game server target is being scraped successfully

3. **Set up Grafana for visualization:**
   - Download Grafana from [https://grafana.com/grafana/download](https://grafana.com/grafana/download)
   - Or run it via Docker:
     ```bash
     docker run -d -p 3000:3000 --name=grafana grafana/grafana
     ```
   - Access Grafana at `http://localhost:3000` (default credentials: admin/admin)

4. **Configure Grafana to scrape Prometheus data:**
   - In Grafana, go to **Configuration** → **Data Sources**
   - Click **Add data source** and select **Prometheus**
   - Set the URL to `http://localhost:9090` (or your Prometheus instance location)
   - Click **Save & Test**

5. **Create dashboards:**
   - Create custom dashboards to visualize game server metrics
   - Monitor player count, network latency, CPU/memory usage, and game-specific metrics

#### Available Metrics

The game server exposes various metrics on `http://localhost:8082/metrics`:
- Player connection/disconnection events
- Network packet rates (TCP/UDP)
- Game state updates frequency
- Server performance metrics (CPU, memory)
- Game-specific metrics (enemies spawned, projectiles fired, etc.)

### Gameplay

#### Objectives
**R-Type**:
- Navigate through levels while avoiding or destroying enemies.
- Defeat the boss at the end of each level to progress.
- Work with teammates in multiplayer mode to achieve victory.

#### Controls
- **Movement**: Arrow keys (Up, Down, Left, Right)
- **Shoot**: Spacebar

## Project Structure
```
├── client/                    # Client-side code
├── server/                    # Server-side code
├── ecs/                       # Shared ECS implementation
├── transferData/              # Shared network protocol
├── assets/                    # Game assets
├── doc/                       # Project documentation PDFs
├── submodules/                # Git submodules (cmake, SFML)
├── docker-compose.yml         # Monitoring stack configuration
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

### Monitoring
- **Prometheus**: Metrics collection on port 8082
- **Grafana**: Visualization and dashboards for real-time monitoring
- **Docker Compose**: Easy deployment of the complete monitoring stack

## Games Supported
- **R-Type**: Side-scrolling shooter with enemies and projectiles
- **Multiplayer**: Synchronized multiplayer gameplay

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## Documentation
See [ARCHITECTURE.md](ARCHITECTURE.md) for a detailed technical documentation for developers.